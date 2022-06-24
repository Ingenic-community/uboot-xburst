/*
 * X1830 burner pll configuration
 *
 * Copyright (c) 2013 Ingenic Semiconductor Co.,Ltd
 * Author: cli <cli@ingenic.cn>
 *
 * Note: Burner's firmware is too big , for reduce code.
 *	 Burner fix cpu freq and ddr freq code.
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

/* #define DEBUG */
#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <asm/arch/cpm.h>
#include <asm/arch/clk.h>

DECLARE_GLOBAL_DATA_PTR;

void pll_init(void)
{
        unsigned int cpccr = (CONFIG_SYS_CPCCR_SEL & (~(0xf << 28))) | (1 << 30) | (2 << 28);
        unsigned int tmp;

        cpm_outl(CONFIG_SYS_MPLL_MNOD | (0x1 << 0), CPM_CPMPCR);
        while(!(cpm_inl(CPM_CPMPCR) & (0x1 << 3)));

	tmp = (cpm_inl(CPM_CPCCR) & (0xff << 24))
		| (cpccr & ~(0xff << 24))
		| (7 << 20);
	cpm_outl(tmp, CPM_CPCCR);
	while(cpm_inl(CPM_CPCSR) & 0x7);
	tmp = (cpccr & (0xff << 24)) | (cpm_inl(CPM_CPCCR) & ~(0xff << 24));
	cpm_outl(tmp,CPM_CPCCR);

        cpm_outl(CONFIG_SYS_APLL_MNOD | (0x1 << 0), CPM_CPAPCR);
        while(!(cpm_inl(CPM_CPAPCR) & (0x1 << 3)));

	cpccr = (cpccr & (0xff << 24)) | (cpm_inl(CPM_CPCCR) & ~(0xff << 24));
	cpm_outl(cpccr, CPM_CPCCR);

	printf("cpapcr %x\n", cpm_inl(CPM_CPAPCR));
	printf("cpmpcr %x\n", cpm_inl(CPM_CPMPCR));
	printf("cpccr:%x\n", cpm_inl(CPM_CPCCR));
}
