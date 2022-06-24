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


#include <common.h>
#include <malloc.h>
#include <errno.h>
#include <nand.h>
#include <spi.h>
#include <spi_flash.h>
#include <linux/list.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <asm/arch/spi.h>
#include <ingenic_nand_mgr/nand_param.h>
#include "../spi/spi_flash_internal.h"
#include "../../spi/jz_spi.h"
#include "jz_spinand.h"
#include <cloner/cloner.h>
#define IDCODE_CONT_LEN 0
#define IDCODE_PART_LEN 5

#ifdef MTDIDS_DEFAULT
static const char *const mtdids_default = MTDIDS_DEFAULT;
#else
static const char *const mtdids_default = "nand0:nand";
#endif
#define SIZE_UBOOT  0x100000    /* 1M */
#define SIZE_KERNEL 0x800000    /* 8M */
#define SIZE_ROOTFS (0x100000 * 40)        /* -1: all of left */

extern unsigned int sfc_rate ;
static struct jz_spinand_partition jz_mtd_spinand_partition[] = {
        {
                .name =     "uboot",
                .offset =   0,
                .size =     SIZE_UBOOT,
                .manager_mode = MTD_MODE,
        },
        {
                .name =     "kernel",
                .offset =   SIZE_UBOOT,
                .size =     SIZE_KERNEL,
                .manager_mode = MTD_MODE,
        },
        {
                .name   =       "rootfs",
                .offset =   SIZE_UBOOT + SIZE_KERNEL,
                .size   =   SIZE_ROOTFS,
                .manager_mode = UBI_MANAGER,
                //.manager_mode = MTD_MODE
        },
};

static unsigned short column_cmdaddr_bits=24;/* read from cache ,the bits of cmd + addr */


struct nand_param_from_burner nand_param_from_burner;


/* wait time before read status (us) for spi nand */
//static int t_reset = 500;
static int t_read  = 120;
static int t_write = 700;
static int t_erase = 5000;

extern void spi_init(void);
extern void jz_cs_reversal(void );

static struct nand_ecclayout gd5f_ecc_layout_128 = {
	.oobavail = 0,
};

