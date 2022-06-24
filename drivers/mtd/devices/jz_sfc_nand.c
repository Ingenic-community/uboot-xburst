/*
 * jz mtd spi nand driver probe interface
 *
 *  Copyright (C) 2013 Ingenic Semiconductor Co., LTD.
 *  Author: cli <chen.li@ingenic.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later versio.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <config.h>
#include <common.h>
#include <malloc.h>
#include <errno.h>
#include <nand.h>
#include <linux/list.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <asm/io.h>
#include <asm/arch/spinand.h>
#include "jz_sfc_common.h"
#include "./nand_device/nand_common.h"


extern void active_die(struct sfc_flash *flash, uint8_t die_id);

#define W25M02GV_MID	    (0xEF)
#define W25M02GV_DID	    (0xAB)

#ifdef MTDIDS_DEFAULT
static const char *const mtdids_default = MTDIDS_DEFAULT;
#else
static const char *const mtdids_default = "nand0:nand";
#endif

static LIST_HEAD(nand_list);
static	struct sfc_flash *flash;

/*struct nand_param_from_burner nand_param_from_burner;*/
struct jz_sfcnand_burner_param jz_sfc_nand_burner_param;

static int sfcnand_block_checkbad(struct mtd_info *mtd, loff_t ofs,int getchip,int allowbbt);
static int jz_sfcnand_block_markbad(struct mtd_info *mtd, loff_t ofs);

static struct nand_ecclayout gd5f_ecc_layout_128 = {
	.oobavail = 0,
};

static int32_t jz_sfc_nand_erase_blk(struct sfc_flash *flash, uint32_t pageaddr)
{
	struct jz_sfcnand_flashinfo *nand_info = flash->flash_info;
	struct jz_sfcnand_erase *nand_erase_ops = &nand_info->ops.nand_erase_ops;
	struct sfc_transfer transfer[2];
	struct flash_operation_message op_info = {
		.flash = flash,
		.pageaddr = pageaddr,
		.columnaddr = 0,
		.buffer = NULL,
		.len = 0,
	};
	int32_t ret;

	memset(transfer, 0, sizeof(transfer));
	sfc_list_init(transfer);

	/*1. write enable */
	nand_erase_ops->write_enable(transfer, &op_info);

	/*2. block erase*/
	nand_erase_ops->block_erase(&transfer[1], &op_info);
	sfc_list_add_tail(&transfer[1], transfer);

	if(sfc_sync(flash->sfc, transfer)) {
		printf( "sfc_sync error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		return -EIO;
	}

	mdelay(nand_info->param.tBE);
	/*3. get feature*/
	ret = nand_erase_ops->get_feature(&op_info);

	if(ret)
		printf("Erase error,get state error ! %s %s %d \n",__FILE__,__func__,__LINE__);

	return ret;
}

static int jz_sfc_nand_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	//struct sfc_flash *flash = TO_SFC_FLASH(mtd);
	uint32_t addr = (uint32_t)instr->addr;
	uint32_t end;
	int32_t ret;
	struct jz_sfcnand_flashinfo *nand_info = flash->flash_info;

	if(addr % mtd->erasesize) {
		printf("ERROR:%s line %d eraseaddr no align\n", __func__,__LINE__);
		return -EINVAL;
	}
	end = addr + instr->len;
	instr->state = MTD_ERASING;
	while (addr < end) {
		if((ret = jz_sfc_nand_erase_blk(flash, addr / mtd->writesize))) {
			printf("spi nand erase error blk id  %d !\n",addr / mtd->erasesize);
			instr->state = MTD_ERASE_FAILED;
			goto erase_exit;
		}
		addr += mtd->erasesize;
	}

	if(nand_info->id_manufactory == W25M02GV_MID &&
	    nand_info->id_device == W25M02GV_DID)
		active_die(flash, 0);

	instr->state = MTD_ERASE_DONE;
erase_exit:
	//mtd_erase_callback(instr);
	return ret;
}

