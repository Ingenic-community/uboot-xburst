/*
 * jz mtd nand driver probe interface
 *
 *  Copyright (C) 2013 Ingenic Semiconductor Co., LTD.
 *  Author: cli <chen.li@ingenic.com>
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
#define DEBUG
#include <common.h>
#include <malloc.h>
#include <errno.h>
#include <nand.h>
#include <linux/list.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <asm/arch/nand.h>
#include "jz_nand.h"

#if defined(CONFIG_MTD_NAND_JZ_PN) || (defined(CONFIG_MTD_NAND_JZ_AUTO_PARAMS) && defined(CONFIG_SPL_BUILD)) || defined(CONFIG_BURNER)
static struct jz_nand_priv nand_privs[1];
void jz_nand_read_buf(struct mtd_info *mtd, uint8_t *buf, int len)
{
	struct nand_chip *this = mtd->priv;
	struct jz_nand_priv *priv = this->priv;
	uint32_t off, pn_len;
	int i;

	if (priv->pn_bytes && priv->pn_size) {
		/* apply pseudo-random noise to each pn_size block */
		for (i = 0; i < (len / priv->pn_size); i++) {
			jz_nand_enable_pn(priv->pn_bytes && !priv->pn_skip, true);
			if (priv->pn_skip)
				priv->pn_skip--;
			off = i * priv->pn_size;
			pn_len = MIN(priv->pn_size, len - off);
			nand_read_buf(mtd, &buf[off], pn_len);
			priv->pn_bytes = MAX(0, priv->pn_bytes - pn_len);
		}
	} else {
		jz_nand_enable_pn(!!priv->pn_bytes, false);
		nand_read_buf(mtd, buf, len);
		priv->pn_bytes = MAX(0, priv->pn_bytes - len);
	}

	jz_nand_enable_pn(!!priv->pn_bytes, false);
}

static void jz_nand_write_buf(struct mtd_info *mtd, const uint8_t *buf, int len)
{
	struct nand_chip *this = mtd->priv;
	struct jz_nand_priv *priv = this->priv;
	uint32_t off, pn_len;
	int i;
	if (priv->pn_bytes && priv->pn_size) {
		/* apply pseudo-random noise to each pn_size block */
		for (i = 0; i < (len / priv->pn_size); i++) {
			jz_nand_enable_pn(priv->pn_bytes && !priv->pn_skip, true);
			if (priv->pn_skip)
				priv->pn_skip--;
			off = i * priv->pn_size;
			pn_len = MIN(priv->pn_size, len - off);
			nand_write_buf(mtd, &buf[off], pn_len);
			priv->pn_bytes = MAX(0, priv->pn_bytes - pn_len);
		}
	} else {
		jz_nand_enable_pn(!!priv->pn_bytes, false);
		nand_write_buf(mtd, buf, len);
		priv->pn_bytes = MAX(0, priv->pn_bytes - len);
	}
	jz_nand_enable_pn(!!priv->pn_bytes, false);
}

void jz_nand_set_pn(nand_info_t *nand, int bytes, int size, int skip)
{
	struct nand_chip *chip = nand->priv;
	struct jz_nand_priv *priv = chip->priv;
	priv->pn_bytes = bytes;
	priv->pn_size = size;
	priv->pn_skip = skip;
}
#endif	/* CONFIG_MTD_NAND_JZ_PN || (CONFIG_MTD_NAND_JZ_AUTO_PARAMS && CONFIG_SPL_BUILD) || defined(CONFIG_BURNER)*/

#if defined(CONFIG_MTD_NAND_JZ_AUTO_PARAMS) || defined(CONFIG_BURNER)
static void* zalloc(size_t size) {
	void *p = 0;

	p = malloc(size);
	memset(p, 0, size);
	return p;
}

