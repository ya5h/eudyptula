#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/timer.h>
#include <linux/sysfs.h>
#include <linux/string.h>
#include <linux/kobject.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <linux/uaccess.h>

struct my_struct {
	char *data;
	struct semaphore sem;
};

#define DATA_SIZE 65536

int g_id_size = 13;
char g_id[12] = "fabbe86ebad5";
struct my_struct g_st;

ssize_t hello_read(struct kobject *kobj, struct kobj_attribute *attr, char *buff)
{
	int count = 0;

	if (down_interruptible(&g_st.sem))
		return -ERESTARTSYS;

	if ( !(strcmp(attr->attr.name, "id")) ) {
		count = sprintf(buff, "%s\n", g_id);
	}else if ( !(strcmp(attr->attr.name, "jiffies")) ) {
		count = sprintf(buff, "kernel jiffies: %ld\n", jiffies);
	}else if ( !(strcmp(attr->attr.name, "foo")) ) {
		count = strlen(g_st.data) + 1;
		strncpy(buff, g_st.data, count);
		memset(g_st.data,0,count);
	}else {
		up(&g_st.sem);
		return -ENODEV;
	}
		
	up(&g_st.sem);
	return count;
}

ssize_t hello_write(struct kobject *kobj, struct kobj_attribute *attr, const char *buff, size_t count)
{
	ssize_t retval = -EINVAL;
	char *kbuf = kzalloc(count, GFP_KERNEL);
	if (down_interruptible(&g_st.sem))
		return -ERESTARTSYS;

	retval = sprintf(kbuf,"%s", buff);

	if ( !(strcmp(attr->attr.name, "id")) ) {
		if (strncmp(kbuf, "fabbe86ebad5\n", count))
			retval = -EINVAL;
	}else if ( !(strcmp(attr->attr.name, "foo")) ) {
		retval = sprintf(g_st.data, "%s", kbuf);
	}else {
		retval = -ENODEV;
	}

	up(&g_st.sem);
	return retval;
}

static struct kobj_attribute foo_attribute =
	__ATTR(foo, 0644, hello_read, hello_write);
static struct kobj_attribute id_attribute =
	__ATTR(id, 0666, hello_read, hello_write);
static struct kobj_attribute jiffies_attribute =
	__ATTR(jiffies, 0444, hello_read, hello_write);

static struct attribute *attrs[] = {
	&foo_attribute.attr,
	&id_attribute.attr,
	&jiffies_attribute.attr,
	NULL,
};

static struct attribute_group hello_attr_grp = {
	.attrs = attrs,
};

static struct kobject *my_kobj;

static int __init hello(void)
{
	int retval = 0;
	pr_debug("Hello World!\n");

	my_kobj = kobject_create_and_add("eudyptula", kernel_kobj);
	if (!my_kobj) {
		printk(KERN_ALERT "hello: failed to create /sys/kernel/eudyptula\n");
		return -ENOMEM;
	}

	retval = sysfs_create_group(my_kobj, &hello_attr_grp);
	if (retval)
		kobject_put(my_kobj);
	
	g_st.data = kzalloc(DATA_SIZE, GFP_KERNEL);
	sema_init(&g_st.sem, 1);

	return retval;
}

static void __exit bye(void)
{
	pr_debug("unloding hello module\n");
	kfree(g_st.data);
	kobject_put(my_kobj);
}

module_init(hello);
module_exit(bye);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yash Shah");
MODULE_DESCRIPTION("hello world module");
