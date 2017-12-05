#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/usb.h>
#include <linux/hid.h>

static struct usb_device_id skel_table[ ] = {
	{ USB_DEVICE_INFO(USB_INTERFACE_CLASS_HID, USB_INTERFACE_SUBCLASS_BOOT, USB_INTERFACE_PROTOCOL_KEYBOARD) },
	{ }
};
MODULE_DEVICE_TABLE(usb, skel_table);

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