int spi_nand_erase(struct mtd_info *mtd,int addr)
{
	unsigned char cmd[COMMAND_MAX_LENGTH];
	volatile unsigned char read_buf;
	int erase_cmd;
	int page = addr / mtd->writesize;
	int block_size = mtd->erasesize;
	int timeout = 2000;

	switch(block_size){
		case 4 * 1024:
			erase_cmd = CMD_ERASE_4K;
			break;
		case 32 * 1024:
			erase_cmd = CMD_ERASE_32K;
			break;
		case 64 * 1024:
			erase_cmd = CMD_ERASE_64K;
			break;
		case 128 * 1024:
			erase_cmd = CMD_ERASE_128K;
			break;
		default:
			printf("WARNING: don't support the erase size !\n");
			break;
	}

	jz_cs_reversal();
	cmd[0] = CMD_WREN;
	spi_send_cmd(cmd, 1);

	jz_cs_reversal();
	cmd[0] = erase_cmd;
	cmd[1] = (page >> 16) & 0xff;
	cmd[2] = (page >> 8) & 0xff;
	cmd[3] = page & 0xff;
	spi_send_cmd(cmd, 4);
	udelay(t_erase);

	do{
		jz_cs_reversal();
		cmd[0] = CMD_GET_FEATURE;
		cmd[1] = FEATURE_ADDR;
		spi_send_cmd(cmd, 2);
		spi_recv_cmd(&read_buf, 1);
		timeout--;
	}while((read_buf & SPINAND_IS_BUSY) && (timeout > 0));

	if(read_buf & E_FAIL)
		return -1;

	return 0;
}
static int spi_nand_write_page(u_char *buffer,int page,int column,size_t wlen)
{
	size_t ops_len;
	unsigned char cmd[COMMAND_MAX_LENGTH];
	volatile unsigned char state;
	int timeout = 2000;

	jz_cs_reversal();
	cmd[0] = CMD_PRO_LOAD;
	cmd[1] = (column >> 8) & 0xf;
	cmd[2] = column & 0xff;
	spi_send_cmd(cmd, 3);

	ops_len = wlen;
	while(ops_len) {
		if(ops_len > FIFI_THRESHOLD) {
			spi_send_cmd(buffer, FIFI_THRESHOLD);
			buffer += FIFI_THRESHOLD;
			ops_len -= FIFI_THRESHOLD;
		} else {
			spi_send_cmd(buffer, ops_len);
			buffer += ops_len;
			ops_len = 0;
		}
	}
	jz_cs_reversal();
	cmd[0] = CMD_WREN;
	spi_send_cmd(cmd, 1);

	jz_cs_reversal();
	cmd[0] = CMD_PE;
	cmd[1] = (page >> 16) & 0xff;
	cmd[2] = (page >> 8) & 0xff;
	cmd[3] = page & 0xff;
	spi_send_cmd(cmd, 4);
	udelay(t_write);

	do{
		jz_cs_reversal();
		cmd[0] = CMD_GET_FEATURE;
		cmd[1] = FEATURE_ADDR;
		spi_send_cmd(cmd, 2);

		spi_recv_cmd(&state, 1);
		timeout--;
	}while((state & SPINAND_IS_BUSY) && (timeout > 0));

	if(state & P_FAIL){
		printf("WARNING: write fail !\n");
		return -EIO;
	}
	return wlen;
}
static int spi_nand_write(struct mtd_info *mtd,loff_t addr,int column,size_t len,u_char *buf)
{
	int page_size = mtd->writesize;
	int page;
	int wlen,i,write_num;
	size_t page_overlength;
	u_char *buffer = buf;
	page = (size_t)addr / page_size;
	size_t ops_addr;
	size_t ops_len;
	int ret;


	if(column){
		ops_addr = (unsigned int)addr;
		ops_len = len;

		if(len <= (page_size - column))
			page_overlength = len;
		else
			page_overlength = page_size - column;/*random read but len over a page */

		while(ops_addr < addr + len){
			page = ops_addr / page_size;

			if(page_overlength){
				ret = spi_nand_write_page(buffer,page,column,page_overlength);
				if(ret < 0)
					return ret;

				ops_len -= page_overlength;
				buffer += page_overlength;
				ops_addr += page_overlength;
				page_overlength = 0;
			}else{
				column = 0;
				if(ops_len >= page_size)
					wlen = page_size;
				else
					wlen = ops_len;

				ret = spi_nand_write_page(buffer,page,column,wlen);
				if(ret < 0)
					return ret;
			}
		}
	}else{

		write_num = (len + page_size - 1) / page_size;
		page = addr / page_size;

		for(i = 0; i < write_num; i++){
			if(len >= page_size)
				wlen = page_size;
			else
				wlen = len;

			ret = spi_nand_write_page(buffer,page,column,wlen);
			if(ret < 0)
				return ret;

			buffer += wlen;
			len -= wlen;
			page++;
		}
	}
	return 0;
}

static int spi_nand_read_page(u_char *buffer,int page,int column,size_t rlen)
{
	unsigned char cmd[COMMAND_MAX_LENGTH];
	volatile unsigned char read_buf;
	int timeout = 2000;

	jz_cs_reversal();
	cmd[0] = CMD_PARD;
	cmd[1] = (page >> 16) & 0xff;
	cmd[2] = (page >> 8) & 0xff;
	cmd[3] = page & 0xff;
	spi_send_cmd(cmd, 4);
	udelay(t_read);

	do{
		jz_cs_reversal();
		cmd[0] = CMD_GET_FEATURE;
		cmd[1] = FEATURE_ADDR;
		spi_send_cmd(cmd, 2);

		spi_recv_cmd(&read_buf, 1);
		timeout--;
	}while((read_buf & SPINAND_IS_BUSY) && (timeout > 0));

	if((read_buf & 0x30) == 0x20) {
		printf("%s %d read error pageid = %d!!!\n",__func__,__LINE__,page);
		return -EIO;
	}
	jz_cs_reversal();
	switch(column_cmdaddr_bits){
		case 24:
			cmd[0] = CMD_R_CACHE;
			cmd[1] = (column >> 8) & 0xff;
			cmd[2] = column & 0xff;
			cmd[3] = 0x0;

			spi_send_cmd(cmd, 4);
			break;
		case 32:
			cmd[0] = CMD_FR_CACHE;/* CMD_R_CACHE read odd addr may be error */
			cmd[1] = 0x0;
			cmd[2] = (column >> 8) & 0xff;
			cmd[3] = column & 0xff;
			cmd[4] = 0;/*dummy*/

			spi_send_cmd(cmd, 5);
			break;
		default:
			printk("can't support the column addr format !!!\n");
			break;
	}

	spi_recv_cmd(buffer, rlen);
	return rlen;
}

