#include <linux/module.h>
#include <linux/proc_fs.h>
#include <net/net_namespace.h>
#include <linux/stat.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>

#define HWF_PRIVATE_IPADDR_SIZE 250 //TODO: 255

#define HWF_PROC_FILE_LINE_SIZE	21
#define HWF_PROC_FILE_LINE_MIN_SIZE 17
#define HWF_PROC_FILE_SIZE	(HWF_PRIVATE_IPADDR_SIZE * HWF_PROC_FILE_LINE_SIZE)
#define HWF_PRIVATE_IPADDR_PREFFIX "192.168.199." //TODO: INT %pI4 Endian Network
//TODO: 255.255.255.0 htonl(0xFFFFFF00)

#define HWF_PROC_FILE_EOF_SIZE (HWF_PROC_FILE_LINE_MIN_SIZE * (HWF_PRIVATE_IPADDR_SIZE -1))

static int hwf_ipaddrs_bw[HWF_PRIVATE_IPADDR_SIZE];
static DEFINE_MUTEX(hwf_mutex);

static ssize_t hwf_file_read(struct file *f, char __user *buffer, size_t len, loff_t *offset)
{
	int j;
	char kbuf[HWF_PROC_FILE_SIZE]; "XXX.XXX.XXX.XXX XXXX"
	
	//TODO: 
	memset(kbuf, 0, sizeof(kbuf));

	//TODO: EOF
	if (*offset < 0 || *offset >= HWF_PROC_FILE_EOF_SIZE)
		return 0;
		
	//TODO: Remove the lock. 
	//printk("====================offset%d\n", *offset);
	mutex_lock(&hwf_mutex);
	for(j = 1; j < HWF_PRIVATE_IPADDR_SIZE; j++)
	{
		char tmp[50];
		memset(tmp, 0, sizeof(tmp));
		sprintf(tmp, "%s%d %d\n", HWF_PRIVATE_IPADDR_PREFFIX, j, hwf_ipaddrs_bw[j]);
		strcat(kbuf, tmp);
	}
	mutex_unlock(&hwf_mutex);
	
	if(copy_to_user(buffer, kbuf, strlen(kbuf)))
		return -EFAULT;
		
	*offset += strlen(kbuf);
	return strlen(kbuf);

}

// XXX.XXX.XXX.XXX XXX
static ssize_t hwf_file_write(struct file *f, const char __user * data, size_t len, loff_t * offset)
{
	int ip_index = 0, band_width = 0, n =0;
	char *p, kbuf[50];
	memset(kbuf, 0, sizeof(kbuf));

	long old_bw;
	//TODO: Check the len.
	copy_from_user(kbuf, data, len);
	printk("user input %s adn len is %d\n", kbuf, len);

	/* TODO: inet_aton -> check IP -> Get 0xFF -> check
	if ( 2 != (rv = sscanf("%s %u")) ) {
		return error;
	}*/
	
	p = strchr(kbuf, ' ');
	printk("p emty: %s\n", p);
	*p = '\0';
	p++;
	n = sscanf(p,"%d", &band_width);
	printk("band width: %d\n", band_width);
	if (band_width < 0 || band_width > 40960)
		return 0;
	p = strrchr(kbuf,'.');
	p++;
	printk("p dot: %s\n", p);
	n = sscanf(p, "%d", &ip_index);

	//Save current band width to hwf ipaddr bw arry
	//TODO: use macro
	if ( ip_index < 0 || ip_index > 250)
		return 4;
		
	mutex_lock(&hwf_mutex);
	old_bw = hwf_ipaddrs_bw[ip_index];
	if (old_bw == band_width){
		mutex_unlock(&hwf_mutex);
		return 4;
	}
		
	hwf_ipaddrs_bw[ip_index] = band_width; 
	mutex_unlock(&hwf_mutex);
	return sizeof(int);
}

//TODO: Add __read_mostly.
static struct file_operations hwf_file_ops = {
	.owner	= THIS_MODULE,
	.read	= hwf_file_read,
	.write	= hwf_file_write,
};

int __init hwf_proc_bw_init(void)
{
	struct proc_dir_entry *entry;

	//TODO: -X
	entry = proc_create("hwf-firo", S_IRWXUGO, init_net.proc_net, &hwf_file_ops);
	
	//TODO: If failed.

	return 0;
}

void __exit hwf_proc_bw_exit(void)
{
	remove_proc_entry("hwf-firo", init_net.proc_net);
}

module_init(hwf_proc_bw_init);
module_exit(hwf_proc_bw_exit);

//TODO: move to the head
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Firo Yang <firo@gmail.com>");  //TODO
MODULE_DESCRIPTION("Sample proc practice for hiwifi band width control.");
