// SPDX-License-Identifier: GPL-2.0
#include <linux/init.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/string.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("yoson");
MODULE_DESCRIPTION("fortytwo misc character device driver");
MODULE_VERSION("1.0");

static const char login[] = "yoson";
static const size_t login_len = sizeof(login) - 1;

static ssize_t ft_read(struct file *file, char __user *buf,
                             size_t count, loff_t *ppos)
{
	return simple_read_from_buffer(buf, count, ppos, login, login_len);
}

static ssize_t ft_write(struct file *filp, const char __user *buf,
                              size_t count, loff_t *ppos)
{
	char kbuf[login_len];

	if (count != login_len)
		return -EINVAL;
	if (copy_from_user(kbuf, buf, login_len))
		return -EFAULT;
	if (memcmp(kbuf, login, login_len))
		return -EINVAL;
	return login_len;
}

static const struct file_operations ft_fops = {
	.owner = THIS_MODULE,
	.read  = ft_read,
	.write = ft_write,
};

static struct miscdevice ft_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name  = "fortytwo",
	.fops  = &ft_fops,
	.mode  = 0666,
};

static int __init ft_init(void)
{
	int error;

	error = misc_register(&ft_misc_device);
	if (error) {
		pr_err("fortytwo: Failed to register misc device\n");
		return error;
	}
	pr_info("fortytwo: Successfully registered misc device /dev/%s (minor: %d)\n",
        	ft_misc_device.name, ft_misc_device.minor);
	return 0;
}

static void __exit ft_exit(void)
{
	misc_deregister(&ft_misc_device);
	pr_info("fortytwo: Successfully deregistered /dev/%s\n",
        	ft_misc_device.name);
}

module_init(ft_init);
module_exit(ft_exit);
