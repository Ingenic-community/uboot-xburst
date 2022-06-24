/*
 * X1830 pll configuration
 *
 * Copyright (c) 2017 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
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

/*#define DEBUG*/
#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <asm/errno.h>
#include <asm/arch/cpm.h>
#include <asm/arch/clk.h>

DECLARE_GLOBAL_DATA_PTR;

#undef assert
#define assert(con) if (!(con)) { printf("error %s %d",__func__, __LINE__); while(1); }

struct pll_cfg {
	unsigned apll_freq;
	unsigned mpll_freq;
	unsigned vpll_freq;
	unsigned epll_freq;
	unsigned cdiv;
	unsigned l2div;
	unsigned h0div;
	unsigned h2div;
	unsigned pdiv;
} pll_cfg;

#ifndef CONFIG_SYS_CPCCR_SEL
#define SEL_SRC		(0X2)
#define SEL_CPLL	((CONFIG_CPU_SEL_PLL == APLL) ? 0x1 : 0x2)
#define SEL_H0CLK	((CONFIG_DDR_SEL_PLL == APLL) ? 0x1 : 0x2)
#define SEL_H2CLK	SEL_H0CLK
#define CPCCR_CFG      \
	(((SEL_SRC& 3) << 30)                \
	 | ((SEL_CPLL & 3) << 28)                \
	 | ((SEL_H0CLK & 3) << 26)                 \
	 | ((SEL_H2CLK & 3) << 24)                 \
	 | (((pll_cfg.pdiv - 1) & 0xf) << 16)       \
	 | (((pll_cfg.h2div - 1) & 0xf) << 12)         \
	 | (((pll_cfg.h0div - 1) & 0xf) << 8)          \
	 | (((pll_cfg.l2div - 1) & 0xf) << 4)          \
	 | (((pll_cfg.cdiv - 1) & 0xf) << 0))
#else
#define CPCCR_CFG CONFIG_SYS_CPCCR_SEL
#endif

/*
 *  cclk = 1,2,3 or 4 times l2clk
 *  h2clk = 1,2 times pclk
 */
static void fill_pll_cfg(void)
{
#ifndef CONFIG_SYS_CPCCR_SEL
	unsigned cpupll, ddrpll;

#endif
	assert((gd->arch.gi->ddrfreq && gd->arch.gi->cpufreq));
	pll_cfg.apll_freq = CONFIG_SYS_APLL_FREQ > 0 ? CONFIG_SYS_APLL_FREQ : 0;
	pll_cfg.mpll_freq = CONFIG_SYS_MPLL_FREQ > 0 ? CONFIG_SYS_MPLL_FREQ : 0;
	pll_cfg.vpll_freq = CONFIG_SYS_VPLL_FREQ > 0 ? CONFIG_SYS_VPLL_FREQ : 0;
	pll_cfg.epll_freq = CONFIG_SYS_EPLL_FREQ > 0 ? CONFIG_SYS_EPLL_FREQ : 0;

#ifndef CONFIG_SYS_CPCCR_SEL
	cpupll = (CONFIG_CPU_SEL_PLL == APLL) ? pll_cfg.apll_freq : pll_cfg.mpll_freq;
	ddrpll = (CONFIG_DDR_SEL_PLL == APLL) ? pll_cfg.apll_freq : pll_cfg.mpll_freq;
	gd->arch.gi->ddr_div = ddrpll/gd->arch.gi->ddrfreq;
	pll_cfg.cdiv = cpupll/gd->arch.gi->cpufreq;
	pll_cfg.l2div = 2;
	pll_cfg.pdiv = 10;
	pll_cfg.h0div = 5;
	pll_cfg.h2div = 5;
#else
	if (CONFIG_DDR_SEL_PLL == APLL)
		gd->arch.gi->ddr_div = pll_cfg.apll_freq/gd->arch.gi->ddrfreq;
	else
		gd->arch.gi->ddr_div = pll_cfg.mpll_freq/gd->arch.gi->ddrfreq;
	pll_cfg.pdiv = (((CONFIG_SYS_CPCCR_SEL) >> 16) & 0xf) + 1;
	pll_cfg.h2div = (((CONFIG_SYS_CPCCR_SEL) >> 12) & 0xf) + 1;
	pll_cfg.h0div = (((CONFIG_SYS_CPCCR_SEL) >> 8) & 0xf) + 1;
	pll_cfg.l2div = (((CONFIG_SYS_CPCCR_SEL) >> 4) & 0xf) + 1;
	pll_cfg.cdiv = (((CONFIG_SYS_CPCCR_SEL) >> 0) & 0xf) + 1;
#endif
	printf("pdiv = %d, h2div = %d, h0div = %d, cdiv = %d, l2div = %d, ddrdiv = %d\n",
			pll_cfg.pdiv,
			pll_cfg.h2div,
			pll_cfg.h0div,
			pll_cfg.cdiv,
			pll_cfg.l2div,
			gd->arch.gi->ddr_div);
	return;
}

