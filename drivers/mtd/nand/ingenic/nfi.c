/*
 *  Copyright (C) 2013 Ingenic Semiconductor Co., LTD.
 *  Author: rhuang <rui.huang@ingenic.com>
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
#include <asm/arch/nand.h>
#include <asm/arch/clk.h>
#include <linux/mtd/nand.h>
#include "jz_nand.h"

extern unsigned int clk_get_rate(int clk);
static void nand_nfi_setup_nand_param(nand_timing_param  *param, int buswidth)
{
	unsigned int lbit,hbit;
	unsigned int h2clk = clk_get_rate(H2CLK);
	unsigned long cycle = 1000000000/(h2clk/1000);
	unsigned int reg;

	/* set BCH clock divide */
	clk_set_rate(BCH, h2clk);

	do {
		reg = readl(NAND_NFCR);
	} while (!(reg & NAND_NFCR_EMPTY));
	reg &= ~(NAND_NFCR_SEL_MASK | NAND_NFCR_BUSWIDTH_MASK);
	reg |=	NAND_NFCR_BUSWIDTH(buswidth == 16) | NAND_NFCR_SEL(COMMON_NAND);
	writel(reg, NAND_NFCR);

	if (!param) {
		writel(0x30009, NAND_NFIT0);
		writel(0x30003, NAND_NFIT1);
		writel(0x90003, NAND_NFIT2);
		writel(0xf000f, NAND_NFIT3);
		writel(0xff000f, NAND_NFIT4);
		return;
	}

	hbit = ((param->tWH - param->tDH) * 1000) / cycle;
	lbit = ((param->tWP) * 1000 + cycle - 1) / cycle;
	writel((NAND_NFIT0_SWE(hbit) | NAND_NFIT0_WWE(lbit)),NAND_NFIT0);

	hbit = ((param->tDH) * 1000) / cycle;
	lbit = ((param->tREH - param->tDH ) * 1000 + cycle - 1) / cycle;
	writel((NAND_NFIT1_HWE(hbit) | NAND_NFIT1_SRE(lbit)),NAND_NFIT1);

	hbit = ((param->tRP) * 1000 + cycle - 1) / cycle;
	lbit = ((param->tDH) * 1000) / cycle;
	writel((NAND_NFIT2_WRE(hbit) | NAND_NFIT2_HRE(lbit)),NAND_NFIT2);

	hbit = ((param->tCS) * 1000 + cycle - 1) / cycle;
	lbit = ((param->tCH) * 1000 + cycle - 1) / cycle;
	writel((NAND_NFIT3_SCS(hbit) | NAND_NFIT3_WCS(lbit)),NAND_NFIT3);

	hbit = ((param->tRR) * 1000 + cycle - 1) / cycle;
	lbit = (1 * 1000 + cycle - 1) / cycle ;
	writel((NAND_NFIT4_BUSY(hbit) | NAND_NFIT4_EDO(lbit)),NAND_NFIT4);
}

static void nand_nfi_chip_select(int select)	 /*on bootloader we just support one chip*/
{
	if (select != -1) {
		writel(0x0,NAND_NFCS);
		writel(0x1,NAND_NFCS);
	} else {
		writel(0x0,NAND_NFCS);
	}
}

static void nand_nfi_enable(void)
{
	unsigned int reg;
	reg = readl(NAND_NFCR);
	reg &= ~(NAND_NFCR_CSEN_MASK);
	reg |= NAND_NFCR_CSMOD | NAND_NFCR_INIT | NAND_NFCR_EN;
	writel(reg, NAND_NFCR);
	ndelay(100);
}

void jz_nand_enable_pn(int enable, bool force)
{
	uint32_t reg = readl(NAND_PNCR);

	if (!force && !((reg & NAND_PNCR_PNEN) ^ !!enable))
		return;
	if (enable)
		reg |= NAND_PNCR_PNEN | NAND_PNCR_PNRST;
	else
		reg &= ~NAND_PNCR_PNEN;
	writel(reg, NAND_PNCR);
}

static inline void nand_nfi_pn_init(struct nand_chip *nand)
{
	jz_nand_enable_pn(0, false);
}

void jz_nand_select_chip(struct mtd_info *mtd, int select)
{
	//nand_nfi_chip_select(select);		/*we just support select 0 (cs 1)*/
}

int jz_nand_device_ready(struct mtd_info *mtd)
{	
	ndelay(200);
	return !!(readl(GPIO_PXPIN(0)) & 0x00100000);	/*FIXME: PA20*/
}

void jz_nand_cmd_ctrl(struct mtd_info *mtd, int cmd, unsigned int ctrl)
{
	struct nand_chip *this = mtd->priv;

	if (ctrl & NAND_CTRL_CHANGE) {
		if (ctrl & NAND_ALE)
			this->IO_ADDR_W = (void __iomem *)(CONFIG_SYS_NAND_BASE + OFFSET_ADDR);
		else if (ctrl & NAND_CLE)
			this->IO_ADDR_W = (void __iomem *)(CONFIG_SYS_NAND_BASE + OFFSET_CMD);
		else
			this->IO_ADDR_W = (void __iomem *)(CONFIG_SYS_NAND_BASE + OFFSET_DATA);
	}
	if (cmd != NAND_CMD_NONE)
		writeb(cmd, this->IO_ADDR_W);
}

int jz_get_nand_id(void)
{
	u32 timeo = (CONFIG_SYS_HZ * 20) / 1000;
	u32 time_start;
	unsigned int maf_id = 0,dev_id = 0;

	nand_nfi_enable();
	nand_nfi_setup_nand_param(NULL, 8);
	nand_nfi_chip_select(0);

	/*reset*/
	writeb(NAND_CMD_RESET, CONFIG_SYS_NAND_BASE + OFFSET_CMD);
	udelay(20);
	while(!(readl(GPIO_PXPIN(0)) & 0x00100000));

	/*read id*/
	writeb(NAND_CMD_READID, (CONFIG_SYS_NAND_BASE + OFFSET_CMD));
	writeb(0x0, (CONFIG_SYS_NAND_BASE + OFFSET_ADDR));
	udelay(20);
	while(!(readl(GPIO_PXPIN(0)) & 0x00100000));

	maf_id = readb(CONFIG_SYS_NAND_BASE + OFFSET_DATA);	/*maf_id*/
	dev_id = readb(CONFIG_SYS_NAND_BASE + OFFSET_DATA);	/*dev_id*/
	return (maf_id << 8 | dev_id);
}

int jz_nand_init(struct nand_chip *nand, nand_timing_param *param, int buswidth)
{
	/* initilize NFI controller */
	nand_nfi_enable();
	nand_nfi_setup_nand_param(param, buswidth);
	nand_nfi_chip_select(0);
	nand_nfi_pn_init(nand);
	nand->IO_ADDR_R = (void __iomem *)(CONFIG_SYS_NAND_BASE +  OFFSET_DATA);
	return 0;
}