static int spi_nand_read(struct mtd_info *mtd,loff_t addr,int column,size_t len,u_char *buf)
{
	int page_size = mtd->writesize;
	int page = addr / page_size;
	int i,read_num,rlen;
	u_char *buffer = buf;
	size_t page_overlength;
	size_t ops_addr;
	size_t ops_len;
	int ret;

	if(column){
		ops_addr = (unsigned int)addr;
		ops_len = len;

		if(len <= (page_size - column))
			page_overlength = len;
		else
			page_overlength = page_size - column;/*random read but len over a page */
		while(ops_addr < addr + len){
			page = ops_addr / page_size;
			if(page_overlength){
				ret = spi_nand_read_page(buffer,page,column,page_overlength);
				if(ret < 0)
					return ret;
				ops_len -= page_overlength;
				buffer += page_overlength;
				ops_addr += page_overlength;
				page_overlength = 0;
			}else{
				column = 0;
				if(ops_len >= page_size)
					rlen = page_size;
				else
					rlen = ops_len;

				ret = spi_nand_read_page(buffer,page,column,rlen);
				if(ret < 0)
					return ret;

				buffer += rlen;
				ops_len -= rlen;
				ops_addr += rlen;
			}
		}
	}else{
		read_num = (len + page_size - 1) / page_size;
		page = addr / page_size;
		for(i = 0; i < read_num; i++){
			if(len >= page_size)
				rlen = page_size;
			else
				rlen = len;

			ret = spi_nand_read_page(buffer,page,column,rlen);
			if(ret < 0)
				return ret;

			buffer += rlen;
			len -= rlen;
			page++;
		}
	}
	return 0;
}
static int spinand_write_oob(struct mtd_info *mtd,loff_t addr,struct mtd_oob_ops *ops)
{
	unsigned char cmd[COMMAND_MAX_LENGTH];
	int page = addr / mtd->writesize;
	int ret,state;
	int column = mtd->writesize;
	int ops_len = ops->ooblen;
	int timeout = 2000;
	u_char *buffer = ops->oobbuf;

	jz_cs_reversal();
	cmd[0] = CMD_PARD;
	cmd[1] = (page >> 16) & 0xff;
	cmd[2] = (page >> 8) & 0xff;
	cmd[3] = page & 0xff;
	spi_send_cmd(cmd, 4);
	udelay(t_read);

	jz_cs_reversal();
	cmd[0] = CMD_PRO_RDM;
	cmd[1] = (column >> 8) & 0xff;
	cmd[2] = column & 0xff;
	spi_send_cmd(cmd, 3);

	while(ops_len) {
		if(ops_len > FIFI_THRESHOLD) {
			spi_send_cmd(buffer, FIFI_THRESHOLD);
			buffer += FIFI_THRESHOLD;
			ops_len -= FIFI_THRESHOLD;
		} else {
			spi_send_cmd(buffer, ops_len);
			buffer += ops_len;
			ops_len = 0;
		}
	}
	jz_cs_reversal();
	cmd[0] = CMD_WREN;
	spi_send_cmd(cmd, 1);

	jz_cs_reversal();
	cmd[0] = CMD_PE;
	cmd[1] = (page >> 16) & 0xff;
	cmd[2] = (page >> 8) & 0xff;
	cmd[3] = page & 0xff;
	spi_send_cmd(cmd, 4);
	udelay(t_write);

	do{
		jz_cs_reversal();
		cmd[0] = CMD_GET_FEATURE;
		cmd[1] = FEATURE_ADDR;
		spi_send_cmd(cmd, 2);

		spi_recv_cmd(&state, 1);
		timeout--;
	}while((state & SPINAND_IS_BUSY) && (timeout > 0));

	if(state & P_FAIL){
		printf("WARNING: write fail !\n");
		return -EIO;
	}
	return 0;
}
static int spinand_block_isbad(struct mtd_info *mtd,loff_t ofs) ;
static int spinand_block_markbad(struct mtd_info *mtd,loff_t ofs)
{
	struct nand_chip *chip = mtd->priv;
# if 0
	int ret;

	ret = spinand_block_isbad(mtd, ofs);
	if (ret) {
		/* If it was bad already, return success and do nothing */
		if (ret > 0)
			return 0;
		return ret;
	}
#endif
	return chip->block_markbad(mtd, ofs);
}
static int spinand_read_oob(struct mtd_info *mtd,loff_t addr,struct mtd_oob_ops *ops);
int spinand_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	int block_size = mtd->erasesize;
	int block,addr,ret;
	addr = instr->addr;

	ret = spi_nand_erase(mtd,addr);
	if(ret){
		printf("WARNING: block %d erase fail !\n",addr / mtd->erasesize);

		ret = spinand_block_markbad(mtd,addr);
		if(ret){
			printf("mark bad block error, there will occur error,so exit !\n");
			return -1;
		}
	}
	instr->state = MTD_ERASE_DONE;
	return ret;
}
static int spinand_read_oob(struct mtd_info *mtd,loff_t addr,struct mtd_oob_ops *ops)
{
	int len = ops->ooblen;
	int column = mtd->writesize;
	unsigned char cmd[COMMAND_MAX_LENGTH];
	volatile unsigned char read_buf;
	int page_size = mtd->writesize;
	int page = addr / page_size;
	int timeout = 2000;
	u_char *buffer = ops->oobbuf;

	jz_cs_reversal();
	cmd[0] = CMD_PARD;
	cmd[1] = (page >> 16) & 0xff;
	cmd[2] = (page >> 8) & 0xff;
	cmd[3] = page & 0xff;
	spi_send_cmd(cmd, 4);
	udelay(t_read);

	do{
		jz_cs_reversal();
		cmd[0] = CMD_GET_FEATURE;
		cmd[1] = FEATURE_ADDR;
		spi_send_cmd(cmd, 2);

		spi_recv_cmd(&read_buf, 1);
		timeout--;
	}while((read_buf & SPINAND_IS_BUSY) && (timeout > 0));

	if((read_buf & 0x30) == 0x20) {
		printf("%s %d read error pageid = %d!!!\n",__func__,__LINE__,page);
		memset(ops->oobbuf,0,ops->ooblen);
		return 1;
	}
	jz_cs_reversal();
	switch(column_cmdaddr_bits){
		case 24:
			cmd[0] = CMD_R_CACHE;
			cmd[1] = (column >> 8) & 0xff;
			cmd[2] = column & 0xff;
			cmd[3] = 0x0;

			spi_send_cmd(cmd, 4);
			break;
		case 32:
			cmd[0] = CMD_FR_CACHE;/* CMD_R_CACHE read odd addr may be error */
			cmd[1] = 0x0;
			cmd[2] = (column >> 8) & 0xff;
			cmd[3] = column & 0xff;
			cmd[4] = 0;/*dummy*/

			spi_send_cmd(cmd, 5);
			break;
		default:
			printk("can't support the column addr format !!!\n");
			break;
	}
	spi_recv_cmd(buffer, len);

	return 0;
}
static int badblk_check(int len,unsigned char *buf)
{
	int i,bit0_cnt = 0;
	unsigned short *check_buf = (unsigned short *)buf;

	if(check_buf[0] != 0xff){
		for(i = 0; i < len * 8; i++){
			if(!((check_buf[0] >> i) & 0x1))
				bit0_cnt++;
		}
	}
	if(bit0_cnt > 6 * len)
		return 1; // is bad blk

	return 0;
}
static int spinand_block_checkbad(struct mtd_info *mtd, loff_t ofs,int getchip,int allowbbt)
{
	struct nand_chip *chip = mtd->priv;

	if (!(chip->options & NAND_BBT_SCANNED)) {
		chip->options |= NAND_BBT_SCANNED;
		chip->scan_bbt(mtd);
	}

	if (!chip->bbt)
		return chip->block_bad(mtd, ofs,getchip);

	/* Return info from the table */
	return nand_isbad_bbt(mtd, ofs, allowbbt);
}

