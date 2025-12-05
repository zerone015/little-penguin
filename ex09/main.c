// SPDX-License-Identifier: GPL-2.0
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/nsproxy.h>
#include <linux/mount.h>
#include <linux/dcache.h>
#include <linux/slab.h>
#include <linux/limits.h>
#include <linux/rbtree.h>
#include <../fs/mount.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("yoson");
MODULE_DESCRIPTION("/proc/mymounts — list mount points");

static void print_fullpath(struct seq_file *seq, struct path *path)
{
	char *buf, *str;

	buf = kmalloc(PATH_MAX, GFP_KERNEL);
	if (!buf)
		return;
	str = d_path(path, buf, PATH_MAX);
	if (!IS_ERR(str))
		seq_printf(seq, "%s", str);
	kfree(buf);
}

static int show_mounts(struct seq_file *seq, void *v)
{
	struct mount *mnt, *tmp;
	struct rb_root *mounts;
	struct super_block *sb;
	struct path path;

	mounts = &current->nsproxy->mnt_ns->mounts;
	rbtree_postorder_for_each_entry_safe(mnt, tmp, mounts, mnt_node) {
		sb = mnt->mnt.mnt_sb;
		path.mnt = &mnt->mnt;
		path.dentry = mnt->mnt.mnt_root;
		seq_printf(seq, "%s ", sb->s_id);
		print_fullpath(seq, &path);
		seq_putc(seq, '\n');
	}
	return 0;
}

static int mymounts_open(struct inode *inode, struct file *file)
{
	return single_open(file, show_mounts, NULL);
}

static const struct proc_ops mymounts_proc_ops = {
	.proc_open    = mymounts_open,
	.proc_read    = seq_read,
	.proc_lseek   = seq_lseek,
	.proc_release = single_release,
};

static int __init mymounts_init(void)
{
	if (!proc_create("mymounts", 0444, NULL, &mymounts_proc_ops))
		return -ENOMEM;
	pr_info("mymounts: /proc/mymounts created successfully\n");
	return 0;
}

static void __exit mymounts_exit(void)
{
	remove_proc_entry("mymounts", NULL);
	pr_info("mymounts: removed\n");
}

module_init(mymounts_init);
module_exit(mymounts_exit);
