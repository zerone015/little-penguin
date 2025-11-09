// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/rwsem.h>
#include "fortytwo.h"

static char foo_buffer[MAX_FOO_LEN];
static size_t foo_size = 0;
static DECLARE_RWSEM(foo_rwsem); 

static ssize_t foo_read(struct file *file, char __user *buf, size_t len,
			loff_t *ppos)
{
	ssize_t ret;
	
	if (down_read_interruptible(&foo_rwsem))
		return -ERESTARTSYS;
	ret = simple_read_from_buffer(buf, len, ppos, foo_buffer, foo_size);
	up_read(&foo_rwsem);
	return ret;
}

static ssize_t foo_write(struct file *file, const char __user *buf, size_t len,
			 loff_t *ppos)
{
	ssize_t ret;

	if (down_write_killable(&foo_rwsem))
		return -ERESTARTSYS;
	ret = simple_write_to_buffer(foo_buffer, MAX_FOO_LEN, ppos, buf, len);
	if (ret > 0)
		foo_size = max_t(size_t, foo_size, *ppos); 
	up_write(&foo_rwsem);
	return ret;
}

const struct file_operations foo_fops = {
	.owner = THIS_MODULE,
	.read  = foo_read,
	.write = foo_write,
};