static int blkid_bak = -1;
static int spinand_block_isbad(struct mtd_info *mtd,loff_t ofs)
{
	int ret = 0;
	int block_id = ofs / mtd->erasesize;
	if(block_id != blkid_bak){
		ret = spinand_block_checkbad(mtd, ofs,1, 0);
		blkid_bak = block_id;
	}
	return ret;
}
static int jz_spinand_block_bad_check(struct mtd_info *mtd, loff_t ofs,int getchip)
{
	int column = mtd->writesize;
	int check_len = 2;
	unsigned char check_buf[] = {0xaa,0xaa};
	unsigned char  cmd[COMMAND_MAX_LENGTH];
	struct mtd_oob_ops ops;

	ops.oobbuf = check_buf;
	ops.ooblen = check_len;
	spinand_read_oob(mtd,ofs,&ops);

	if(badblk_check(check_len,check_buf))
		return 1;
	return 0;
}
static int spinand_read(struct mtd_info *mtd,loff_t addr,size_t len,size_t *retlen,u_char *buf)
{
	int ret;

	ret = spi_nand_read(mtd,addr,0,len,buf);
	if(ret)
		*retlen += ret;
	else
		*retlen += len;

	return 0;
}
static int spinand_write(struct mtd_info *mtd,loff_t addr,size_t len,size_t *retlen, u_char *buf)
{
	int ret,i,n=0;
	int readv;

	ret = spi_nand_write(mtd,addr,0,len,buf);
	*retlen = ret;

	return ret;
}
static void mtd_spinand_init(struct mtd_info *mtd)
{
	mtd->_erase = spinand_erase;
	mtd->_point = NULL;
	mtd->_unpoint = NULL;
	mtd->_read = spinand_read;
	mtd->_write = spinand_write;
	mtd->_read_oob = spinand_read_oob;
	mtd->_write_oob = spinand_write_oob;
	mtd->_lock = NULL;
	mtd->_unlock = NULL;
	mtd->_block_isbad = spinand_block_isbad;
	mtd->_block_markbad = spinand_block_markbad;
}

