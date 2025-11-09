// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>      
#include <linux/fs.h>          
#include <linux/uaccess.h>
#include <linux/string.h>
#include "fortytwo.h"

static ssize_t id_read(struct file *file, char __user *buf,
                       size_t count, loff_t *ppos)
{
    return simple_read_from_buffer(buf, count, ppos, LOGIN, LOGIN_LEN);
}

static ssize_t id_write(struct file *file, const char __user *buf,
                        size_t count, loff_t *ppos)
{
	char kbuf[LOGIN_LEN];

	if (count != LOGIN_LEN)
		return -EINVAL;
	if (copy_from_user(kbuf, buf, LOGIN_LEN))
		return -EFAULT;
	if (memcmp(kbuf, LOGIN, LOGIN_LEN))
		return -EINVAL;
	return LOGIN_LEN;
}

const struct file_operations id_fops = {
	.owner = THIS_MODULE,
	.read  = id_read,
	.write = id_write,
};