static int jz_nand_auto_init_ecc(struct mtd_info *mtd, struct nand_chip* nand_chip, int strength)
{
	int i;

	struct nand_ecclayout *nand_ecclayout = (struct nand_ecclayout *)zalloc(sizeof(struct nand_ecclayout));

	if (!nand_ecclayout)
		return -ENOMEM;
	nand_chip->ecc.size		= CONFIG_SYS_NAND_ECCSIZE;
	nand_chip->ecc.strength		= strength;
	nand_chip->ecc.bytes		= nand_chip->ecc.strength * 14/8;
	nand_chip->ecc.steps		= mtd->writesize/nand_chip->ecc.size;
	nand_chip->ecc.mode		= NAND_ECC_HW_OOB_FIRST;
	nand_chip->ecc.hwctl		= jz_nand_hwctl;
	nand_chip->ecc.correct		= jz_nand_correct_data;
	nand_chip->ecc.calculate	= jz_nand_calculate_ecc;
	nand_chip->ecc.layout		= nand_ecclayout;

	nand_ecclayout->eccbytes = nand_chip->ecc.bytes * nand_chip->ecc.steps;
	nand_ecclayout->oobavail = mtd->oobsize - nand_ecclayout->eccbytes - 2	/*bab block mark*/;
	nand_ecclayout->oobfree[0].offset = 2;
	nand_ecclayout->oobfree[0].length = nand_ecclayout->oobavail;
	for (i = 0; i < nand_ecclayout->eccbytes; i++) {
		nand_ecclayout->eccpos[i] = nand_ecclayout->oobavail + 2 + i;
	}
#ifdef CONFIG_MTD_NAND_BITFLIP_THRESHOLD
	mtd->bitflip_threshold = CONFIG_MTD_NAND_BITFLIP_THRESHOLD;
#else
	mtd->bitflip_threshold = nand_chip->ecc.strength / 3;
#endif
	return 0;
}

int mtd_nand_auto_init(nand_flash_param* nand_param)
{
	nand_timing_param  *timing = &nand_param->timing;
	struct nand_chip *nand_chip;
	struct nand_flash_dev nand_flash_dev[2];
	struct mtd_info *mtd;
	int ret = 0;

	mtd = &nand_info[0];
	nand_chip = zalloc(sizeof(struct nand_chip));
	if (!nand_chip)
		return -ENOMEM;

	/*nand_chip init*/
	mtd->priv = nand_chip;
	nand_chip->cmd_ctrl		= jz_nand_cmd_ctrl;
	nand_chip->select_chip		= jz_nand_select_chip;
	nand_chip->chip_delay		= 50;
	nand_chip->dev_ready		= jz_nand_device_ready;
#if defined(CONFIG_MTD_NAND_JZ_PN) || defined(CONFIG_BURNER)
	nand_chip->read_buf		= jz_nand_read_buf;
	nand_chip->write_buf		= jz_nand_write_buf;
	nand_chip->priv			= &nand_privs[0];
	nand_privs[0].pn_bytes		= 0;
	nand_privs[0].pn_size		= 0;
	nand_privs[0].pn_skip		= 0;
#endif
	nand_chip->bbt_options		= NAND_BBT_USE_FLASH;
	nand_chip->options		= NAND_NO_SUBPAGE_WRITE;

	ret = jz_nand_init(nand_chip, timing, nand_param->buswidth);
	if (ret)
		return ret;

	memset(nand_flash_dev, 0, 2*sizeof(struct nand_flash_dev));
	nand_flash_dev[0].name = zalloc(32*sizeof(char));
	strcpy(nand_flash_dev[0].name, nand_param->name);
	nand_flash_dev[0].id = (nand_param->id) & 0xff;
	nand_flash_dev[0].pagesize = nand_param->pagesize;
	nand_flash_dev[0].erasesize = nand_param->blocksize;
	nand_flash_dev[0].chipsize = (nand_param->blocksize/1024)*
		nand_param->totalblocks/1024;	/*Mbytes*/
	nand_flash_dev[0].chipsize = 1 << (fls(nand_flash_dev[0].chipsize) - 1); /*Align*/
	if (nand_param->buswidth == 16)
		nand_flash_dev[0].options = NAND_BUSWIDTH_16;

	ret = nand_scan_ident(mtd, nand_param->chips, nand_flash_dev);
	if (ret)
		return ret;

	mtd->oobsize = nand_param->oobsize;
	ret = jz_nand_auto_init_ecc(mtd, nand_chip, nand_param->eccbit);
	if (ret)
		return ret;

	ret = nand_scan_tail(mtd);
	if (ret)
		return ret;

	//dump_nand_info(nand_chip);
	nand_register(0);
	return ret;
}
#else	 /* CONFIG_MTD_NAND_JZ_AUTO_PARAMS */
#include "chips.h"
#ifndef CONFIG_SPL_BUILD
#define ECCBYTES_ONE_PAGE ((CONFIG_SYS_NAND_PAGE_SIZE/CONFIG_SYS_NAND_ECCSIZE)*CONFIG_SYS_NAND_ECCBYTES)
static struct nand_ecclayout nand_ecclayout = {
	.eccbytes = ECCBYTES_ONE_PAGE,
	.eccpos = CONFIG_SYS_NAND_ECCPOS,
	.oobavail = CONFIG_SYS_NAND_OOBSIZE - ECCBYTES_ONE_PAGE - 2,
	.oobfree = {
		{ .offset = 2, .length = CONFIG_SYS_NAND_OOBSIZE - 2 - ECCBYTES_ONE_PAGE },
	},
};
#endif


