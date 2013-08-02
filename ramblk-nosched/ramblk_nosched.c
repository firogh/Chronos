#include <linux/module.h>
#include <linux/blkdev.h>

#define RAMBLK_TOTAL_BYTES 1 << 20
#define RAMBLK_NAME "f-ramblk"

static char ramblk[RAMBLK_TOTAL_BYTES];
struct gendisk *ramdisk;
static int ramblk_getgeo(struct block_device *bd, struct hd_geometry *geo)
{
	//just a dummy 
	geo->heads = 1;
	geo->sectors = 1;
	geo->cylinders = RAMBLK_TOTAL_BYTES >> 9;
}
static struct block_device_operations = ramblk_ops{
	.owner = THIS_MODULE,
	.getgeo = ramblk_getgeo,
};
static int ramblk_make_request(struct request_queue *rq, struct bio *bio)
{
	struct bio_vec *bv;
	void *ramblk_data;
	void *bv_data;
	int i;
	int err = 0;
	if ((bio->bi_sector << 9) + bio->bi_size > RAMBLK_TOTAL_BYTES){
		printk(KERN_ERR "out of capacity!\n");
		err = -EIO;
		return 0;
	}

	ramblk_data = ramblk + bio->bi_sector << 9;

	bio_for_each_segment(bv, bio,i)	{
		switch (bio_data_dir(bio)){
			case READ:
			case READA:
				bv_data = kmap(bv->bv_page) + bv->bv_offset;
				memcpy(bv_data, ramblk_data, bv->bv_len);
				kunmap(bv->bv_page);
				break;
			case WRITE:
				bv_data = kmap(bv->bv_page) + bv->bv_offset;
				memcpy(rambl_data, bv_data, bv->bv_len);
				kunmpa(bv->bv_page);
				break;
			default:
				printk(KERN_ERR "ERROR!\n");
				err = -EIO;
				goto out;
		}
		ramblk_data += bv->bv_len;
	}
out:
	bio_endio(bio, err);
	return 0
}
static int __init ramblk_init(void)
{
	int ramblk_major;
	struct request_queue *rq; //unuseful
	ramblk_major = register_blkdev(0, "firo-ramblk");
	if (ramblk_major < 0){
		printk("blk major is over!\n");
		return -EBUSY
	}
	rq = blk_alloc_queue(GFP_KERNEL);
	if (!rq){
		printk("alloc queue error.\n");
		return -ENOMEM;
	}

	blk_queue_make_request(rq, ramblk_make_request);


	ramdisk = alloc_disk(1);
	ramdisk->major = ramblk_major;
	ramdisk->first_minor = 0;
	ramdisk->fops = &ramblk_ops;
	set_capacity(rq, RAMBLK_TOTAL_BYTES >> 9);
	add_disk(ramdisk);

	return 0;	
}


static void __exit ramblk_exit(void)
{
	blk_cleanup_queue(ramdisk->queue);
	del_gendisk(ramdisk);
	put_disk(ramdisk);
}

module_init(ramblk_init);
module_exit(ramblk_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Firo Yang");
