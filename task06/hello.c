#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/miscdevice.h>
#include <linux/usb.h>
#include <linux/string.h>

static struct miscdevice my_dev;
int g_size = 14;

int hello_open(struct inode *inode, struct file *filp)
{
	pr_debug("Hello: open method called\n");
	return 0;
}

int hello_release(struct inode *inode, struct file *filp)
{
	pr_debug("Hello: release method called\n");
	return 0;
}

ssize_t hello_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos)
{
        if (*f_pos >= g_size)
                return 0;
        if (*f_pos + count > g_size)
                count = g_size - *f_pos;

	if (copy_to_user(buff, "fabbe86ebad5\n", count)) {
		pr_debug("Hello: Error in copying data to user buffer\n");
		return -EFAULT;
	}
	*f_pos += count;
	return count;
}

ssize_t hello_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos)
{
	char *k_buf =  kzalloc(count, GFP_KERNEL);

	if (copy_from_user(k_buf, buff, count)) {
		pr_debug("Hello: Error in copying data from user buffer\n");
		return -EFAULT;
	}

	if (strcmp(k_buf, "fabbe86ebad5"))
		return -EINVAL;
	else
		return count;
}

const struct file_operations hello_fops = {
	.owner = THIS_MODULE,
	.read = hello_read,
	.write = hello_write,
	.open = hello_open,
	.release = hello_release
};

static int __init hello(void)
{
	int ret = 0;
	pr_debug("Hello: init called!\n");

	my_dev.minor = MISC_DYNAMIC_MINOR;
	my_dev.name = "eudyptula";
	my_dev.fops = &hello_fops;
	ret = misc_register(&my_dev);
	if (ret) return ret;
		printk("Hello: got minor %i\n",my_dev.minor);
	return 0;

}

static void __exit bye(void)
{
	pr_debug("Hello: unloding the module\n");
	misc_deregister(&my_dev);
}

module_init(hello);
module_exit(bye);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yash Shah");
MODULE_DESCRIPTION("hello world module");
