#include <linux/module.h>
#include <linux/printk.h>
#include <linux/blkdev.h>
#define SIMPLE_BLK_MINORS 1
#define SIMPLE_BLK_SECTOR_SIZE 512
#define SIMPLE_BLK_TOTAL_BYTES 16 << 20

static struct gendisk *simple_blk;
struct block_device_operations simple_blk_ops = {
	.owner = THIS_MODULE,
};
static struct request_queue *simple_blk_queue;
static simple_blk_major;
static void simple_blk_handle_request(struct request_queue *q)
{
	struct request *req;
	sector_t start;
	int size;
	req = blk_fetch_request(q);
	while (req){
		start = blk_rq_pos(req);
		size = blk_rq_cur_bytes(req);
		if (SIMPLE_BLK_TOTAL_BYTES < start + size){
			printk(KERN_ERR "Firo disk bad request block= %llu, counts=%u\n", 
				(unsigned long long)start, size);
			goto err_out;
		}
		if (rq_data_dir(req) == READ){
			//copy to simple disk
			printk(KERN_DEBUG "copy to simple disk");
		}else{
			printk(KERN_DEBUG "copy from simple disk");
			//copyt from simple disk
		}
err_out:
		if(!__blk_end_request_cur(req, 0))
			req = blk_fetch_request(q);
	}
}


static int __init simple_blk_drv_init(void)
{
	printk(KERN_DEBUG " firo blk driver \n");
	int ret = 0;
	struct gendisk *simple_blk;
	simple_blk_major = register_blkdev(0, "firo-sblk");
	if (simple_blk_major == -ENOMEM | simple_blk_major == -EBUSY)
		return 0;
	simple_blk = alloc_disk(SIMPLE_BLK_MINORS);
	if (!simple_blk){
		ret = -ENOMEM;
		goto err_out;
	}
	strcpy(simple_blk->disk_name, "f-disk");
	simple_blk->major = simple_blk_major;
	simple_blk->first_minor = 0;
	simple_blk->fops = &simple_blk_ops;
	simple_blk_queue = blk_init_queue(simple_blk_handle_request, NULL);
	if (!simple_blk_queue)
	{
		ret = -ENOMEM;
		goto err_queue;
	}
	simple_blk->queue = simple_blk_queue;  
	set_capacity(simple_blk, SIMPLE_BLK_TOTAL_BYTES >> 9);
	add_disk(simple_blk);
	printk(KERN_DEBUG " firo driver end \n");
err_queue:
	del_gendisk(simple_blk);
	put_disk(simple_blk);
err_out:
	unregister_blkdev(simple_blk_major, "firo-sblk");
	return ret;
}

static void __exit simple_blk_drv_clean(void)
{
	blk_cleanup_queue(simple_blk_queue);
	del_gendisk(simple_blk);
	put_disk(simple_blk);
	unregister_blkdev(simple_blk_major, "firo-sblk");
	printk(KERN_DEBUG " See you\n");
}
module_init(simple_blk_drv_init);
module_exit(simple_blk_drv_clean);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Firo Yang");
MODULE_DESCRIPTION("simple practice");

