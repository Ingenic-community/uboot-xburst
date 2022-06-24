/*
 * (C) Copyright 2006-2008
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <common.h>
#include <nand.h>
#include <asm/io.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/mtd/nand.h>
#include <spl.h>
#include <ingenic_nand_mgr/nand_param.h>
#include "asm/arch/nand.h"
#include "nand_spl_flag.h"
#include "jz_nand.h"

static int nand_ecc_pos;
static nand_info_t mtd;
static struct nand_chip nand_chip;
struct spl_basic_param spl_param;
static int (*nand_command)(int block, int page, uint32_t offs, u8 cmd) = NULL;

static int parse_flag(unsigned char *flag_buf)
{
	int cnt_55 = 0, cnt_aa = 0;
	int i;

	for (i = 0; i < 32; i++) {
		if (flag_buf[i] == 0x55)
			cnt_55++;
		else if (flag_buf[i] == 0xaa)
			cnt_aa++;
	}

	if ((cnt_55 - cnt_aa) > 7)
		return 0x55;
	else if ((cnt_aa - cnt_55) > 7)
		return 0xaa;
	else
		return 0;
}

static void get_nand_buswidth(unsigned char *buswidth, unsigned char *flag_buf)
{
	int flag = parse_flag(flag_buf + BUSWIDTH_FLAG_OFFSET);
	if(flag == FLAG_BUSWIDTH_8BIT)
		*buswidth = 8;
	else if(flag == FLAG_BUSWIDTH_16BIT)
		*buswidth = 16;
	else
		*buswidth = -1;
}

static void get_nand_nandtype(unsigned char *nandtype, unsigned char *flag_buf)
{
	int flag = parse_flag(flag_buf + NANDTYPE_FLAG_OFFSET);
	if(flag == FLAG_NANDTYPE_COMMON)
		*nandtype = 0;
	else if(flag == FLAG_NANDTYPE_TOGGLE)
		*nandtype = 1;
	else
		*nandtype = -1;
}

static inline void get_nand_rowcycles(unsigned char *rowcycles, unsigned char *flag_buf)
{
#ifdef NANDTYPE_ROWCYCLE
	*rowcycles = *(uint32_t *)(flag_buf + NANDTYPE_ROWCYCLE);
#else
	int flag = parse_flag(flag_buf + ROWCYCLE_FLAG_OFFSET);
	if(flag == FLAG_ROWCYCLE_3)
		*rowcycles = 3;
	else if(flag == FLAG_ROWCYCLE_2)
		*rowcycles = 2;
	else
		*rowcycles = -1;
#endif
}

static inline void get_nand_pagesize(unsigned int *pagesize, unsigned char *flag_buf)
{
#ifdef NANDTYPE_PAGESIZE
	*pagesize = *(uint32_t *)(flag_buf + NANDTYPE_PAGESIZE);
#else
	int pagesize_flag2 = parse_flag(flag_buf + PAGESIZE_FLAG2_OFFSET);
	int pagesize_flag1 = parse_flag(flag_buf + PAGESIZE_FLAG1_OFFSET);
	int pagesize_flag0 = parse_flag(flag_buf + PAGESIZE_FLAG0_OFFSET);
	int pagesize_flag = pagesize_flag2 << 16 | pagesize_flag1 << 8 | pagesize_flag0;

	if (pagesize_flag == FLAG_PAGESIZE_512)
		*pagesize = 512;
	else if (pagesize_flag == FLAG_PAGESIZE_2K)
		*pagesize = 2048;
	else if (pagesize_flag == FLAG_PAGESIZE_4K)
		*pagesize = 4096;
	else if (pagesize_flag == FLAG_PAGESIZE_8K)
		*pagesize = 8192;
	else if (pagesize_flag == FLAG_PAGESIZE_16K)
		*pagesize = 16384;
	else
		*pagesize = -1;
#endif
}

static inline void get_nand_spl_basic_param(struct spl_basic_param *param)
{
	unsigned char *spl_flag = (unsigned char *)CONFIG_SYS_SPL_NAND_FLAG_ADDR;

	get_nand_buswidth(&param->buswidth, spl_flag);
	get_nand_nandtype(&param->nandtype, spl_flag);
	get_nand_rowcycles(&param->rowcycles, spl_flag);
	get_nand_pagesize(&param->pagesize, spl_flag);

	debug("param->buswidth = %d\n", param->buswidth);
	debug("param->nandtype = %d\n", param->nandtype);
	debug("param->rowcycles = %d\n", param->rowcycles);
	debug("param->pagesize = %d\n", param->pagesize);
}

static int nand_command_sp(int block, int page, uint32_t offs,
	u8 cmd)
{
	struct nand_chip *this = mtd.priv;
	int page_addr = page + block * (mtd.erasesize/mtd.writesize);

	while (!this->dev_ready(&mtd))
		;

	/* Begin command latch cycle */
	this->cmd_ctrl(&mtd, cmd, NAND_CTRL_CLE | NAND_CTRL_CHANGE);
	/* Set ALE and clear CLE to start address cycle */
	/* Column address */
	this->cmd_ctrl(&mtd, offs, NAND_CTRL_ALE | NAND_CTRL_CHANGE);
	this->cmd_ctrl(&mtd, page_addr & 0xff, NAND_CTRL_ALE); /* A[16:9] */
	this->cmd_ctrl(&mtd, (page_addr >> 8) & 0xff,
		       NAND_CTRL_ALE); /* A[24:17] */

	if (spl_param.rowcycles == 2) {
		/* One more address cycle for devices > 32MiB */
		this->cmd_ctrl(&mtd, (page_addr >> 16) & 0x0f,
				NAND_CTRL_ALE); /* A[28:25] */
	}

	/* Latch in address */
	this->cmd_ctrl(&mtd, NAND_CMD_NONE, NAND_NCE | NAND_CTRL_CHANGE);

	/*
	 * Wait a while for the data to be ready
	 */
	while (!this->dev_ready(&mtd))
		;

	return 0;
}
/*
 * NAND command for large page NAND devices (2k)
 */