int mtd_nand_init(struct nand_chip *nand_chip)
{
#ifndef CONFIG_SPL_BUILD
	nand_timing_param timing;
#endif
#ifdef CFG_NAND_BW8
	int buswidth = 8;
#else
	int buswidth = 16;
#endif

	/*nand_chip init*/
	nand_chip->cmd_ctrl		= jz_nand_cmd_ctrl;
	nand_chip->select_chip		= jz_nand_select_chip;
	nand_chip->chip_delay		= 50;
	nand_chip->dev_ready		= jz_nand_device_ready;
#ifdef CONFIG_MTD_NAND_JZ_PN
	nand_chip->read_buf		= jz_nand_read_buf;
	nand_chip->priv			= &nand_privs[0];
	nand_privs[0].pn_bytes		= 0;
	nand_privs[0].pn_size		= 0;
	nand_privs[0].pn_skip		= 0;
	nand_chip->write_buf		= jz_nand_write_buf;
#elif defined(CONFIG_SPL_BUILD)
	nand_chip->read_buf		= nand_read_buf;
#endif
	nand_chip->bbt_options		= NAND_BBT_USE_FLASH;
	nand_chip->options		= NAND_NO_SUBPAGE_WRITE;
	nand_chip->ecc.mode		= NAND_ECC_HW_OOB_FIRST;
	nand_chip->ecc.hwctl		= jz_nand_hwctl;
	nand_chip->ecc.correct		= jz_nand_correct_data;
	nand_chip->ecc.calculate	= jz_nand_calculate_ecc;
	nand_chip->ecc.size		= CONFIG_SYS_NAND_ECCSIZE;
	nand_chip->ecc.bytes		= CONFIG_SYS_NAND_ECCBYTES;
	nand_chip->ecc.strength		= CONFIG_SYS_NAND_ECCSTRENGTH;
#ifdef CONFIG_MTD_NAND_BITFLIP_THRESHOLD
	mtd->bitflip_threshold = CONFIG_MTD_NAND_BITFLIP_THRESHOLD;
#else
	mtd->bitflip_threshold = nand_chip->ecc.strength / 3;
#endif

#ifndef CONFIG_SPL_BUILD
	nand_chip->ecc.layout		= &nand_ecclayout;
	timing.tALS  = NAND_ALS;
	timing.tALH  = NAND_ALH;
	timing.tRP   = NAND_RP;
	timing.tWP   = NAND_WP;
	timing.tRHW  = NAND_RHW;
	timing.tWHR  = NAND_WHR;
	timing.tWHR2 = NAND_WHR2;
	timing.tRR   = NAND_RR;
	timing.tWB   = NAND_WB;
	timing.tADL  = NAND_ADL;
	timing.tCWAW = NAND_CWAW;
	timing.tCS   = NAND_CS;
	timing.tCLH  = NAND_CLH;
	timing.tWH   = NAND_WH;
	timing.tCH   = NAND_CH;
	timing.tDH   = NAND_DH;
	timing.tREH   = NAND_REH;
	return jz_nand_init(nand_chip, &timing, buswidth);
#else
	return jz_nand_init(nand_chip, NULL, buswidth);
#endif
}
#endif /* !CONFIG_MTD_NAND_JZ_AUTO_PARAMS */
