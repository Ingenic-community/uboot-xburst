
/*
 * JZ NAND (NEMC/BCH) driver
 *
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

void jz_nand_enable_pn(int enable, bool force)
{
	uint32_t reg = readl(NEMC_BASE + NEMC_PNCR);

	/* check whether PNEN should change */
	if (!force && !((reg & NEMC_PNCR_PNEN) ^ !!enable))
		return;

	/* ...it should, either enable & reset or disable */
	if (enable)
		reg |= NEMC_PNCR_PNEN | NEMC_PNCR_PNRST;
	else
		reg &= ~NEMC_PNCR_PNEN;

	writel(reg, NEMC_BASE + NEMC_PNCR);
}

int jz_nand_device_ready(struct mtd_info *mtd)
{
	ndelay(200);
	return !!(readl(GPIO_PXPIN(0)) & 0x00100000);
}

void jz_nand_select_chip(struct mtd_info *mtd, int select)
{
}

void jz_nand_cmd_ctrl(struct mtd_info *mtd, int cmd, unsigned int ctrl)
{
	struct nand_chip *this = mtd->priv;
	uint32_t reg;

	if (ctrl & NAND_CTRL_CHANGE) {
		if (ctrl & NAND_ALE)
			this->IO_ADDR_W = (void __iomem *)
				(CONFIG_SYS_NAND_BASE + OFFSET_ADDR);
		else if (ctrl & NAND_CLE)
			this->IO_ADDR_W = (void __iomem *)
					(CONFIG_SYS_NAND_BASE + OFFSET_CMD);
		else
			this->IO_ADDR_W = (void __iomem *)
				(CONFIG_SYS_NAND_BASE + OFFSET_DATA);

		reg = readl(NEMC_BASE + NEMC_NFCSR);
		if (ctrl & NAND_NCE)
			reg |= NEMC_NFCSR_FCEn(1);
		else
			reg &= ~NEMC_NFCSR_FCEn(1);
		writel(reg, NEMC_BASE + NEMC_NFCSR);
	}
	if (cmd != NAND_CMD_NONE)
		writeb(cmd, this->IO_ADDR_W);
}

int jz_get_nand_id(void)
{
	u32 timeo = (CONFIG_SYS_HZ * 20) / 1000;
	u32 time_start;
	int chip_id = 0;

	/*writel(0x18664400, NEMC_BASE + NEMC_SMCR1);*/

	writel(readl(NEMC_BASE + NEMC_NFCSR) | NEMC_NFCSR_NFEn(1), NEMC_BASE + NEMC_NFCSR);

	writeb(NAND_CMD_RESET,(CONFIG_SYS_NAND_BASE + OFFSET_CMD));

	ndelay(100);

	time_start = get_timer(0);
	while (get_timer(time_start) < timeo) {
		udelay(20);
		if (!!(readl(GPIO_PXPIN(0)) & 0x00100000))
			break;
	}

	writeb(NAND_CMD_READID,(CONFIG_SYS_NAND_BASE + OFFSET_CMD));

	writeb(0,(CONFIG_SYS_NAND_BASE + OFFSET_ADDR));

	chip_id = readb(CONFIG_SYS_NAND_BASE);	/*mef_id*/
	chip_id = (chip_id << 8) | readb(CONFIG_SYS_NAND_BASE);	/*dev_id*/
	return chip_id;
}

extern unsigned int clk_get_rate(int clk);
static int nand_set_optimize(nand_timing_param *timing)
{
	int valume, smcr = 0;
	int h2clk = clk_get_rate(H2CLK);
	int cycle = 1000000000/(h2clk/1000);
	printf("h2clk = %dHZ\n", h2clk);

	/* set BCH clock divide */
	clk_set_rate(BCH, h2clk);

	if (!timing)
		return 0;

	/* NEMC.TAS */
	valume = (timing->tALS * 1000 + cycle - 1) / cycle;
	/**
	 * here we reduce one cycle, because that,
	 * IC maybe designed as add one cycle at
	 * TAS & TAH, but here we can't set TAS & TAH
	 * as '0', because set '0' can cause bugs of
	 * bitcount, the bitcount may count less that
	 * the really count if '0' be set at TAS & TAH
	*/
	valume -= (valume > 1) ? 1 : 0;
	smcr |= (valume & NEMC_SMCR_TAS_MASK) << NEMC_SMCR_TAS_BIT;
	/* NEMC.TAH */
	valume = (timing->tALH * 1000 + cycle -1) / cycle;
	valume -= (valume > 1) ? 1 : 0;
	smcr |= (valume & NEMC_SMCR_TAH_MASK) << NEMC_SMCR_TAH_BIT;
	/* NEMC.TBP */
	valume = (timing->tWP * 1000 + cycle - 1) / cycle;
	smcr |= (valume & NEMC_SMCR_TBP_MASK) << NEMC_SMCR_TBP_BIT;
	/* NEMC.TAW */
	valume = (timing->tRP * 1000 + cycle -1) / cycle;
	smcr |= (valume & NEMC_SMCR_TAW_MASK) << NEMC_SMCR_TAW_BIT;
	/* NEMC.STRV */
	valume = (timing->tRHW * 1000 + cycle - 1) / cycle;
	smcr |= (valume & NEMC_SMCR_STRV_MASK) << NEMC_SMCR_STRV_BIT;

	printf("INFO: tals=%d talh=%d twp=%d trp=%d smcr=0x%08x\n"
                        , timing->tALS, timing->tALH, timing->tWP, timing->tRP, smcr);
	writel(smcr, NEMC_BASE + NEMC_SMCR1);
	return 0;
}

int jz_nand_init(struct nand_chip *nand, nand_timing_param *param, int buswidth)
{
	uint32_t smcr, reg;

	/* optimise timing */
	nand_set_optimize(param);

	/* enable flash chip */
	writel(readl(NEMC_BASE + NEMC_NFCSR) | NEMC_NFCSR_NFEn(1), NEMC_BASE + NEMC_NFCSR);

	/* begin without pseudo-random noise */
	jz_nand_enable_pn(0, false);

	nand->IO_ADDR_R = (void __iomem *)(CONFIG_SYS_NAND_BASE +  OFFSET_DATA);
	return 0;
}
