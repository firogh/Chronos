#include <linux/module.h>
#include <linux/printk.h>
#include <linux/blkdev.h>
#define SIMPLE_BLK_MINORS 1
#define SIMPLE_BLK_SECTOR_SIZE 512
#define SIMPLE_BLK_TOTAL_BYTES 1 << 20

static struct gendisk *simple_blk;
unsigned char simple_blk_disk[SIMPLE_BLK_TOTAL_BYTES];
struct block_device_operations simple_blk_ops = {
	.owner = THIS_MODULE,
};
static struct request_queue *simple_blk_queue;
static int simple_blk_major;
static void simple_blk_handle_request(struct request_queue *q)
{
	struct request *req;
	sector_t start;
	int size;
	req = blk_fetch_request(q);
	while (req){
		start = blk_rq_pos(req);
		size = blk_rq_cur_bytes(req);

		if (req->cmd_type != REQ_TYPE_FS){
			printk("skip non-fs request\n");
			blk_end_request_all(req, -EIO);
			continue;
		}
		if (SIMPLE_BLK_TOTAL_BYTES < (start << 9) + size){
			printk(KERN_ERR "Firo disk bad request: block= %llu, counts=%u\n", 
				(unsigned long long)start, size);
			goto err_end;
		}
		if (rq_data_dir(req) == READ){
			//copy from simple disk
			printk(KERN_DEBUG "copy from simple disk");
			memcpy(req->buffer, simple_blk_disk + (start << 9), size);
		}else{
			printk(KERN_DEBUG "copy from simple disk");
			memcpy(simple_blk_disk + (start << 9), req->buffer, size);
			//copyt from simple disk
		}
err_end:
		if(!__blk_end_request_cur(req, 0))
			req = blk_fetch_request(q);
	}
}


static int __init simple_blk_drv_init(void)
{
	int ret = 0;
	struct elevator_queue *elv;
	printk(KERN_DEBUG " firo blk driver \n");
	simple_blk_major = register_blkdev(0, "firo-sblk");
	if (simple_blk_major < 0){
		printk("major is over\n");
		return -EBUSY;
	}

	simple_blk_queue = blk_init_queue(simple_blk_handle_request, NULL);
	if (!simple_blk_queue){
		printk(KERN_ERR "blk_init_queue error\n");
		return -ENOMEM;
	}

	elv = simple_blk_queue->elevator;
	ret = elevator_change(simple_blk_queue, "noop");
	if (IS_ERR_VALUE(ret)){
		printk(KERN_ERR "elevator_change error, use default elevator\n");
		simple_blk_queue->elevator = elv;
	}
	

	simple_blk = alloc_disk(SIMPLE_BLK_MINORS);
	if (!simple_blk){
		printk(KERN_ERR "alloc_disk error\n");
		return -ENOMEM;
	}
	strcpy(simple_blk->disk_name, "firo-disk5");
	simple_blk->major = simple_blk_major;
	simple_blk->first_minor = 0;
	simple_blk->fops = &simple_blk_ops;
	simple_blk->queue = simple_blk_queue;  

	set_capacity(simple_blk, SIMPLE_BLK_TOTAL_BYTES >> 9);
	add_disk(simple_blk);

	printk(KERN_DEBUG " firo driver end \n");
	return 0;
}

static void __exit simple_blk_drv_clean(void)
{
	del_gendisk(simple_blk);
	put_disk(simple_blk);
	blk_cleanup_queue(simple_blk_queue);
	unregister_blkdev(simple_blk_major, "firo-sblk");
	printk(KERN_DEBUG "See you\n");
}
module_init(simple_blk_drv_init);
module_exit(simple_blk_drv_clean);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Firo Yang");
MODULE_DESCRIPTION("simple practice");