static int read_spinand_id(void *response,size_t len)
{
	struct spi_slave *spi;
	int ret;
	int cmd[2];

	spi = spi_setup_slave(0, 0, 24000000, SPI_MODE_3);
	if (!spi) {
		printf("SF: Failed to set up slave\n");
		return -1;
	}
	ret = spi_claim_bus(spi);
	if (ret) {
		debug("SF: Failed to claim SPI bus: %d\n", ret);
	}

	cmd[0] = CMD_READ_ID;
	cmd[1] = 0;
	spi_flash_cmd_read(spi,&cmd,2,response,len);

	spi_release_bus(spi);
}
static void jz_spinand_ext_init(void )
{
	unsigned char cmd[COMMAND_MAX_LENGTH];
	unsigned char read_cmd[COMMAND_MAX_LENGTH];

	/* disable write protect */
	jz_cs_reversal();
	cmd[0] = 0x1f;
	cmd[1] = 0xa0;
	cmd[2] = 0x0;
	spi_send_cmd(cmd, 3);

	/* enable ECC */
	jz_cs_reversal();
	cmd[0] = 0x1f;
	cmd[1] = 0xb0;
	cmd[2] = 0x10;
	spi_send_cmd(cmd, 3);

	/* check */
	jz_cs_reversal();
	cmd[0] = 0x0f;
	cmd[1] = 0xa0;
	spi_send_cmd(cmd, 2);
	spi_recv_cmd(read_cmd, 1);
	if(read_cmd[0] != 0x0)
		printf("read status 0xa0 : %x\n", read_cmd[0]);

	jz_cs_reversal();
	cmd[0] = 0x0f;
	cmd[1] = 0xb0;
	spi_send_cmd(cmd, 2);
	spi_recv_cmd(read_cmd, 1);
	if(read_cmd[0] != 0x10)
		printf("read status 0xb0 : %x\n", read_cmd[0]);

}
static int jz_spinand_block_markbad(struct mtd_info *mtd, loff_t ofs)
{
	struct nand_chip *chip = mtd->priv;
	uint8_t buf[2] = { 0, 0 };
	int block, res, ret = 0, i = 0;
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
			res = spinand_write_oob(mtd, wr_ofs, &ops);
			if (!ret)
				ret = res;

			i++;
			wr_ofs += mtd->writesize;
		} while ((chip->bbt_options & NAND_BBT_SCAN2NDPAGE) && i < 2);
	}

	/* Update flash-based bad block table */
	if (chip->bbt_options & NAND_BBT_USE_FLASH) {
		res = nand_update_bbt(mtd, ofs);
		if (!ret)
			ret = res;
	}

	if (!ret)
		mtd->ecc_stats.badblocks++;

	return ret;

}

