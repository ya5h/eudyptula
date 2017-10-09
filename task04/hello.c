#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/module.h>

static int __init hello(void)
{
	pr_debug("Hello World!\n");
	return 0;
}

static void __exit bye(void)
{
	pr_debug("unloding hello module\n");
}

module_init(hello);
module_exit(bye);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yash Shah");
MODULE_DESCRIPTION("hello world module");