static int32_t jz_sfc_nand_write(struct sfc_flash *flash, const u_char *buffer, uint32_t pageaddr, uint32_t columnaddr, size_t len)
{
	struct jz_sfcnand_flashinfo *nand_info = flash->flash_info;
	struct jz_sfcnand_write *nand_write_ops = &nand_info->ops.nand_write_ops;
	struct sfc_transfer transfer[3];
	struct flash_operation_message op_info = {
		.flash = flash,
		.pageaddr = pageaddr,
		.columnaddr = columnaddr,
		.buffer = (u_char *)buffer,
		.len = len,
	};
	int32_t ret = 0;

	memset(transfer, 0, sizeof(transfer));
	sfc_list_init(transfer);

	/*1. write enable*/
	nand_write_ops->write_enable(transfer, &op_info);

	/*2. write to cache*/
	if(nand_info->param.need_quad) {
		nand_write_ops->quad_load(&transfer[1], &op_info);
	} else {
		nand_write_ops->single_load(&transfer[1], &op_info);
	}
	sfc_list_add_tail(&transfer[1], transfer);

	/*3. program exec*/
	nand_write_ops->program_exec(&transfer[2], &op_info);
	sfc_list_add_tail(&transfer[2], transfer);

	if(sfc_sync(flash->sfc, transfer)) {
		printf("sfc_sync error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		return -EIO;
	}

	udelay(nand_info->param.tPP);
	/*4. get status to be sure nand wirte completed*/
	ret = nand_write_ops->get_feature(&op_info);

	return  ret;
}

static int32_t jz_sfc_nand_read(struct sfc_flash *flash, int32_t pageaddr, int32_t columnaddr, uint8_t *buffer, size_t len)
{
	struct jz_sfcnand_flashinfo *nand_info = flash->flash_info;
	struct jz_sfcnand_read *nand_read_ops = &nand_info->ops.nand_read_ops;
	struct sfc_transfer transfer;
	struct flash_operation_message op_info = {  .flash = flash,
						    .pageaddr = pageaddr,
						    .columnaddr = columnaddr,
						    .buffer = buffer,
						    .len = len,
						};

	int32_t ret = 0;

	memset(&transfer, 0, sizeof(transfer));
	sfc_list_init(&transfer);

	/*1. pageread_to_cache*/
	nand_read_ops->pageread_to_cache(&transfer, &op_info);
	if(sfc_sync(flash->sfc, &transfer)) {
		printf("sfc_sync error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		return -EIO;
	}

	/*2. read delay	*/
	udelay(nand_info->param.tRD);

	/*3. read feature*/
	ret = nand_read_ops->get_feature(&op_info);
	if(ret == -EIO) {
		printf("sfc nand read get_feature error!\n");
		return ret;
	}

	memset(&transfer, 0, sizeof(transfer));
	sfc_list_init(&transfer);

	/*4. read data to mem*/
	if(nand_info->param.need_quad) {
		nand_read_ops->quad_read(&transfer, &op_info);
	} else {
		nand_read_ops->single_read(&transfer, &op_info);
	}
	if(sfc_sync(flash->sfc, &transfer)) {
		printf("sfc_sync error ! %s %s %d\n", __FILE__, __func__, __LINE__);
		ret = -EIO;
	}
	return ret;
}

static int jz_sfcnand_read(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, u_char *buf)
{
	uint32_t pagesize = mtd->writesize;
	uint32_t pageaddr;
	uint32_t columnaddr;
	uint32_t rlen;
	int32_t ret = 0, reterr = 0, ret_eccvalue = 0;
	struct jz_sfcnand_flashinfo *nand_info = flash->flash_info;

	*retlen = 0;
	while(len) {
		pageaddr = (uint32_t)from / pagesize;
		columnaddr = (uint32_t)from % pagesize;
		rlen = min_t(uint32_t, len, pagesize - columnaddr);
		ret = jz_sfc_nand_read(flash, pageaddr, columnaddr, buf, rlen);
		if(ret < 0) {
			printf("%s %s %d: jz_sfc_nand_read error, ret = %d, \
				pageaddr = %u, columnaddr = %u, rlen = %u\n",
				__FILE__, __func__, __LINE__,
				ret, pageaddr, columnaddr, rlen);
			reterr = ret;
			if(ret == -EIO)
				break;
		} else if (ret > 0) {
			printf("%s %s %d: jz_sfc_nand_read, ecc value = %d\n",
				__FILE__, __func__, __LINE__, ret);
			ret_eccvalue = ret;
		}

		len -= rlen;
		from += rlen;
		buf += rlen;
		*retlen += rlen;
	}
	if(nand_info->id_manufactory == W25M02GV_MID &&
	    nand_info->id_device == W25M02GV_DID)
		active_die(flash, 0);
	return reterr ? reterr : (ret_eccvalue ? ret_eccvalue : ret);
}

static int jz_sfcnand_write_oob(struct mtd_info *mtd, loff_t addr, struct mtd_oob_ops *ops)
{
	uint32_t oob_addr = (uint32_t)addr;
	int32_t ret;

	debug("write oob_addr %x, datalen %d ooboff %d, ooblen %d\n", oob_addr, ops->len, ops->ooboffs, ops->ooblen);

	if(ops->datbuf && ops->len) {
		if((ret = jz_sfc_nand_write(flash, ops->datbuf, oob_addr / mtd->writesize, 0, ops->len))) {
			printf( "spi nand write oob data area error %s %s %d \n",__FILE__,__func__,__LINE__);
			goto write_oob_exit;
		}
	}
	if(ops->oobbuf && ops->ooblen) {
		if((ret = jz_sfc_nand_write(flash, ops->oobbuf, oob_addr / mtd->writesize, mtd->writesize + ops->ooboffs, ops->ooblen))) {
			printf( "spi nand write oob oob area error %s %s %d \n",__FILE__,__func__,__LINE__);
			goto write_oob_exit;
		}
	}
	ops->retlen = ops->len;
	ops->oobretlen = ops->ooblen;

write_oob_exit:
	return ret;
}

static int sfcnand_block_isbad(struct mtd_info *mtd,loff_t ofs)
{
	int ret;
	ret = sfcnand_block_checkbad(mtd, ofs,1, 0);
	return ret;
}

static int sfcnand_block_markbad(struct mtd_info *mtd,loff_t ofs)
{
	struct nand_chip *chip = mtd->priv;
	int ret;

	ret = sfcnand_block_isbad(mtd, ofs);
	if (ret) {
		/* If it was bad already, return success and do nothing */
		if (ret > 0)
			return 0;
		return ret;
	}

	return chip->block_markbad(mtd, ofs);
}

static int jz_sfcnand_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	int ret;
	if((ret = jz_sfc_nand_erase(mtd, instr))) {
		printf("WARNING: block %d erase fail !\n",(uint32_t)instr->addr / mtd->erasesize);
		if((ret = jz_sfcnand_block_markbad(mtd, instr->addr))) {
			printf("mark bad block error, there will occur error,so exit !\n");
			return -1;
		}
	}
	instr->state = MTD_ERASE_DONE;
	return 0;
}

static int32_t jz_sfcnand_read_oob(struct mtd_info *mtd, loff_t from, struct mtd_oob_ops *ops)
{
	uint32_t addr = (uint32_t)from;
	uint32_t pageaddr = addr / mtd->writesize;
	int32_t ret = 0, ret_eccvalue = 0;

	if(ops->datbuf && ops->len) {
		ret = jz_sfc_nand_read(flash, pageaddr, 0, ops->datbuf, ops->len);
		if(ret < 0) {
			printf("spi nand read error %s %s %d ,ret = %d\n", __FILE__, __func__,  __LINE__, ret);
			if(ret == -EIO) {
				return ret;
			} else {
				ret_eccvalue = ret;
			}
		}

		ops->retlen = ops->len;
	}
	if(ops->oobbuf && ops->ooblen){
		ret = jz_sfc_nand_read(flash, pageaddr, mtd->writesize + ops->ooboffs, ops->oobbuf, ops->ooblen);
		if(ret < 0)
		            printf("%s %s %d : spi nand read oob error ,ret= %d , oob addr %x, ooboffs %d, ooblen %d\n",
					__FILE__, __func__, __LINE__, ret, addr, ops->ooboffs, ops->ooblen);

		if(ret != -EIO)
		            ops->oobretlen = ops->ooblen;
	}
	return ret ? ret : ret_eccvalue;
}

static int badblk_check(int len,unsigned char *buf)
{
	int j = 0;
	unsigned char *check_buf = buf;

	for(j = 0; j < len; j++){
	    if(check_buf[j] != 0xff){
		return 1;
	    }
	}

	return 0;
}

static int sfcnand_block_checkbad(struct mtd_info *mtd, loff_t ofs,int getchip,int allowbbt)
{
	struct nand_chip *chip = mtd->priv;
	int ret = 0;

	if (chip->bbt && (chip->options & NAND_BBT_SCANNED))
		ret = nand_isbad_bbt(mtd, ofs, allowbbt);
	else
		ret = chip->block_bad(mtd, ofs,getchip);

	return ret;
}


static int jz_sfcnand_block_bad_check(struct mtd_info *mtd, loff_t ofs,int getchip)
{
	int check_len = 1;
	unsigned char check_buf[] = {0xff, 0xff};
	struct nand_chip *chip = (struct nand_chip *)mtd->priv;
	struct mtd_oob_ops ops;

	memset(&ops, 0, sizeof(ops));
	if (chip->options & NAND_BUSWIDTH_16)
		check_len = 2;

	ops.oobbuf = check_buf;
	ops.ooblen = check_len;
	jz_sfcnand_read_oob(mtd, ofs, &ops);

	if(badblk_check(check_len, check_buf))
		return 1;
	return 0;
}

static int jz_sfcnand_write(struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, const u_char *buf)
{
	uint32_t pagesize = mtd->writesize;
	uint32_t pageaddr;
	uint32_t columnaddr;
	uint32_t wlen;
	int32_t ret;
	struct jz_sfcnand_flashinfo *nand_info = flash->flash_info;

	while(len) {
		pageaddr = (uint32_t)to / pagesize;
		columnaddr = (uint32_t)to % pagesize;
		wlen = min_t(uint32_t, pagesize - columnaddr, len);

		if((ret = jz_sfc_nand_write(flash, buf, pageaddr, columnaddr, wlen))) {
			printf("%s %s %d : spi nand write fail, ret = %d, \
				pageaddr = %u, columnaddr = %u, wlen = %u\n",
				__FILE__, __func__, __LINE__, ret,
				pageaddr, columnaddr, wlen);
			break;
		}
		*retlen += wlen;
		len -= wlen;
		to += wlen;
		buf += wlen;
	}
	if(nand_info->id_manufactory == W25M02GV_MID &&
	    nand_info->id_device == W25M02GV_DID)
		active_die(flash, 0);

	return ret;
}

static void mtd_sfcnand_init(struct mtd_info *mtd)
{
	mtd->_erase = jz_sfcnand_erase;
	mtd->_point = NULL;
	mtd->_unpoint = NULL;
	mtd->_read = jz_sfcnand_read;
	mtd->_write = jz_sfcnand_write;
	mtd->_read_oob = jz_sfcnand_read_oob;
	mtd->_write_oob = jz_sfcnand_write_oob;
	mtd->_lock = NULL;
	mtd->_unlock = NULL;
	mtd->_block_isbad = sfcnand_block_isbad;
	mtd->_block_markbad = sfcnand_block_markbad;
}

static int jz_sfcnand_block_markbad(struct mtd_info *mtd, loff_t ofs)
{
	struct nand_chip *chip = mtd->priv;
	uint8_t buf[2] = { 0, 0 };
	int res, ret = 0, i = 0;
	int write_oob = !(chip->bbt_options & NAND_BBT_NO_OOB_BBM);

	/* Write bad block marker to OOB */
	if (write_oob) {
		struct mtd_oob_ops ops;
		loff_t wr_ofs = ofs;

		ops.datbuf = NULL;
		ops.oobbuf = buf;
		ops.ooboffs = chip->badblockpos;
		if (chip->options & NAND_BUSWIDTH_16) {
			ops.ooboffs &= ~0x01;
			ops.len = ops.ooblen = 2;
		} else {
			ops.len = ops.ooblen = 1;
		}
		ops.mode = MTD_OPS_PLACE_OOB;

		/* Write to first/last page(s) if necessary */
		if (chip->bbt_options & NAND_BBT_SCANLASTPAGE)
			wr_ofs += mtd->erasesize - mtd->writesize;
		do {
			res = jz_sfcnand_write_oob(mtd, wr_ofs, &ops);
			if (!ret)
				ret = res;

			i++;
			wr_ofs += mtd->writesize;
		} while ((chip->bbt_options & NAND_BBT_SCAN2NDPAGE) && i < 2);
	}

	/* Update flash-based bad block table */
	if (chip->bbt_options & NAND_BBT_USE_FLASH) {
		res = nand_update_bbt(mtd, ofs);
	}

	return ret;

}

static void get_partition_from_spinand(struct sfc_flash *flash)
{
	size_t retlen;
	jz_sfcnand_read(flash->mtd, CONFIG_SPIFLASH_PART_OFFSET, sizeof(struct jz_sfcnand_burner_param) - 4, &retlen, (u_char *)&jz_sfc_nand_burner_param);
	jz_sfc_nand_burner_param.partition = malloc(sizeof(struct jz_sfcnand_partition) * jz_sfc_nand_burner_param.partition_num);
	jz_sfcnand_read(flash->mtd, CONFIG_SPIFLASH_PART_OFFSET + sizeof(struct jz_sfcnand_burner_param) - 4,
			sizeof(struct jz_sfcnand_partition) * jz_sfc_nand_burner_param.partition_num, &retlen,
			(u_char *)jz_sfc_nand_burner_param.partition);
#ifdef DEBUG
	{
		int32_t i;
	for(i = 0; i < jz_sfc_nand_burner_param.partition_num; i++) {
		printf("name = %s\n", jz_sfc_nand_burner_param.partition[i].name);
		printf("size = %x\n", jz_sfc_nand_burner_param.partition[i].size);
		printf("offset = %x\n", jz_sfc_nand_burner_param.partition[i].offset);
		printf("mask_flags = %x\n", jz_sfc_nand_burner_param.partition[i].mask_flags);
	}
	}
#endif
}

static int32_t sfc_nand_reset(void)
{
	struct sfc_transfer transfer;
	memset(&transfer, 0, sizeof(transfer));

	sfc_list_init(&transfer);
	transfer.sfc_mode = TM_STD_SPI;
	transfer.cmd_info.cmd = 0xff;

	transfer.direction = GLB_TRAN_DIR_WRITE;

	transfer.ops_mode = CPU_OPS;

	if(sfc_sync(flash->sfc, &transfer)) {
		printf("%s %s %d: sfc sync failed!\n", __FILE__, __func__, __LINE__);
		return -EIO;
	}
	return 0;
}

static int32_t jz_sfcnand_fill_ops(struct sfc_flash *flash, struct jz_sfcnand_ops *slave_ops) {

	struct jz_sfcnand_flashinfo *nand_info = flash->flash_info;
	/*master ops:sfcnand driver ops*/
	struct jz_sfcnand_ops *master_ops = &nand_info->ops;
	struct jz_sfcnand_read *master_read_ops = &master_ops->nand_read_ops;
	struct jz_sfcnand_write *master_write_ops = &master_ops->nand_write_ops;
	struct jz_sfcnand_erase *master_erase_ops = &master_ops->nand_erase_ops;

	*master_ops = *slave_ops;

	/*read ops*/
	master_read_ops->pageread_to_cache ? : (master_read_ops->pageread_to_cache = nand_pageread_to_cache);

	if(!master_read_ops->get_feature) {
		printf("ERROR: nand device must have get_read_feature function,id_manufactory= %02x, id_device=%02x\n", nand_info->id_manufactory, nand_info->id_device);
		return -EIO;
	}

	master_read_ops->single_read ? : (master_read_ops->single_read = nand_single_read);

	master_read_ops->quad_read ? : (master_read_ops->quad_read = nand_quad_read);

	/*write ops*/
	master_write_ops->write_enable ? : (master_write_ops->write_enable = nand_write_enable);

	master_write_ops->single_load ? : (master_write_ops->single_load = nand_single_load);

	master_write_ops->quad_load ? : (master_write_ops->quad_load = nand_quad_load);

	master_write_ops->program_exec ? : (master_write_ops->program_exec = nand_program_exec);

	master_write_ops->get_feature ? : (master_write_ops->get_feature = nand_get_program_feature);

	/*erase ops*/
	master_erase_ops->write_enable ? : (master_erase_ops->write_enable = nand_write_enable);

	master_erase_ops->block_erase ? : (master_erase_ops->block_erase = nand_block_erase);

	master_erase_ops->get_feature ? : (master_erase_ops->get_feature = nand_get_erase_feature);

	return 0;
}

static int32_t jz_sfc_nand_try_id(struct sfc_flash *flash, struct jz_sfcnand_flashinfo *nand_info)
{
	struct jz_sfcnand_device *nand_device;
	struct sfc_transfer transfer;
	uint8_t addr_len[2] = {0, 1};
	uint8_t id_buf[2] = {0};
	uint8_t i = 0;

	for(i = 0; i < sizeof(addr_len); i++) {

		memset(id_buf, 0, 2);
		memset(&transfer, 0, sizeof(transfer));
		sfc_list_init(&transfer);
		transfer.sfc_mode = TM_STD_SPI;
		transfer.cmd_info.cmd = SPINAND_CMD_RDID;

		transfer.addr = 0;
		transfer.addr_len = addr_len[i];

		transfer.cmd_info.dataen = ENABLE;
		transfer.data = id_buf;
		transfer.len = sizeof(id_buf);
		transfer.direction = GLB_TRAN_DIR_READ;
		transfer.data_dummy_bits = 0;

		transfer.ops_mode = CPU_OPS;

		if(sfc_sync(flash->sfc, &transfer)){
			printf("sfc_sync error ! %s %s %d\n",__FILE__,__func__,__LINE__);
			return -EIO;
		}

		printf("id_manufactory = %x, id_device %x\n", id_buf[0], id_buf[1]);
		list_for_each_entry(nand_device, &nand_list, list) {
			if(nand_device->id_manufactory == id_buf[0]) {
				nand_info->id_manufactory = id_buf[0];
				nand_info->id_device = id_buf[1];
				break;
			}
		}
		if(nand_info->id_manufactory && nand_info->id_device) {
			break;
		}
		udelay(500);
	}

	if(!nand_info->id_manufactory && !nand_info->id_device) {
		printf("ERROR!: don`t support this nand manufactory, please add nand driver, id_manufactory %x id_device %x\n", id_buf[0], id_buf[1]);
		return -ENODEV;
	} else {
		struct device_id_struct *device_id = nand_device->id_device_list;
		int32_t id_count = nand_device->id_device_count;
		while(id_count--) {
			if(device_id->id_device == nand_info->id_device) {
				/*notice :base_param and partition param should read from nand*/
				nand_info->param = *device_id->param;
				break;
			}
			device_id++;
		}
		if(id_count < 0) {
			printf("ERROR: do support this device, id_manufactory = 0x%02x, id_device = 0x%02x\n", nand_info->id_manufactory, nand_info->id_device);
			return -ENODEV;
		}
	}

	printf("Found nand: id_manufactory: 0x%02x id_device: 0x%02x\n", nand_info->id_manufactory, nand_info->id_device);

	return jz_sfcnand_fill_ops(flash, &nand_device->ops);
}

int jz_sfcnand_register(struct jz_sfcnand_device *flash) {

	if (!flash)
		return -EINVAL;
	list_add_tail(&flash->list, &nand_list);
	return 0;
}

/******************************************************************/
static int32_t sfc_nand_get_feature(struct sfc_flash *flash, uint8_t addr, uint8_t *val)
{
	struct sfc_transfer transfer;

	memset(&transfer, 0, sizeof(transfer));
	sfc_list_init(&transfer);

	transfer.sfc_mode = TM_STD_SPI;
	transfer.cmd_info.cmd = 0x0f;

	transfer.addr_len = 1;
	transfer.addr = addr;

	transfer.cmd_info.dataen = ENABLE;
	transfer.len = 1;
	transfer.data = val;
	transfer.direction = GLB_TRAN_DIR_READ;

	transfer.ops_mode = CPU_OPS;

	if(sfc_sync(flash->sfc, &transfer)) {
	        printf("sfc_sync error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		return -EIO;
	}
	return 0;
}

static int32_t sfc_nand_set_feature(struct sfc_flash *flash, uint8_t addr, uint8_t val)
{
	struct sfc_transfer transfer;

	memset(&transfer, 0, sizeof(transfer));
	sfc_list_init(&transfer);

	transfer.sfc_mode = TM_STD_SPI;
	transfer.cmd_info.cmd = 0x1f;

	transfer.addr_len = 1;
	transfer.addr = addr;

	transfer.cmd_info.dataen = ENABLE;
	transfer.len = 1;
	transfer.data = &val;
	transfer.direction = GLB_TRAN_DIR_WRITE;

	transfer.ops_mode = CPU_OPS;

	if(sfc_sync(flash->sfc, &transfer)) {
		printf("sfc_sync error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		return -EIO;
	}
	return 0;
}

static int32_t sfc_nand_clear_write_protect(struct sfc_flash *flash)
{
	return sfc_nand_set_feature(flash, 0xa0, 0);
}

static int32_t sfc_nand_enable_ecc(struct sfc_flash *flash)
{
	int32_t ret = 0;
	uint8_t val = 0;
	if((ret = sfc_nand_get_feature(flash, 0xb0, &val))) {
		printf(" %s %s %d: sfc_nand_get_feature failed, ret = %d\n",
			__FILE__, __func__, __LINE__, ret);
		return ret;
	}

	val |= (1 << 4) | (1 << 3) | 0x1;

	if((ret = sfc_nand_set_feature(flash, 0xb0, val))) {
		printf(" %s %s %d: sfc_nand_set_feature failed, ret = %d\n",
			__FILE__, __func__, __LINE__, ret);
	}
	return ret;
}

static int32_t sfc_nand_special_init(struct sfc_flash *flash)
{
	int32_t ret = 0;
	ret = sfc_nand_clear_write_protect(flash);
	if(ret) {
		printf("sfc_nand_clear_write_protect failed!, ret= %d\n",ret);
		return ret;
	}
	ret = sfc_nand_enable_ecc(flash);
	if(ret) {
		printf("sfc_nand_enable_ecc failed!, ret= %d\n", ret);
	}
	return ret;
}

static inline int32_t spinand_moudle_init(void)
{
	spinand_regcall_t *call = ll_entry_start(spinand_regcall_t, flash);
	int ret = 0 , i, count;

	for (i = 0, count = ll_entry_count(spinand_regcall_t, flash);
		i < count;
		call++, i++) {
	    ret = (*call)();
	    if (ret) {
		printf("jz spi nand ops init func error\n");
		break;
	    }
	}
	return ret;
}

int32_t jz_sfc_nand_init(uint32_t sfc_quad_mode, struct jz_sfcnand_burner_param *param)
{
	struct nand_chip *chip;
	struct mtd_info *mtd;
	struct jz_sfcnand_flashinfo *flash_info;
	int32_t ret = 0;

	if(!flash) {
		flash = malloc(sizeof(struct sfc_flash));
		if (!flash) {
			printf("ERROR: %s %d alloc() error !\n",__func__,__LINE__);
			return -1;
		}
		memset(flash, 0, sizeof(struct sfc_flash));
		flash->sfc = sfc_res_init(CONFIG_SFC_NAND_RATE);
	}
	mtd = &nand_info[0];
	flash_info = calloc(sizeof(struct jz_sfcnand_flashinfo), sizeof(uint8_t));
	if(!flash_info) {
		printf("ERROR: %s %d alloc() error !\n",__func__,__LINE__);
		return -ENOMEM;
	}
	flash->flash_info = flash_info;
	flash->mtd = mtd;

	if (spinand_moudle_init()) {
		ret = -EINVAL;
		goto failed;
	}

#define THOLD   5
#define TSETUP  5
#define TSHSL_R     20
#define TSHSL_W     50

	set_flash_timing(flash->sfc, THOLD, TSETUP, TSHSL_R, TSHSL_W);

	if((ret = sfc_nand_reset())) {
		printf("ERR : sfc reset error!\n");
		goto failed;
	}

	if((ret = jz_sfc_nand_try_id(flash, flash_info))) {
		printf("ERR: sfc try id error!\n");
		goto failed;
	}

	set_flash_timing(flash->sfc, flash_info->param.tHOLD, flash_info->param.tSETUP, flash_info->param.tSHSL_R, flash_info->param.tSHSL_W);

	if((ret = sfc_nand_special_init(flash))) {
		printf("ERR :sfcnand special init failed!\n");
		goto failed;
	}
#if defined(CONFIG_JZ_SPINAND_SN) && defined(CONFIG_JZ_SPINAND_MAC)
	mtd->size = flash_info->param.flashsize - CONFIG_SN_SIZE - CONFIG_MAC_SIZE;
#else
	mtd->size = flash_info->param.flashsize;
#endif

#ifdef CONFIG_BURNER
	/* for burner get pt indext */
	flash_info->partition.num_partition = param->partition_num;
	flash_info->partition.partition = &param->partition;
	{
		int part = 0;
		for (part = 0; part < flash_info->partition.num_partition; part++)
			if (flash_info->partition.partition[part].size == 0UL ||
					flash_info->partition.partition[part].size == -1UL)
				break;
		if (part != flash_info->partition.num_partition) {
			flash_info->partition.partition[part].size = mtd->size -
				flash_info->partition.partition[part].offset;
			printf("flash size 0x%x, part size 0x%x\n", flash_info->param.flashsize, flash_info->partition.partition[part].size);
		}
	}
#else
	mtd->writesize = flash_info->param.pagesize;
	get_partition_from_spinand(flash);
	flash_info->partition.num_partition =jz_sfc_nand_burner_param.partition_num;
	flash_info->partition.partition = jz_sfc_nand_burner_param.partition;
#endif

	chip = calloc(1, sizeof(struct nand_chip));
	if (!chip) {
		ret = -ENOMEM;
		goto failed;
	}
	memset(chip,0,sizeof(struct nand_chip));

	mtd->flags |= MTD_CAP_NANDFLASH;
	mtd->erasesize = flash_info->param.blocksize;
	mtd->writesize = flash_info->param.pagesize;
	mtd->oobsize = flash_info->param.oobsize;

	chip->select_chip = NULL;
	chip->badblockbits = 8;
	chip->scan_bbt = nand_default_bbt;
	chip->block_bad = jz_sfcnand_block_bad_check;
	chip->block_markbad = jz_sfcnand_block_markbad;
	chip->ecc.layout= &gd5f_ecc_layout_128; // for erase ops
	chip->bbt_erase_shift = chip->phys_erase_shift = ffs(mtd->erasesize) - 1;
	if (!(chip->options & NAND_OWN_BUFFERS))
		chip->buffers = memalign(ARCH_DMA_MINALIGN,sizeof(*chip->buffers));

	/* Set the bad block position */
	if (mtd->writesize > 512 || (chip->options & NAND_BUSWIDTH_16))
		chip->badblockpos = NAND_LARGE_BADBLOCK_POS;
	else
		chip->badblockpos = NAND_SMALL_BADBLOCK_POS;

	mtd->priv = chip;

	mtd_sfcnand_init(mtd);
	nand_register(0);
	return ret;

failed:
	free(flash_info);
	free(flash);
	return ret;
}

static void mtd_sfcnand_partition_analysis(uint32_t blk_sz, uint8_t partcount, struct jz_sfcnand_partition *jz_mtd_spinand_partition)
{
	char mtdparts_env[X_ENV_LENGTH];
	char command[X_COMMAND_LENGTH];
	uint8_t part = 0;

	memset(mtdparts_env, 0, X_ENV_LENGTH);
	memset(command, 0, X_COMMAND_LENGTH);

	/*MTD part*/
	sprintf(mtdparts_env, "mtdparts=nand:");
	for (part = 0; part < partcount; part++) {
		if (jz_mtd_spinand_partition[part].size == -1UL || jz_mtd_spinand_partition[part].size == 0UL) {
			sprintf(mtdparts_env,"%s-(%s)", mtdparts_env,
					jz_mtd_spinand_partition[part].name);
			break;
		} else if (jz_mtd_spinand_partition[part].size != 0) {
			if(jz_mtd_spinand_partition[part].size % blk_sz != 0)
				printf("ERROR:the partition [%s] don't algin as block size [0x%08x] ,it will be error !\n",jz_mtd_spinand_partition[part].name, blk_sz);

			sprintf(mtdparts_env, "%s%dK@%d(%s),", mtdparts_env,
					jz_mtd_spinand_partition[part].size / 0x400,
					jz_mtd_spinand_partition[part].offset,
					jz_mtd_spinand_partition[part].name);
		} else {
			break;
		}
	}
	debug("env:mtdparts=%s\n", mtdparts_env);
	setenv("mtdids", mtdids_default);
	setenv("mtdparts", mtdparts_env);
	setenv("partition", NULL);
}


struct jz_sfcnand_partition *get_partion_index(u32 startaddr,u32 length,int *pt_index)
{
	struct jz_sfcnand_flashinfo *nand_info = flash->flash_info;
	int i;
	int ptcount = nand_info->partition.num_partition;
	struct jz_sfcnand_partition *jz_mtd_spinand_partition=nand_info->partition.partition;
	for(i = 0; i < ptcount; i++){
		if(startaddr >= jz_mtd_spinand_partition[i].offset && (startaddr + length) <= (jz_mtd_spinand_partition[i].offset + jz_mtd_spinand_partition[i].size)){
			*pt_index = i;
			break;
		}
	}
	if(i >= ptcount){
		printf("startaddr 0x%x can't find the pt_index or you partition size 0x%x is not align with 128K\n",startaddr, length);
		*pt_index = -1;
		return NULL;
	}
	return &jz_mtd_spinand_partition[i];
}

int32_t mtd_sfcnand_probe_burner(uint32_t *erase_mode, uint32_t sfc_quad_mode, struct jz_sfcnand_burner_param *param)
{
	struct mtd_info *mtd = &nand_info[0];
	struct nand_chip *chip;
	int32_t ret;

	if(jz_sfc_nand_init(sfc_quad_mode, param)) {
		printf("ERR: jz_sfc_nand_init error!\n");
		return -EIO;
	}
	chip = mtd->priv;
	chip->scan_bbt(mtd);
	chip->options |= NAND_BBT_SCANNED;
	/*0: none 1, force-erase, force erase contain creat bbt*/
	if (*erase_mode == 1)
		if((ret = run_command("nand erase.chip -y", 0)))
			return ret;

	if(chip->bbt)
		free(chip->bbt);
	chip->scan_bbt(mtd);
	chip->options |= NAND_BBT_SCANNED;
	mtd_sfcnand_partition_analysis(mtd->erasesize, param->partition_num,
			(void *)&param->partition/*This is a structure rather than a pointer in burner*/);
	return 0;
}