static int nand_command_lp(int block, int page, uint32_t offs,
	u8 cmd)
{
	struct nand_chip *this = mtd.priv;
	int page_addr = page + block * (mtd.erasesize/mtd.writesize);
	void (*hwctrl)(struct mtd_info *mtd, int cmd,
			unsigned int ctrl) = this->cmd_ctrl;

	while (!this->dev_ready(&mtd))
		;

	/* Emulate NAND_CMD_READOOB */
	if (cmd == NAND_CMD_READOOB) {
		offs += mtd.writesize;
		cmd = NAND_CMD_READ0;
	}

	/* Shift the offset from byte addressing to word addressing. */
	if (this->options & NAND_BUSWIDTH_16)
		offs >>= 1;

	/* Begin command latch cycle */
	hwctrl(&mtd, cmd, NAND_CTRL_CLE | NAND_CTRL_CHANGE);
	/* Set ALE and clear CLE to start address cycle */
	/* Column address */
	hwctrl(&mtd, offs & 0xff,
		       NAND_CTRL_ALE | NAND_CTRL_CHANGE); /* A[7:0] */
	hwctrl(&mtd, (offs >> 8) & 0xff, NAND_CTRL_ALE); /* A[11:9] */
	/* Row address */
	hwctrl(&mtd, (page_addr & 0xff), NAND_CTRL_ALE); /* A[19:12] */
	hwctrl(&mtd, ((page_addr >> 8) & 0xff),
		       NAND_CTRL_ALE); /* A[27:20] */

	if (spl_param.rowcycles == 3) {
		/* One more address cycle for devices > 128MiB */
		hwctrl(&mtd, (page_addr >> 16) & 0x0f,
				NAND_CTRL_ALE); /* A[31:28] */
	}

	/* Latch in address */
	hwctrl(&mtd, NAND_CMD_READSTART,
		       NAND_CTRL_CLE | NAND_CTRL_CHANGE);
	hwctrl(&mtd, NAND_CMD_NONE, NAND_NCE | NAND_CTRL_CHANGE);

	/*
	 * Wait a while for the data to be ready
	 */
	while (!this->dev_ready(&mtd))
		;
	return 0;
}

