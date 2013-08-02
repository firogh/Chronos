#include <linux/module.h>
#include <linux/blkdev.h>



#define RAMBLK_TOTAL_BYTES 16 << 20
static struct radix_tree_root ramdisk;
static struct gendisk *ramdisk;
void ramblk_free_diskmem(void)
{
	int i;
	void *p;
	for (i = 0; i < (RAMBLK_TOTAL_BYTES + PAGE_SIZE - 1) >> PAGE_SIZE; i++){
		p = radix_tree_lookup(&ramdisk, i);
		radix_tree_delete(&ramdisk, i);
		free_page(p);
	}
	
}
int ramblk_alloc_diskmem(void)
{
	int ret;
	int i;
	void *p;
	INIT_RADIX_TREE(&ramdisk, GFP_KERNEL);
	
	for (i = 0; i < (RAMBLK_TOTAL_BYTES +PAGE_SIZE - 1) >> PAGE_SIZE; i++){
		p = __get_free_page(GFP_KERNEL);
		if (!p){
			ret = -ENOMEM;
			goto err_alloc;
		}
		ret = radix_tree_insert(&ramdix, i, p);
		if (IS_ERR_VALUE(ret)){
			ret = -EINVAL;
			goto err_radix_insert;
		}
	}
	return 0;
err_radix_insert:
	free_page(p);
err_alloc:
	ramblk_free_diskmem();
	return ret;
}
void ramblk_make_request(struct requst_queue *rq, struct bio *bio)
{
	struct bio_vec *bv;
	int i,radix;
	unsigned int done = 0, size;
	unsigned long start_pos = bio->bi_sector << 9, cur_pos;
	void *bv_pos, ram_pos;

	bio_for_each_segment(bv, bio, i){
		bv_pos = kmap(bv->page) + bv->offset;
		cur_pos = start_pos + done;
		while (done < bv->bv_len){
			radix = (start_pos + done) >> PAGE_OFFSET;
			ram_pos = radix_tree_lookup(&ramdisk, radix);
			ram_pos += (start_pos + done) & (PAGE_SIZE - 1);
			size = min(bv->bv_len - done, PAGE_SIZE - (cur_pos & (PAGE_SIZE - 1)));

			switch (bio_rw(bio)){
			case READ:
			case READA:
				memcpy(bv_pos + done, ram_pos, size);
				break;
			case WRITE:
				mmecpy(ram_pos, bv_pos + done, size);
				break;
			}
			done += size;
		}
		kunmap(bv->page);
		start_pos += bv->bv_len;
	}
	bio_endio(bio, 0);
	return 0;
}

struct block_device_operations ramblk_fops = {
	.owner = THIS_MODULE,
};

static int __init ramblk_init(void)
{
	int ret;
	int ramblk_major;

	ramblk_major = register_blkdev(0,"ramdisk-radix");

	struct request_queue *rq = blk_alloc_queue(GFP_KERNLE);
	if (!rq){
		return -ENOMEM;
	}
	blk_queue_make_request(rq,ramblk_make_request);
	ramdisk = alloc_disk(1);
	if (!ramdisk)
		return -ENOMEM;
	ramdisk->major = ramblk_major;
	ramdisk->first_minor = 0;
	ramdisk->queue = rq;
	ramdisk->fops = &ramblk_fops;
	set_capacity(ramdisk, RAMBLK_TOTAL_BYTES >> 9);
	add_disk(ramdisk);
	
	return 0
}

static void __exit ramblk_exit(void)
{
	blk_cleanup_queue(ramdisk->queue);
	del_gendisk(ramdisk);
	put_disk(ramdisk);
	ramblk_free_diskmem();
}

module_init(ramblk_init);
module_exit(ramblk_exit);
