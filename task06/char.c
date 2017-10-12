#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/usb.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
	
struct my_dev {
	unsigned long size; 		/* amount of data stored here */
	struct cdev cdev; 		/* Char device structure */
};

struct my_dev *dev;
unsigned int my_minor, my_major, count = 1;
dev_t devno;

int hello_open(struct inode *inode, struct file *filp)
{
	struct my_dev *dev; 					/* device information */
	pr_debug("Hello: open method called\n");
	dev = container_of(inode->i_cdev, struct my_dev, cdev);
	filp->private_data = dev; 				/* for other methods */
	return 0;
}

int hello_release(struct inode *inode, struct file *filp)
{
	pr_debug("Hello: release method called\n");
	return 0;
}

ssize_t hello_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos)
{
	struct my_dev *dev = filp->private_data;
	ssize_t retval = 0;

	pr_debug("Hello: read method called\n");
	if (*f_pos >= dev->size)
		return retval;
	if (*f_pos + count > dev->size)
		count = dev->size - *f_pos;

	if (copy_to_user(&buff, "1asf1233", count)) {
		pr_debug("Hello: Error in copying data to user buffer\n");
		return -EFAULT;
	}
	*f_pos += count;
	retval = count;

	return retval;
}

ssize_t hello_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos)
{
	struct my_dev *dev = filp->private_data;
	ssize_t retval = -ENOMEM;
	char *buf;

	buf = kmalloc(count, GFP_KERNEL);
	pr_debug("Hello: write method called\n");
	if (copy_from_user(buf, buff, count)) {
		pr_debug("Hello: Error in copying data from user buffer\n");
		return -EFAULT;
	}
	*f_pos += count;
	retval = count;

	/* update the size */
	if (dev->size < *f_pos)
		dev->size = *f_pos;

	return retval;
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

	dev = kzalloc(sizeof(struct my_dev), GFP_KERNEL);
	ret = alloc_chrdev_region(&devno, my_minor, count, "eudyptula");
	my_major = MAJOR(devno);
	if (ret < 0) {
		pr_debug("Hello: Cannot get Major no. %d\n",my_major);
		return ret;
	}
	
	cdev_init(&dev->cdev, &hello_fops);
	dev->cdev.owner = THIS_MODULE;
	//dev->cdev.ops = &hello_fops;

	ret = cdev_add(&dev->cdev, devno, count);
	if (ret < 0) {
		pr_debug("Hello: cdev_add failed!!\n");
		unregister_chrdev_region(devno, count);
		return ret;
	}

	return 0;
}

static void __exit bye(void)
{
	pr_debug("Hello: unloding the module\n");

	unregister_chrdev_region(devno, count);
	cdev_del(&dev->cdev);
}

module_init(hello);
module_exit(bye);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yash Shah");
MODULE_DESCRIPTION("hello world module");
