// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>

#define USERNAME	"yoson"
#define USERNAME_LEN	5

MODULE_LICENSE("GPL");

static ssize_t id_read(struct file *file, char __user *buf, size_t count, loff_t *f_pos)
{
	if (*f_pos >= USERNAME_LEN)
		return 0;
	if (count > USERNAME_LEN - *f_pos)
		count = USERNAME_LEN - *f_pos;
	if (copy_to_user(buf, USERNAME, count))
		return -EFAULT;
	*f_pos += count;
	return count;
}

static ssize_t id_write(struct file *file, const char __user *user_buf, size_t count, loff_t *f_pos)
{
	char buf[USERNAME_LEN];

	if (USERNAME_LEN != count)
		return -EINVAL;
	if (copy_from_user(buf, user_buf, count))
		return -EFAULT;
	if (memcmp(buf, USERNAME, count))
		return -EINVAL;
	return count;
}

const struct file_operations fops_id = {
	owner : THIS_MODULE,
	read : id_read,
	write : id_write,
};
