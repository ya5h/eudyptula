#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>

struct identity{
	char  name[20];
	int   id;
	bool  busy;
	struct identity *next;
};

void identity_print(void);
static struct identity *head = NULL;
struct kmem_cache *my_cache = NULL;

int identity_create(char *name, int id)
{
	struct identity	*temp = kmem_cache_alloc(my_cache, GFP_KERNEL);
	
	if (temp) {
		strcpy(temp->name, name);
		temp->id = id;
		temp->busy = false;
		temp->next = head;
		head = temp;
		return 0;
	} else {
		pr_debug("YS: failed to allocated memory to new object\n");
		return -1;
	}
}

void identity_print(void)
{	
	if (head) {
		struct identity *tmp = NULL;
		for (tmp = head; tmp != NULL; tmp = tmp->next) {
			pr_debug("YS: Name:%s\tid:%d\n", tmp->name, tmp->id);
		}
	}else {
		pr_info("YS: Print failed: list is empty\n");
	}	
}

struct identity *identity_find(int id)
{
	struct identity *tmp = NULL;
	for (tmp = head; tmp != NULL; tmp = tmp->next) {
		if (tmp->id == id) 
			return tmp;
	}
	return NULL;
}

void identity_destroy(int id)
{
	struct identity *tmp = NULL, *prev = NULL;
	for (tmp = head; tmp != NULL; tmp = tmp->next) {
		if (tmp->id == id) {
			if (prev) {
				prev->next = tmp->next;
			} else {
				head = tmp->next;
			}
			kmem_cache_free(my_cache, tmp);
			return;
		}
		prev = tmp;
	}
	pr_info("YS: Destroy failed: unable to find any object with id:%d\n", id);
}

static void my_constructor(void *addr)
{
	memset(addr, 0, sizeof(struct identity));
}

static int __init hello(void)
{
	struct identity *temp;
	my_cache = kmem_cache_create("eudyptula", sizeof(struct identity), 0, 0, my_constructor);
	if (!my_cache) {
		pr_info("Error in kmem_cache_create\n");
		return -1;
	}
		

	identity_create("Alice", 1); 
	identity_create("Bob", 2); 
	identity_create("Dave", 3); 
	identity_create("Gena", 10);

	temp = identity_find(3);
	pr_debug("id 3 = %s\n", temp->name);

	temp = identity_find(42);
	if (temp == NULL)
		pr_debug("id 42 not found\n");
	
	identity_print();
	pr_debug("-------------------------------------\n");

	identity_destroy(2);
	identity_destroy(1);
	identity_destroy(10);
	identity_destroy(42);
	identity_destroy(3);

	identity_print();
	pr_debug("Hello World!\n");
	return 0;
}

static void __exit bye(void)
{
	struct identity *tmp = NULL, *curr = NULL;
	for (curr = head; curr != NULL; curr = tmp) {
		tmp = curr->next;
		kmem_cache_free(my_cache, curr);
	}
	kmem_cache_destroy(my_cache);
	pr_debug("unloding hello module\n");
}

module_init(hello);
module_exit(bye);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yash Shah");
MODULE_DESCRIPTION("hello world module");