static struct jz_spi_support_from_burner *spi_nandflash_probe(u8 *idcode,struct nand_param_from_burner *param_array)
{
	int i;
	struct jz_spi_support_from_burner *params=param_array->addr;
	printf("flash id=%04x\n",(idcode[0]<<8)|idcode[1]);
//	printf("sfcnand param num=%d\n",param_array->para_num);
	for (i = 0; i < param_array->para_num; i++) {
		if(params->id_manufactory == ((idcode[0]<<8) | idcode[1]))
			break;
		params++;
	}

	if (i == param_array->para_num ) {
		printf("ingenic: Unsupported ID %04x\n", (idcode[0]<<8)|idcode[1]);
		return NULL;
	}
#if 0
	printf("***********************chip information*************************\n");
	printf("param num=%x\n",param_array->para_num);
	printf("param id=%x\n",params->id_manufactory);
	printf("param name=%s\n",params->name);

	printf("param name=%s\n",params->name);
	printf("param page_size=%d\n",params->page_size);
	printf("param oobsize=%d\n",params->oobsize);
	printf("param size=%d\n",params->size);
	printf("param block_size=%d\n",params->block_size);
	printf("param page_num=%d\n",params->page_num);
	printf("param tRD_maxbusy=%d\n",params->tRD_maxbusy);
	printf("param tPROG_maxbusy=%d\n",params->tPROG_maxbusy);
	printf("param tBERS_maxbusy=%d\n",params->tBERS_maxbusy);
	printf("param column_cmdaddr_bits=%d\n",params->column_cmdaddr_bits);
	printf("partition_num=%d\n",param_array->partition_num);
	for(i=0;i<param_array->partition_num;i++){
		printf("partition%d name=%s   ",i,param_array->partition[i].name);
		printf("partition%d size=0x%x   ",i,param_array->partition[i].size);
		printf("partition%d offset=0x%x   ",i,param_array->partition[i].offset);
		printf("partition%d flags=0x%x   ",i,param_array->partition[i].mask_flags);
		printf("partition%d manager_mode=0x%x\n",i,param_array->partition[i].manager_mode);
	}
	printf("***********************end************************************\n");
#endif
	param_array->addr=params;
	param_array->para_num=1;
	return params;
}
static void jz_spi_get_param(char *buffer,struct nand_param_from_burner *param)
{
	char * member_addr=buffer+sizeof(int32_t);
	memcpy(&param->version,member_addr,sizeof(param->version));
	member_addr+=sizeof(param->version);
	memcpy(&param->flash_type,member_addr,sizeof(param->flash_type));
	member_addr+=sizeof(param->flash_type);
        param->para_num=*(int *)member_addr;
	member_addr+=sizeof(param->para_num);
        param->addr=member_addr;
	member_addr+=param->para_num*sizeof(struct jz_spi_support_from_burner);
        param->partition_num=*(int *)member_addr;
	member_addr+=sizeof(param->partition_num);
        param->partition=member_addr;
}
static int get_spinand_pagesize(int page,int column)
{
       char buffer[100];
       int pagesize=0;
       spi_nand_read_page(buffer,page,column,100);
       pagesize=buffer[SPL_TYPE_FLAG_LEN+5]*1024;
       return pagesize;
}
static int32_t get_nand_magic(int page,int column)
{
        char buffer[column+sizeof(int)];
        int32_t nand_magic=0;
        spi_nand_read_page(buffer,page,0,column+sizeof(int));
        nand_magic=*(int32_t *)(buffer+column);
        return nand_magic;
}
static int get_spinand_param(char *buffer,int ptcount,struct nand_param_from_burner *param,int pagesize)
{
        int offset=ptcount % pagesize;
        spi_nand_read_page(buffer,ptcount/pagesize,0,pagesize);
        jz_spi_get_param(buffer+offset,param);
}
/********************************************************************************************
 * get spinand params from nand ,
 *addr in CONFIG_SPIFLASH_PART_OFFSET,
 *you must check addr is in same page,
 * *****************************************************************************************/
