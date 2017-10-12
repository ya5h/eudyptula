#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/debugfs.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/usb.h>
#include <linux/timer.h>

struct my_struct {
	char *data;
	struct semaphore sem;
	long long size;
};

#define DATA_SIZE 65536

static struct dentry *dir = 0;
int g_size = 13;
int g_size_jf = 1;
struct my_struct g_st;

int hello_open(struct inode *inode, struct file *filp)
{
	pr_debug("Hello: open method called...path = %s\n", filp->f_path.dentry->d_iname);
	return 0;
}

int hello_release(struct inode *inode, struct file *filp)
{
	pr_debug("Hello: release method called\n");
	return 0;
}

ssize_t hello_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos)
{
	char *kbuf = NULL;
	int retval = 0;
	if (down_interruptible(&g_st.sem))
		return -ERESTARTSYS;

	if ( !(strcmp(filp->f_path.dentry->d_iname, "id")) ) {
		if (*f_pos >= g_size)
			goto out;
		if (*f_pos + count > g_size)
			count = g_size - *f_pos;
		if (copy_to_user(buff, "fabbe86ebad5\n", count)) {
			pr_debug("Hello: Error in copying data to user buffer\n");
			retval = -EFAULT;
			goto out;
		}
		*f_pos += count;
		retval = count;
	}else if ( !(strcmp(filp->f_path.dentry->d_iname, "jiffies")) ) {
		if (*f_pos >= g_size_jf)
			goto out;
		if (*f_pos + count > g_size_jf)
			count = g_size_jf - *f_pos;
		kbuf = kzalloc(50, GFP_KERNEL);
		snprintf(kbuf, 50,  "kernel jiffies: %ld\n", jiffies);
		count = strlen(kbuf);
		if (copy_to_user(buff, kbuf, count)) {
                        pr_debug("Hello: Error in copying data to user buffer...jiffies\n");
                        retval = -EFAULT;
			goto out;
                }
		g_size_jf = count;
		*f_pos += count;
		kfree(kbuf);
		retval = count;
	}else if ( !(strcmp(filp->f_path.dentry->d_iname, "foo")) ) {
		if (*f_pos >= g_st.size)
			goto out;
		if (*f_pos + count > g_st.size)
			count = g_st.size - *f_pos;
		if (copy_to_user(buff, g_st.data, count)) {
                        pr_debug("Hello: Error in copying data to user buffer...foo\n");
                        retval = -EFAULT;
			goto out;
                }
		memset(g_st.data,0,count);
		*f_pos += count;
		retval = count;
	}
out:
	up(&g_st.sem);
	return retval;	
}

ssize_t hello_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos)
{
	ssize_t retval = -EINVAL;
	if (down_interruptible(&g_st.sem))
		return -ERESTARTSYS;

	if (copy_from_user(g_st.data, buff, count)) {
		pr_debug("Hello: Error in copying data from user buffer\n");
		retval = -EFAULT;
		goto out;
	}

	if ( !(strcmp(filp->f_path.dentry->d_iname, "id")) ) {
		if (count != g_size || strncmp(g_st.data, "fabbe86ebad5", g_size - 1))
			retval = -EINVAL;
		else
			retval = count;
	}else if ( !(strcmp(filp->f_path.dentry->d_iname, "foo")) ) {
		*f_pos += count;
		retval = count;

		if (g_st.size < *f_pos)
			g_st.size = *f_pos;
	}
out:
	up(&g_st.sem);
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
	struct dentry *de_ret;
	
	pr_debug("Hello World!\n");

	dir = debugfs_create_dir("eudyptula", 0);
	if (!dir) {
		// Abort module load.
		printk(KERN_ALERT "hello: failed to create /sys/kernel/debug/eudyptula\n");
		return -1;
	}

	de_ret = debugfs_create_file("id", S_IRWXUGO, dir, NULL, &hello_fops);
	if (!de_ret) {
		printk(KERN_ALERT "hello: failed to create /sys/kernel/debug/eudyptula/id\n");
		return -1;
	}
	
	de_ret = debugfs_create_file("jiffies", S_IRUGO, dir, NULL, &hello_fops);
	if (!de_ret) {
		printk(KERN_ALERT "hello: failed to create /sys/kernel/debug/eudyptula/jiffies\n");
		return -1;
	}
	
	de_ret = debugfs_create_file("foo", S_IRUGO, dir, NULL, &hello_fops);
	if (!de_ret) {
		printk(KERN_ALERT "hello: failed to create /sys/kernel/debug/eudyptula/foo\n");
		return -1;
	}
	
	g_st.data = kzalloc(DATA_SIZE, GFP_KERNEL);
	sema_init(&g_st.sem, 1);

	return 0;
}

static void __exit bye(void)
{
	pr_debug("unloding hello module\n");
	kfree(g_st.data);
	debugfs_remove_recursive(dir);
}

module_init(hello);
module_exit(bye);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yash Shah");
MODULE_DESCRIPTION("hello world module");
