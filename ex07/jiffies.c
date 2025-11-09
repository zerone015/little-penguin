// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/jiffies.h>
#include <linux/uaccess.h>
#include "fortytwo.h"

static ssize_t jiffies_read(struct file *file, char __user *buf, size_t len,
			    loff_t *ppos)
{
	char kbuf[22]; 
	int n;

	n = snprintf(kbuf, sizeof(kbuf), "%lu\n", jiffies);
	return simple_read_from_buffer(buf, len, ppos, kbuf, n);
}

const struct file_operations jiffies_fops = {
	.owner   = THIS_MODULE,
	.read    = jiffies_read,
};