static char *get_chip_param_from_nand(struct nand_param_from_burner **param,int *using_way)
{
        int pagesize=0;
        char *buffer=NULL;
        int32_t nand_magic;
        pagesize=get_spinand_pagesize(0,0);
        nand_magic=get_nand_magic(CONFIG_SPIFLASH_PART_OFFSET/pagesize,CONFIG_SPIFLASH_PART_OFFSET%pagesize);
        if(nand_magic==0x6e616e64)
        {
                *param=malloc(sizeof(struct nand_param_from_burner));
                buffer=malloc(pagesize);
                get_spinand_param(buffer,CONFIG_SPIFLASH_PART_OFFSET,*param,pagesize);
                *using_way=0;
        }else{
		printf("warning: can't get right magic\n");
                *using_way=1;
        }
        return buffer;
}
static void tran_old_burnway(struct nand_param_from_burner **param)
{
        int i;
        *param=malloc(sizeof(struct nand_param_from_burner));
        (*param)->version=-1;
        (*param)->flash_type=1;
        (*param)->para_num=ARRAY_SIZE(jz_spi_nand_support_table);
        (*param)->addr=malloc((*param)->para_num*sizeof(struct jz_spi_support_from_burner));
        for(i=0;i<(*param)->para_num;i++){
                (((*param)->addr)[i]).id_manufactory=jz_spi_nand_support_table[i].id_manufactory*256+
			jz_spi_nand_support_table[i].id_device;
                (((*param)->addr)[i]).id_device=jz_spi_nand_support_table[i].id_device;
                memcpy((((*param)->addr)[i]).name,jz_spi_nand_support_table[i].name,32);
                (((*param)->addr)[i]).page_size=jz_spi_nand_support_table[i].page_size;
                (((*param)->addr)[i]).oobsize=jz_spi_nand_support_table[i].oobsize;
                (((*param)->addr)[i]).sector_size=jz_spi_nand_support_table[i].sector_size;
                (((*param)->addr)[i]).block_size=jz_spi_nand_support_table[i].block_size;
                (((*param)->addr)[i]).size=jz_spi_nand_support_table[i].size;
                (((*param)->addr)[i]).page_num=jz_spi_nand_support_table[i].page_num;
                (((*param)->addr)[i]).column_cmdaddr_bits=jz_spi_nand_support_table[i].column_cmdaddr_bits;
        }
        (*param)->partition_num=ARRAY_SIZE(jz_mtd_spinand_partition);
        (*param)->partition=jz_mtd_spinand_partition;
}
#define IDCODE_LEN (IDCODE_CONT_LEN + IDCODE_PART_LEN)
int jz_spi_nand_init(struct nand_param_from_burner *param)
{
	u8 idcode[IDCODE_LEN + 1];
	struct nand_chip *chip;
	struct mtd_info *mtd;
	struct jz_spi_support_from_burner *spi_flash;
	int using_way;
	mtd = &nand_info[0];

	spi_init();
	//printf("------->> idcode0 = %02x idcode1 = %02x idcode2 = %02x idcode3 = %02x\n",idcode[0],idcode[1],idcode[2],idcode[3]);
#ifndef CONFIG_BURNER
	char *buffer=get_chip_param_from_nand(&param,&using_way);
	if(using_way==1)                //use old way
	{
		tran_old_burnway(&param);
	}
#endif
	read_spinand_id(idcode,sizeof(idcode));
	spi_flash = spi_nandflash_probe(idcode,param);
	chip = malloc(sizeof(struct nand_chip));
	if (!chip)
		return -ENOMEM;
	memset(chip,0,sizeof(struct nand_chip));

	mtd->size = spi_flash->size;
	mtd->flags |= MTD_CAP_NANDFLASH;
	mtd->erasesize = spi_flash->block_size;
	mtd->writesize = spi_flash->page_size;
	mtd->oobsize = spi_flash->oobsize;

	column_cmdaddr_bits = spi_flash->column_cmdaddr_bits;

	chip->select_chip = NULL;
	chip->badblockbits = 8;
	chip->scan_bbt = nand_default_bbt;
	chip->block_bad = jz_spinand_block_bad_check;
	chip->block_markbad = jz_spinand_block_markbad;
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

	jz_spinand_ext_init();
	mtd_spinand_init(mtd);
	nand_register(0);
	return 0;
}
static int mtd_spinand_partition_analysis(unsigned int blk_sz,int partcount,struct jz_spinand_partition *jz_mtd_spinand_partition)
{
	char mtdparts_env[X_ENV_LENGTH];
	char command[X_COMMAND_LENGTH];
	int ptcount = partcount;
	int part, ret;

	memset(mtdparts_env, 0, X_ENV_LENGTH);
	memset(command, 0, X_COMMAND_LENGTH);

	/*MTD part*/
	sprintf(mtdparts_env, "mtdparts=nand:");
//	printf("partcount=%d\n",partcount);
	for (part = 0; part < ptcount; part++) {
		if (jz_mtd_spinand_partition[part].size == -1) {
			sprintf(mtdparts_env,"%s-(%s)", mtdparts_env,
					jz_mtd_spinand_partition[part].name);
			break;
		} else if (jz_mtd_spinand_partition[part].size  != 0) {
			if(jz_mtd_spinand_partition[part].size % blk_sz != 0)
				    printf("ERROR:the partition [%s] don't algin as block size [0x%08x] ,it will be error !\n",jz_mtd_spinand_partition[part].name,blk_sz);

			sprintf(mtdparts_env,"%s%dK(%s),", mtdparts_env,
					jz_mtd_spinand_partition[part].size / 0x400,
					jz_mtd_spinand_partition[part].name);
		} else
			break;
	}
	debug("env:mtdparts=%s\n", mtdparts_env);
	setenv("mtdids", mtdids_default);
	setenv("mtdparts", mtdparts_env);
	setenv("partition", NULL);
}
struct jz_spinand_partition *get_partion_index(u32 startaddr,u32 length,int *pt_index)
{
	int ptcount = nand_param_from_burner.partition_num;
	int i;
	struct jz_spinand_partition *jz_mtd_spinand_partition=nand_param_from_burner.partition;
	for(i = 0; i < ptcount; i++){
		if(startaddr >= jz_mtd_spinand_partition[i].offset && (startaddr + length) <= (jz_mtd_spinand_partition[i].offset + jz_mtd_spinand_partition[i].size)){
			*pt_index = i;
			break;
		}
	}
	if(i >= ptcount){
		printf("startaddr %x can't find the pt_index or you partition size  is not align with 128K\n",startaddr);
		*pt_index = -1;
		return NULL;
	}
	return &jz_mtd_spinand_partition[i];
}
void get_info_to_spl(databuf){

}
int mtd_spinand_probe_burner(int *erase_mode,struct  nand_param_from_burner *param)
{
	int ret;
	struct mtd_info *mtd;
	mtd = &nand_info[0];
	struct nand_chip *chip;
	//int wppin = pinfo->gpio_wp;

	ret = jz_spi_nand_init(param);
	/*0: none 1, force-erase*/
	if (*erase_mode == 1)
		ret = run_command("nand erase.chip -y", 0);

	chip = mtd->priv;
	chip->scan_bbt(mtd);
	chip->options |= NAND_BBT_SCANNED;
	mtd_spinand_partition_analysis(mtd->erasesize,param->partition_num,param->partition);
	return 0;
}
