#include <mmc.h>

#if defined(CONFIG_JZ_MMC) || defined(CONFIG_JZ_SDHCI)

#define MMC_BYTE_PER_BLOCK 512


static int mmc_erase()
{
	int curr_device = 0;
	struct mmc *mmc = find_mmc_device(0);
	uint32_t blk, blk_end, blk_cnt;
	uint32_t erase_cnt = 0;
	int timeout = 30000;
	int i;
	int ret;

	if (!mmc) {
		printf("no mmc device at slot %x\n", curr_device);
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
				curr_device, blk, blk_cnt);

		ret = mmc->block_dev.block_erase(curr_device, blk, blk_cnt);
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
	erase_cnt = (mmc_args->mmc_erase_range_count >MMC_ERASE_CNT_MAX) ?
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
				curr_device, blk, blk_cnt);

		ret = mmc->block_dev.block_erase(curr_device, blk, blk_cnt);
		if (!ret) {
			printf("Error: mmc erase error\n");
			return -EIO;
		}

		BURNNER_PRI("mmc part erase, part %d ok\n", i);

	}
	BURNNER_PRI("mmc erase ok\n");
	return 0;
}

int mmc_program(struct cloner *cloner, int mmc_index)
{
#define MMC_BYTE_PER_BLOCK 512
	int curr_device = mmc_index;
	struct mmc *mmc = find_mmc_device(mmc_index);
	u32 blk = (cloner->cmd->write.partition + cloner->cmd->write.offset)/MMC_BYTE_PER_BLOCK;
	u32 cnt = (cloner->cmd->write.length + MMC_BYTE_PER_BLOCK - 1)/MMC_BYTE_PER_BLOCK;
	void *addr = (void *)cloner->write_req->buf;
	u32 n;

	if (!mmc) {
		printf("no mmc device at slot %x\n", curr_device);
		return -ENODEV;
	}

	//debug_cond(BURNNER_DEBUG,"\nMMC write: dev # %d, block # %d, count %d ... ",
	BURNNER_PRI("MMC write: dev # %d, block # %d, count %d ... ",
			curr_device, blk, cnt);

	mmc_init(mmc);

	if (mmc_getwp(mmc) == 1) {
		printf("Error: card is write protected!\n");
		return -EPERM;
	}

	n = mmc->block_dev.block_write(curr_device, blk, cnt, addr);
	//debug_cond(BURNNER_DEBUG,"%d blocks write: %s\n",n, (n == cnt) ? "OK" : "ERROR");
	BURNNER_PRI("%d blocks write: %s\n",n, (n == cnt) ? "OK" : "ERROR");

	if (n != cnt)
		return -EIO;

	if (debug_args->write_back_chk) {
		mmc->block_dev.block_read(curr_device, blk, cnt, addr);
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

int mmc_read_x(int dev, void *buf, loff_t offset ,size_t length)
{
	char command[128];
	loff_t blk = offset/MMC_BYTE_PER_BLOCK;
	size_t cnt = (length + MMC_BYTE_PER_BLOCK -1)/MMC_BYTE_PER_BLOCK;
	int ret = 0;
	memset(command, 0, 128);
	sprintf(command, "mmc dev %d", dev);
	printf("%s\n",command);
	ret = run_command(command, 0);
	if (ret)
		return ret;
	memset(command, 0, 128);
	sprintf(command, "mmc read %p 0x%llx 0x%lx", buf, blk, cnt);
	printf("%s\n", command);
	ret = run_command(command, 0);
	return ret;
}
#endif
