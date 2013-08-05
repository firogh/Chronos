#include <linux/module.h>
#include <linux/semaphore.h>

MODULE_AUTHOR("Firo Yang");
MODULE_LICENSE("GPL");

static struct file_operations chrdev_ops = {
	.owner	= THIS_OWNER,
	.open	= chrdev_open,
	.read	= chrdev_read,
}

static struct semaphore chrdev_lock;
static wait_queue_head_t chrdev_wait;

static int __init chrdev_init(void)
{
	dev_t dev_id;
	int major, err;
	struct cdev *chrdev;
	struct class *class_firo;
	struct device *dev;

	err = alloc_chrdev_region(&dev_id, 0, 1, "firo-char");
	major = MAJOR(devid);
	printk("chrdev major %d\n", major);

	chrdev = cdev_alloc();
	cdev_init(chrdev, &chrdev_ops);

	err = cdev_add(chrdev, devid, 1o);
	if (err){
		printk("cdev add failed,err code %d\n", err);
		return -1;
	}
	
	class_firo = class_create(THIS_MODULE, "firo-class");
	if (IS_ERR(class_firo)){
		printk("class create failed.\n");
		return -1;
	}
	dev = deice_create(class_firo, NULL, devid, NULL, "firo-chrdev");
	printk("register chrdev. \n");
	sema_init(&chrdev_lock);
	init_wait_queue_head(&chrdev_wait);

	return 0;
}