static int nand_is_bad_block(int block)
{
	struct nand_chip *this = mtd.priv;

	nand_command(block, 0, 0, NAND_CMD_READOOB);
	/*
	 * Read one byte (or two if it's a 16 bit chip).
	 */
	if (this->options & NAND_BUSWIDTH_16) {
		if (readw(this->IO_ADDR_R) != 0xffff)
			return 1;
	} else {
		if (readb(this->IO_ADDR_R) != 0xff)
			return 1;
	}

	return 0;
}

static int nand_read_page(int block, int page, uchar *dst)
{
	struct nand_chip *this = mtd.priv;
	u_char ecc_calc[640];
	u_char ecc_code[640];
	u_char oob_data[640];
	int i;
	int eccsize = this->ecc.size;
	int eccbytes = this->ecc.bytes;
	int eccsteps = this->ecc.steps;
	uint8_t *p = dst;

	nand_command(block, page, 0, NAND_CMD_READOOB);
	this->read_buf(&mtd, oob_data, mtd.oobsize);
	nand_command(block, page, 0, NAND_CMD_READ0);

	/* Pick the ECC bytes out of the oob data */
	for (i = 0; i < eccsteps * eccbytes; i++)
		ecc_code[i] = oob_data[nand_ecc_pos + i];


	for (i = 0; eccsteps; eccsteps--, i += eccbytes, p += eccsize) {
		this->ecc.hwctl(&mtd, NAND_ECC_READ);
		this->read_buf(&mtd, p, eccsize);
		this->ecc.calculate(&mtd, p, &ecc_calc[i]);
		this->ecc.correct(&mtd, p, &ecc_code[i], &ecc_calc[i]);
	}

	return 0;
}

int nand_spl_load_image(uint32_t offs, unsigned int size, void *dst)
{
	unsigned int block, lastblock;
	unsigned int page;

	/*
	 * offs has to be aligned to a page address!
	 */
	block = offs / mtd.erasesize;
	lastblock = (offs + size - 1) / mtd.erasesize;
	page = (offs % mtd.erasesize) / mtd.writesize;

	while (block <= lastblock) {
		if (!nand_is_bad_block(block)) {
			/*
			 * Skip bad blocks
			 */
			while (page < (mtd.erasesize/mtd.writesize)) {
				nand_read_page(block, page, dst);
				dst +=  mtd.writesize;
				page++;
			}

			page = 0;
		} else {
			lastblock++;
		}
		block++;
	}
	return 0;
}

extern int jz_spl_bch64_correct(u_char *dat,u_char *read_ecc);

void read_nand_param(void)
{
	int param_start_pages = 6 * 128;
	int param_end_pages = 7 * 128;
	int ret = -1, pages, i, page = 0;
	void *buf, *ecc_buf;


	pages = sizeof(struct mtd_nand_params)/mtd.writesize;
	if (sizeof(struct mtd_nand_params)%mtd.writesize)
		pages += 1;
	do {
		ecc_buf = (void *)(CONFIG_BOARD_TCSM_BASE + mtd.writesize * pages);
		buf = (void *)CONFIG_BOARD_TCSM_BASE;
		for (page = param_start_pages; page < (pages * 2) + param_start_pages; page += 2) {
			nand_command(0, page, 0x0, NAND_CMD_READ0);
			jz_nand_set_pn(&mtd, mtd.writesize, 0, 0);
			nand_chip.read_buf(&mtd, buf, mtd.writesize);
			nand_command(0, page + 1, 0x0, NAND_CMD_READ0);
			jz_nand_set_pn(&mtd, mtd.writesize, 0, 0);
			nand_chip.read_buf(&mtd, ecc_buf, mtd.writesize);

			for (i = 0; i < mtd.writesize/SPL_ECC_SIZE; i++) {
				ret = jz_spl_bch64_correct(&(((u8 *)buf)[i * SPL_ECC_SIZE]),
						&(((u8 *)ecc_buf)[i * SPL_ECC_BYTE]));
				if (ret < 0) {
					printf("nand param is unused ecc err\n");
					break;
				}
			}
			buf += mtd.writesize;
		}
		param_start_pages += 128;
	} while (ret < 0 && page < param_end_pages);
	jz_nand_set_pn(&mtd, 0, 0, 0);
}

