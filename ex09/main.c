// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/nsproxy.h>
#include <linux/rbtree.h>
#include <linux/path.h>
#include "../fs/mount.h"

MODULE_LICENSE("GPL");

static int main_show(struct seq_file *m, void *p)
{
	struct mount *mnt;
	struct mount *tmp;
	struct rb_root *mounts;
	struct path path;

	mounts = &current->nsproxy->mnt_ns->mounts;
	rbtree_postorder_for_each_entry_safe(mnt, tmp, mounts, mnt_node) {
		seq_printf(m, "%s ", mnt->mnt_devname);
		path.mnt = &mnt->mnt;
		path.dentry = mnt->mnt.mnt_root;
		seq_path(m, &path, "");
		seq_putc(m, '\n');
	}
	return 0;
}

static int main_open(struct inode *inode, struct file *file)
{
	return single_open(file, main_show, NULL);
}

static const struct proc_ops proc_fops = {
        .proc_open = main_open,
        .proc_read = seq_read,
	.proc_lseek  = seq_lseek,
        .proc_release = single_release,
};

static int main_init(void)
{
	if (!proc_create("mymounts", 0444, NULL, &proc_fops))
		return -1;
	return 0;
}

static void main_exit(void)
{
	remove_proc_entry("mymounts", NULL);
}

module_init(main_init);
module_exit(main_exit);
