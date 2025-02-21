// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Louis Solofrizzo <louis@ne02ptzero.me>");
MODULE_DESCRIPTION("Useless module");

static ssize_t myfd_read(struct file *fp, char __user *user, size_t size, loff_t *offs);
static ssize_t myfd_write(struct file *fp, const char __user *user, size_t size, loff_t *offs);

// I use a mutex to prevent concurrent writes during input operations.
DEFINE_MUTEX(my_mutex);

static const struct file_operations myfd_fops = {
	.owner = THIS_MODULE,
	.read = &myfd_read,
	.write = &myfd_write
};

static struct miscdevice myfd_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "reverse",
	.fops = &myfd_fops
};

char str[PAGE_SIZE];

ssize_t myfd_read(struct file *fp, char __user *user, size_t size, loff_t *offs)
{
	int ret;
	size_t str_len;
	char *tmp;

	mutex_lock(&my_mutex);
	str_len = strlen(str);
	tmp = kmalloc(sizeof(char) * (str_len + 1), GFP_KERNEL);
	if (!tmp) {
		mutex_unlock(&my_mutex);
		return -ENOMEM;
	}
	for (size_t i = 0; i < str_len; i++)
		tmp[i] = str[str_len - i - 1];
	tmp[str_len] = '\0';
	ret = simple_read_from_buffer(user, size, offs, tmp, str_len + 1);
	kfree(tmp);
	mutex_unlock(&my_mutex);
	return ret;
}

ssize_t myfd_write(struct file *fp, const char __user *user, size_t size, loff_t *offs)
{
	ssize_t res;

	mutex_lock(&my_mutex);
	res = simple_write_to_buffer(str, PAGE_SIZE - 1, offs, user, size);
	if (res >= 0)
		str[res++] = '\0';
	mutex_unlock(&my_mutex);
	return res;
}

static int __init myfd_init(void)
{
	int retval;

	retval = misc_register(&myfd_device);
	return retval;
}

static void __exit myfd_cleanup(void)
{
	misc_deregister(&myfd_device);
}

module_init(myfd_init);
module_exit(myfd_cleanup);
