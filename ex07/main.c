// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/debugfs.h>
#include <linux/jiffies.h>

MODULE_LICENSE("GPL");

extern const struct file_operations fops_id;
extern const struct file_operations fops_foo; 

static struct dentry *root_dir;
static u64 jiff;

static int main_init(void)
{
	root_dir = debugfs_create_dir("fortytwo", NULL);
	if (!root_dir)
		return -1;
	if (!debugfs_create_file("id", 0666, root_dir, NULL, &fops_id) ||
			!debugfs_create_file("foo", 0644, root_dir, NULL, &fops_foo)) {
		debugfs_remove_recursive(root_dir);	
		return -1;
	}
	jiff = get_jiffies_64();
	debugfs_create_u64("jiffies", 0444, root_dir, &jiff);
	return 0;
}

static void main_exit(void)
{
	debugfs_remove_recursive(root_dir);	
}

module_init(main_init);
module_exit(main_exit);
