#include <linux/module.h>
#include <linux/printk.h>

static int __init init_hello(void)
{
	pr_info("USB Keyboard connected\n");
	return 0;
}

static void __exit cleanup_hello(void)
{
}

module_init(init_hello);
module_exit(cleanup_hello);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("yoson");
MODULE_DESCRIPTION("Test module for simulating USB keyboard hotplug event");