/* nand_init() - initialize data to make nand usable by SPL */
static struct jz_nand_priv nand_privs[1];
extern void jz_nand_read_buf(struct mtd_info *mtd, uint8_t *buf, int len);
void nand_init(void)
{
	/*
	 * Init board specific nand support
	 */
	struct mtd_nand_params *mtd_nand_params = (struct mtd_nand_params *)CONFIG_BOARD_TCSM_BASE;
	nand_flash_param *nand_params = &mtd_nand_params->nand_params;

	get_nand_spl_basic_param(&spl_param);
	mtd.priv = &nand_chip;
	nand_chip.cmd_ctrl		= jz_nand_cmd_ctrl;
	nand_chip.chip_delay		= 50;
	nand_chip.dev_ready		= jz_nand_device_ready;
	nand_chip.read_buf		= jz_nand_read_buf;
	nand_chip.priv			= &nand_privs[0];
	nand_privs[0].pn_bytes		= 0;
	nand_privs[0].pn_size		= 0;
	nand_privs[0].pn_skip		= 0;

	nand_chip.options = spl_param.buswidth == 16 ? NAND_BUSWIDTH_16 : 0;
	jz_nand_init(&nand_chip, NULL, spl_param.buswidth);
	mtd.writesize = spl_param.pagesize;
	if (spl_param.pagesize > 512)
		nand_command = nand_command_lp;
	else
		nand_command = nand_command_sp;

	if (nand_chip.select_chip)
		nand_chip.select_chip(&mtd, 0);

	read_nand_param();

	debug("nand_params->pagesize %d\n", nand_params->pagesize);
	debug("nand_params->oobsize %d\n", nand_params->oobsize);
	debug("nand_params->eccbit %d\n", nand_params->eccbit);
	debug("nand_params->blocksize %d\n", nand_params->blocksize);
	if (nand_params->pagesize != mtd.writesize)
		return;
	mtd.erasesize = nand_params->blocksize;
	mtd.oobsize = nand_params->oobsize;

	nand_chip.ecc.mode		= NAND_ECC_HW_OOB_FIRST;
	nand_chip.ecc.hwctl		= jz_nand_hwctl;
	nand_chip.ecc.correct		= jz_nand_correct_data;
	nand_chip.ecc.calculate		= jz_nand_calculate_ecc;
	nand_chip.ecc.size = CONFIG_SYS_NAND_ECCSIZE;
	nand_chip.ecc.strength = nand_params->eccbit;
	nand_chip.ecc.bytes = nand_chip.ecc.strength * 14/8;
	nand_chip.ecc.steps = mtd.writesize/nand_chip.ecc.size;
	nand_ecc_pos = mtd.oobsize - (nand_chip.ecc.bytes * nand_chip.ecc.steps);

	debug("nand_ecc_pos = %d\n", nand_ecc_pos);
}

/* Unselect after operation */
void nand_deselect(void)
{
	if (nand_chip.select_chip)
		nand_chip.select_chip(&mtd, -1);
}

extern void spl_parse_image_header(const struct image_header *header);
struct spl_image_info spl_image;
void spl_nand_load_image(void)
{
	struct image_header *header;
	int *src __attribute__((unused));
	int *dst __attribute__((unused));

	debug("spl: nand - using hw ecc\n");
	nand_init();

	/*use CONFIG_SYS_TEXT_BASE as temporary storage area */
	header = (struct image_header *)(CONFIG_SYS_TEXT_BASE);

	/* Load u-boot */
	nand_spl_load_image((mtd.writesize * 128 * 8),
			mtd.writesize, (void *)header);
	spl_parse_image_header(header);
	nand_spl_load_image((mtd.writesize * 128 * 8),
			spl_image.size, (void *)spl_image.load_addr);
	nand_deselect();
}
