#include <errno.h>
#include <common.h>
#include <part.h>
#include <mmc.h>
#include <cloner/cloner.h>
#include "burn_printf.h"


#define MMC_BYTE_PER_BLOCK 512

struct mmc_param *mmc_args;

static int clmd_mmc_init(struct cloner *cloner, void *args, void *mdata)
{
	mmc_args = (struct mmc_param *)args;
	if(!mmc_args)
	{
		printf("Not found mmc parameters\n");
		return -EINVAL;
	}
	int dev = 0;
	struct mmc *mmc = find_mmc_device(dev);
	uint32_t blk, blk_end, blk_cnt;
	uint32_t erase_cnt = 0;
	int timeout = 30000;
	int i;
	int ret;

	if (!mmc) {
		printf("no mmc device at slot %x\n", dev);
		return -ENODEV;
	}

	ret = mmc_init(mmc);
	if (ret) {
		printf("ERROR: MMC Init error\n");
		return -EPERM;
	}

	if (mmc_getwp(mmc) == 1) {
		printf("Error: card is write protected!\n");
		return -EPERM;
	}

	if (mmc_args->mmc_erase == MMC_ERASE_ALL) {
		blk = 0;
		blk_cnt = mmc->capacity / MMC_BYTE_PER_BLOCK;

		BURNNER_PRI("MMC erase: dev # %d, start block # %d, count %u ... \n",
				dev, blk, blk_cnt);

		ret = mmc->block_dev.block_erase(dev, blk, blk_cnt);
		if (!ret) {
			BURNNER_PRI("Error: mmc erase error\n");
			return -EIO;
		}

		BURNNER_PRI("mmc all erase ok, blocks %d\n", blk_cnt);
		return 0;
	} else if (mmc_args->mmc_erase != MMC_ERASE_PART) {
		return -EINVAL;
	}

	/*mmc part erase */
	erase_cnt = (mmc_args->mmc_erase_range_count > MMC_ERASE_CNT_MAX) ?
		MMC_ERASE_CNT_MAX : mmc_args->mmc_erase_range_count;

	for (i = 0; erase_cnt > 0; i++, erase_cnt--) {
		blk = mmc_args->mmc_erase_range[i].start / MMC_BYTE_PER_BLOCK;
		if(mmc_args->mmc_erase_range[i].end == -1){
			blk_cnt = mmc->capacity / MMC_BYTE_PER_BLOCK - blk ;
		}else{
			blk_end = mmc_args->mmc_erase_range[i].end / MMC_BYTE_PER_BLOCK;
			blk_cnt = blk_end - blk ;
		}

		BURNNER_PRI("MMC erase: dev # %d, start block # 0x%x, count 0x%x ... \n",
				dev, blk, blk_cnt);

		ret = mmc->block_dev.block_erase(dev, blk, blk_cnt);
		if (!ret) {
			printf("Error: mmc erase error\n");
			return -EIO;
		}

		BURNNER_PRI("mmc part erase, part %d ok\n", i);

	}
	BURNNER_PRI("mmc erase ok\n");
	return 0;
}

int clmd_mmc_write(struct cloner *cloner, int sub_type, void *ops_data)
{
#define MMC_BYTE_PER_BLOCK 512
	int dev = sub_type;
	struct mmc *mmc = find_mmc_device(dev);
	u32 blk = (cloner->cmd->write.partition + cloner->cmd->write.offset)/MMC_BYTE_PER_BLOCK;
	u32 cnt = (cloner->cmd->write.length + MMC_BYTE_PER_BLOCK - 1)/MMC_BYTE_PER_BLOCK;
	void *addr = (void *)cloner->write_req->buf;
	u32 n;

	if (!mmc) {
		printf("no mmc device at slot %x\n", dev);
		return -ENODEV;
	}

	//debug_cond(BURNNER_DEBUG,"\nMMC write: dev # %d, block # %d, count %d ... ",
	BURNNER_PRI("MMC write: dev # %d, block # %d, count %d ... ", dev, blk, cnt);

	mmc_init(mmc);

	if (mmc_getwp(mmc) == 1) {
		printf("Error: card is write protected!\n");
		return -EPERM;
	}

	n = mmc->block_dev.block_write(dev, blk, cnt, addr);
	//debug_cond(BURNNER_DEBUG,"%d blocks write: %s\n",n, (n == cnt) ? "OK" : "ERROR");
	BURNNER_PRI("%d blocks write: %s\n",n, (n == cnt) ? "OK" : "ERROR");

	if (n != cnt)
		return -EIO;

	if (debug_args->write_back_chk) {
		mmc->block_dev.block_read(dev, blk, cnt, addr);
		debug_cond(BURNNER_DEBUG,"%d blocks read: %s\n",n, (n == cnt) ? "OK" : "ERROR");
		if (n != cnt)
			return -EIO;

		uint32_t tmp_crc = local_crc32(0xffffffff,addr,cloner->cmd->write.length);
		debug_cond(BURNNER_DEBUG,"%d blocks check: %s\n",n,(cloner->cmd->write.crc == tmp_crc) ? "OK" : "ERROR");
		if (cloner->cmd->write.crc != tmp_crc) {
			printf("src_crc32 = %08x , dst_crc32 = %08x\n",cloner->cmd->write.crc,tmp_crc);
			return -EIO;
		}
	}
	return 0;
}

int clmd_mmc_read(struct cloner *cloner, int sub_type, void *ops_data)
{
	realloc_buf(cloner, ((cloner->cmd->read.length + 0x200) & (~(0x200 - 1))));
	return mmc_read_x(sub_type, cloner->read_req->buf,
			cloner->cmd->read.partition + cloner->cmd->read.offset,
			cloner->cmd->read.length);
}

int clmd_mmc_check(struct cloner *cloner, int sub_type, void *ops_data)
{
	unsigned int buf[128];
	int ret = 0, check_buf = 0;
	ret = mmc_read_x(sub_type, buf, cloner->cmd->check.partition + cloner->cmd->check.offset, 512);
	check_buf = buf[0];
	if (!ret && check_buf == cloner->cmd->check.check)
		return 0;
	else
		return -EINVAL;
}

int cloner_mmc_init(void)
{
	struct cloner_moudle *clmd = malloc(sizeof(struct cloner_moudle));
	int ret;

	if (!clmd)
		return -ENOMEM;
	clmd->medium = MAGIC_MMC;
	clmd->ops = MMC;
	clmd->read = clmd_mmc_read;
	clmd->write = clmd_mmc_write;
	clmd->init = clmd_mmc_init;
	clmd->info = NULL;
	clmd->check = clmd_mmc_check;
	clmd->data = NULL;
	printf("cloner mmc register\n");
	return register_cloner_moudle(clmd);
}
CLONER_MOUDLE_INIT(cloner_mmc_init);
