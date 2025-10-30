#include <linux/module.h>
#include <linux/printk.h>

static int __init init_hello(void)
{
	printk(KERN_INFO "Hello world!\n");

	return 0;
}

static void __exit cleanup_hello(void)
{
	printk(KERN_INFO "Cleaning up module.\n");
}

module_init(init_hello);
module_exit(cleanup_hello);

MODULE_LICENSE("GPL");
