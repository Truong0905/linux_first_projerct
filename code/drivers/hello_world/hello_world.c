#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#define OK 0

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Carlos Cardoso Dias");
MODULE_DESCRIPTION("A simple hello world Linux module.");
MODULE_VERSION("0.01");

static int __init lkm_hello_world_init(void) {
  printk(KERN_INFO "Hello, World!\n");
  return OK;
}

static void __exit lkm_hello_world_exit(void) {
  printk(KERN_INFO "Goodbye, World!\n");
}

module_init(lkm_hello_world_init);
module_exit(lkm_hello_world_exit);
