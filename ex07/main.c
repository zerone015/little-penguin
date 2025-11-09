// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/init.h>
#include <linux/debugfs.h>
#include <linux/err.h>
#include <linux/fs.h>
#include "fortytwo.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("yoson");
MODULE_DESCRIPTION("fortytwo debugfs module (id, jiffies, foo)");
MODULE_VERSION("1.0");

static struct dentry *dir;

static int __init ft_init(void)
{
	struct dentry *tmp;
	
	dir = debugfs_create_dir("fortytwo", NULL);
	if (IS_ERR(dir))
		return PTR_ERR(dir);

	tmp = debugfs_create_file("id", 0666, dir, NULL, &id_fops);
	if (IS_ERR(tmp))
		goto fail;

	tmp = debugfs_create_file("jiffies", 0444, dir, NULL, &jiffies_fops);
	if (IS_ERR(tmp))
		goto fail;

	tmp = debugfs_create_file("foo", 0644, dir, NULL, &foo_fops);
	if (IS_ERR(tmp))
		goto fail;

	pr_info("fortytwo_debugfs: loaded\n");
	return 0;

fail:
	debugfs_remove(dir);
	return PTR_ERR(tmp);
}

static void __exit ft_exit(void)
{
	debugfs_remove(dir);
	pr_info("fortytwo_debugfs: unloaded\n");
}

module_init(ft_init);
module_exit(ft_exit);
