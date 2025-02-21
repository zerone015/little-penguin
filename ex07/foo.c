// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/mutex.h>

MODULE_LICENSE("GPL");

static char foo_buf[PAGE_SIZE];
static size_t foo_buf_size;

DEFINE_MUTEX(foo_mutex);

static ssize_t foo_read(struct file *file, char __user *buf, size_t count, loff_t *f_pos)
{
	mutex_lock(&foo_mutex);
	if (*f_pos >= foo_buf_size)
	{
		mutex_unlock(&foo_mutex);
		return 0;
	}
	if (count > foo_buf_size - *f_pos)
		count = foo_buf_size - *f_pos;
	if (copy_to_user(buf, foo_buf + *f_pos, count))
	{
		mutex_unlock(&foo_mutex);
		return -EFAULT;
	}
	*f_pos += count;
	mutex_unlock(&foo_mutex);
	return count;
}

static ssize_t foo_write(struct file *file, const char __user *user_buf, size_t count, loff_t *f_pos)
{
	if (PAGE_SIZE < count)
		return -EINVAL;
	mutex_lock(&foo_mutex);
	if (copy_from_user(foo_buf, user_buf, count))
	{
		mutex_unlock(&foo_mutex);
		return -EFAULT;
	}
	foo_buf_size = count;
	mutex_unlock(&foo_mutex);
	return count;
}

const struct file_operations fops_foo = {
	owner : THIS_MODULE,
	read : foo_read,
	write : foo_write,
};
