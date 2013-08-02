#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/uaccess.h>
static char data[1024 * 8] = {0};
static ssize_t sim_ps_read_tasklist(struct file *file, char __user *buf, size_t size, loff_t *offset)
{
	char tmp[128] = {0};
	//static int i = 0;
	struct task_struct *p;
	rcu_read_lock();
	for_each_process(p){
		sprintf(tmp, "%d\t\t%d\t\t\t%s\n",
				p->pid,p->parent->pid,p->comm);
		//printk("%s", tmp);
		strcat(data, tmp);
		memset(tmp, 0, sizeof(tmp));
		
	}
	rcu_read_unlock();
	printk("offset %d\n", (int) *offset);
	if (*offset < 0)
		return 0;
	if (copy_to_user(buf, data, strlen(data)))
		return -EFAULT;
	*offset += strlen(data);
	return strlen(buf);
}

struct file_operations proc_fops = {
	.read = sim_ps_read_tasklist, 
};


static int __init sim_ps_init(void)
{
	struct proc_dir_entry *entry;
	entry = proc_create("sim_ps", 0444, NULL, &proc_fops);
	if (entry == 0)
	{
		printk(KERN_ERR "create pro entry failed!\n");
		return -1;
	}
	return 0;
}

static void __exit sim_ps_exit(void)
{
	remove_proc_entry("sim_ps", NULL);
}

module_init(sim_ps_init);
module_exit(sim_ps_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Firo Yang");