/*
 * freq = extal	[5M,200M]
 * fpfd = fref / (plln + 1) [5M,200M]
 * fvco = 2 * fpfd * (pllm + 1) [1500M,3000M]
 * pllout = fvco / (2^pllod) [25M,3000M]
 * pllm [0, 511]
 * plln [0, 63]
 * pllod [0, 6]
 */
static unsigned int get_pllreg_value(int freq)
{
	unsigned int pllm = 512,plln = 64,pllod,range;
	unsigned int pllout,fvco,fpfd,fref;
	unsigned int plln_tmp, pllm_tmp, fvco100, fpfd100;
	int error = 0, error_tmp;
	cpm_cpxpcr_t cppcr;

	/*5MHZ <= EXTAL <= 200MHZ*/
	fref = gd->arch.gi->extal / 1000000;
	assert((fref >= 5 && fref <= 200));

	/*25MHZ <= PLLOUT <= 3000MHZ*/
	pllout = freq / 1000000;
	assert((pllout >= 25 && pllout <= 3000));

	/*1500MHZ <= FVCO = PLLOUT * (2 ^ OD) <= 3000MHZ*/
	fvco = pllout;
	pllod = 0;
	while (fvco < 1500) {
		fvco <<= 1;
		pllod += 1;
	}

	fvco100 = fvco * 100; /*frac 0.01*/
	for (plln_tmp = 0; plln_tmp < 64 && pllod < 7; plln_tmp++) {
		fpfd100 = (fref * 100) / (plln_tmp + 1);
		/*5MHZ <= FPFD <= 200MHZ*/
		if (fpfd100 < 500)
			break;
		if (fpfd100 > 20000)
			continue;
		pllm_tmp = (fvco * (plln_tmp + 1) / (2 * fref)) - 1;
		if (pllm_tmp > 511)
			continue;
		error_tmp =  (int)fvco100 - (int)(fpfd100 * (pllm_tmp + 1) * 2);
		if (error_tmp < 0)
			error_tmp = -error_tmp;
		if (!error_tmp) {
			pllm = pllm_tmp;
			plln = plln_tmp;
			break;
		}
		if (error_tmp < error || !error /*first time*/) {
			pllm = pllm_tmp;
			plln = plln_tmp;
			error = error_tmp;
		}
	}
	assert(!(pllm > 511 || plln > 63 || pllod > 6));

	fpfd = fref/(plln + 1);
	if (fpfd < 5 || fpfd > 200)
		return 0;
	else if (fpfd <= 10)
		range = 1;
	else if (fpfd <= 16)
		range = 2;
	else if (fpfd <= 26)
		range = 3;
	else if (fpfd <= 42)
		range = 4;
	else if (fpfd <= 68)
		range = 5;
	else if (fpfd <= 110)
		range = 6;
	else
		range = 7;

	cppcr.b.PLLM = pllm;
	cppcr.b.PLLN = plln;
	cppcr.b.PLLOD = pllod;
	cppcr.b.PLLRG = range;
	printf("pllm = %d,plln = %d,pllod = %d,range = %d\n",pllm,plln,pllod,range);
	printf("cppcr is %x\n",cppcr.d32);
	return cppcr.d32;
}

/*********set CPXPCR register************/
static void pll_set(int pll,int freq)
{
	unsigned int cppcr = 0;
	unsigned int cpxpcr = 0;

	switch (pll) {
	case APLL:
		cpxpcr = CPM_CPAPCR;
#ifdef CONFIG_SYS_APLL_MNOD
		cppcr = CONFIG_SYS_APLL_MNOD;
#endif
		break;
	case MPLL:
		cpxpcr = CPM_CPMPCR;
#ifdef CONFIG_SYS_MPLL_MNOD
		cppcr = CONFIG_SYS_MPLL_MNOD;
#endif
		break;
	case VPLL:
		cpxpcr = CPM_CPVPCR;
#ifdef CONFIG_SYS_VPLL_MNOD
		cppcr = CONFIG_SYS_VPLL_MNOD;
#endif
		break;
	case EPLL:
		cpxpcr = CPM_CPEPCR;
#ifdef CONFIG_SYS_EPLL_MNOD
		cppcr = CONFIG_SYS_EPLL_MNOD;
#endif
		break;
	default:
		return;
	}

	if (!freq) {	/*0 means close pll*/
		cppcr = cpm_inl(cpxpcr);
		cpm_outl(cppcr & (~(0x1 << 0)), cpxpcr);
		while((cpm_inl(cpxpcr) & (0x1 << 3)));
	} else {
		if (!cppcr)
			cppcr = get_pllreg_value(freq);
		cpm_outl(cppcr | (0x1 << 0), cpxpcr);
		while(!(cpm_inl(cpxpcr) & (0x1 << 3)));
	}
	debug("cpxpcr(%x)0x%x\n", cpxpcr, cpm_inl(cpxpcr));
}

