#include <linux/module.h>

static int main_init(void) {
	printk(KERN_INFO "Hello world !\n");
	return 0;
}

static void main_exit(void) {
	printk(KERN_INFO "Cleaning up module.\n");
}

module_init(main_init);
module_exit(main_exit);

MODULE_LICENSE("GPL");
