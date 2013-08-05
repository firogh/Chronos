#include <linux/module.h>
#include <linux/semaphore.h>
#include <linux/device.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>


static struct semaphore chrdev_lock;
static wait_queue_head_t chrdev_wait;
static struct timer_list chrdev_timer;
static int flag;
struct cdev *chrdev;
dev_t dev_id;
struct class *class_firo;

static int work_routine(int i)
{
	printk("Waiting...\n");
	wait_event_interruptible(chrdev_wait, flag != 0);
	printk("Here we go.\n");
	flag = 0;
	return i + 10;
}

static void chrdev_handler(unsigned long data)
{
	printk("in handler function.\n");
	flag = 1;
	wake_up_interruptible(&chrdev_wait);
	printk("Waked up\n");
}

static int chrdev_open(struct inode *inode, struct file *file)
{
	try_module_get(THIS_MODULE);
	return 0;
}
static int chrdev_release(struct inode *inode, struct file *file)
{
	module_put(THIS_MODULE);
	return 0;
}

static ssize_t chrdev_read(struct file *file, char __user *buf, size_t size, loff_t * offset)
{
	int ret = 0;
	printk("chrdev read is invoking.\n");
	if (down_interruptible(&chrdev_lock))
		return -ERESTARTSYS;
	init_timer(&chrdev_timer);
	chrdev_timer.expires = jiffies + 5 * HZ;
	chrdev_timer.data = 0;
	chrdev_timer.function = chrdev_handler;
	add_timer(&chrdev_timer);

	ret = work_routine(0);
	printk("rentun vlaue is %d.\n", ret);
	del_timer(&chrdev_timer);
	copy_to_user(buf, (char *)&ret, sizeof(ret));

	up(&chrdev_lock);
	return sizeof(ret);
}

static struct file_operations chrdev_ops = {
	.owner		= THIS_MODULE,
	.open		= chrdev_open,
	.read		= chrdev_read,
	.release	= chrdev_release
};


static int __init chrdev_init(void)
{
	int major, err;
	struct device *dev;

	err = alloc_chrdev_region(&dev_id, 0, 1, "firo-char");
	major = MAJOR(dev_id);
	printk("chrdev major %d\n", major);

	chrdev = cdev_alloc();
	cdev_init(chrdev, &chrdev_ops);

	err = cdev_add(chrdev, dev_id, 1);
	if (err){
		printk("cdev add failed,err code %d\n", err);
		return -1;
	}
	
	class_firo = class_create(THIS_MODULE, "firo-class");
	if (IS_ERR(class_firo)){
		printk("class create failed.\n");
		return -1;
	}
	dev = device_create(class_firo, NULL, dev_id, NULL, "firo-chrdev");
	printk("register chrdev. \n");
	sema_init(&chrdev_lock, 1);
	init_waitqueue_head(&chrdev_wait);

	return 0;
}
static int __exit chrdev_exit(void)
{
	device_destroy(&class_firo, dev_id);
	class_destroy(&class_firo);
	cdev_del(chrdev);
	unregister_chrdev_region(dev_id, 1);
}

module_init(chrdev_init);
module_exit(chrdev_exit);
MODULE_AUTHOR("Firo Yang");
MODULE_LICENSE("GPL");