/*
 *bit 20 :22  使能分频值的写功能
 *
 * */
static void cpccr_init(void)
{
	unsigned int cpccr;

	/* change div 改变低24位 改变 分频值 */
	cpccr = (cpm_inl(CPM_CPCCR) & (0xff << 24))
		| (CPCCR_CFG & ~(0xff << 24))
		| (7 << 20);
	cpm_outl(cpccr,CPM_CPCCR);
	while(cpm_inl(CPM_CPCSR) & 0x7);

	/* change sel 改变高8位 选择时钟源 */
	cpccr = (CPCCR_CFG & (0xff << 24)) | (cpm_inl(CPM_CPCCR) & ~(0xff << 24));
	cpm_outl(cpccr,CPM_CPCCR);
	while(((cpm_inl(CPM_CPCSR) >> 28) & 0xf) != 0xf);
	debug("cppcr 0x%x\n",cpm_inl(CPM_CPCCR));
}

int pll_init(void)
{
	fill_pll_cfg();
	pll_set(APLL,pll_cfg.apll_freq);
	pll_set(MPLL,pll_cfg.mpll_freq);
	pll_set(VPLL,pll_cfg.vpll_freq);
	pll_set(EPLL,pll_cfg.epll_freq);
	cpccr_init();
	{
		unsigned apll,mpll,vpll,epll,cclk,l2clk,h0clk,h2clk,pclk;
		unsigned cpccr = cpm_inl(CPM_CPCCR);
		apll = clk_get_rate(APLL);
		mpll = clk_get_rate(MPLL);
		vpll = clk_get_rate(VPLL);
		epll = clk_get_rate(EPLL);
		printf("apll %d \nmpll %d \nvpll = %d\nepll = %d\n",(int)apll,(int)mpll,(int)vpll,(int)epll);

		switch ((cpccr >> 28) & 0x3) {
		case 0x1:
			cclk = apll / pll_cfg.cdiv;
			l2clk = apll / pll_cfg.l2div;
			break;
		case 0x2:
			cclk = mpll / pll_cfg.cdiv;
			l2clk = mpll / pll_cfg.l2div;
			break;
		default:
			cclk = 0; l2clk = 0;
		}
		switch ((cpccr >> 26) & 0x3) {
		case 0x1:
			h0clk = apll / pll_cfg.h0div;
			break;
		case 0x2:
			h0clk = mpll / pll_cfg.h0div;
			break;
		default:
			h0clk = 0;
		}
		switch ((cpccr >> 24) & 0x3) {
		case 0x1:
			h2clk = apll / pll_cfg.h2div;
			pclk = apll / pll_cfg.pdiv;
			break;
		case 0x2:
			h2clk = mpll / pll_cfg.h2div;
			pclk = mpll / pll_cfg.pdiv;
			break;
		default:
			h2clk = 0; pclk = 0;
		}
		if (CONFIG_CPU_SEL_PLL == APLL) {
			assert(!(cpccr & (1 << 23)));
			assert(((cpccr >> 30) & 0x3) == 0x2);
			assert(((cpccr >> 28) & 0x3) == 0x1);
		} else {
			assert(((cpccr >> 28) & 0x3) == 0x2);
		}
		if (CONFIG_DDR_SEL_PLL == APLL) {
			assert(!(cpccr & (1 << 23)));
			assert(((cpccr >> 30) & 0x3) == 0x2);
			assert(((cpccr >> 26) & 0x3) == 0x1);
			assert(((cpccr >> 24) & 0x3) == 0x1);
		} else {
			assert(((cpccr >> 26) & 0x3) == 0x2);
			assert(((cpccr >> 24) & 0x3) == 0x2);
		}
		printf("ddr sel %s, cpu sel %s cpccr(%x)\n", CONFIG_DDR_SEL_PLL == APLL ? "apll" : "mpll",
				CONFIG_CPU_SEL_PLL == APLL ? "apll" : "mpll", cpccr);
		printf("cclk  %d\nl2clk %d\nh0clk %d\nh2clk %d\npclk  %d\n",
				cclk,l2clk,h0clk,h2clk,pclk);
	}
	return 0;
}
