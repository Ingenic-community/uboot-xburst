/*
 * x1000 clock common interface
 *
 * Copyright (C) 2013 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
 * Based on: newxboot/modules/clk/jz4775_clk.c
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
#include <asm/gpio.h>
#include <asm/arch/cpm.h>
#include <asm/arch/clk.h>
#include <generated/clk_reg_values.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef DUMP_CGU_SELECT
static char clk_name[][10] = {
	[LCD] = {"lcd"},
	[SFC] = {"sfc"},
	[CIM] = {"cim"},
	[PWM] = {"pwm"},
	[DDR] = {"ddr"},
	[MSC0] = {"msc0"},
	[MSC1] = {"msc1"},
	[I2S0] = {"I2S0"},
	[I2S1] = {"I2S1"},
	[I2S2] = {"I2S2"},
	[I2S3] = {"I2S3"},
	[MACPHY] = {"macphy"},

};

static char * cgu_name(int clk) {
	return clk_name[clk];
}
#endif

struct clk_cgu_setting cgusetting[] = CGU_REG_VALUE;
void clk_prepare(void)
{
	/*stop clk and set div max*/
	int i;
	unsigned regval = 0, reg = 0;
	unsigned int size = ARRAY_SIZE(cgusetting);

	for (i = 0; i < size; i++) {
		reg = cgusetting[i].addr;
		regval = readl(reg);
		if(cgusetting[i].busy) {
			/*set div max*/
			regval |= cgusetting[i].val;
			writel(regval, reg);
			while (readl(reg) & (1 << cgusetting[i].busy));
		} else {
			regval &= ~(1 << cgusetting[i].ce);
			writel(regval, reg);
		}
#ifdef DUMP_CGU_SELECT
		printf("%s(0x%x) :0x%x\n",clk_name[i] ,reg,  readl(reg));
#endif
	}
}
static inline void cgu_clks_set(void)
{
	int i;
	unsigned regval = 0, reg = 0;
	unsigned int size = ARRAY_SIZE(cgusetting);

	for (i = 0; i < size; i++) {
		reg = cgusetting[i].addr;
		regval = readl(reg);
		regval &= ~(3 << 30);
		regval |= cgusetting[i].sel_val;
		writel(regval, reg);
	}
}
static unsigned int pll_get_rate(int pll)
{
	unsigned int cpxpcr = 0;
	unsigned int m, n, od;
	unsigned int rate;

	switch (pll) {
	case APLL:
		cpxpcr = cpm_inl(CPM_CPAPCR);
		break;
	case MPLL:
		cpxpcr = cpm_inl(CPM_CPMPCR);
		break;
	case EPLL:
		cpxpcr = cpm_inl(CPM_CPEPCR);
		break;
	default:
		return 0;
	}
	m = ((cpxpcr >> 20) & 0xfff) + 1;
	n = ((cpxpcr >> 14) & 0x3f) + 1;
	od = (cpxpcr >> 11) & 0x7;
	od = 1 << od;
#ifdef CONFIG_BURNER
	rate = CONFIG_SYS_EXTAL / 1000 / 1000;//gd->arch.gi->extal / 1000 / 1000;
#else
	rate = CONFIG_SYS_EXTAL / 1000 / 1000;
#endif
	rate = rate * m  * 2/ n / od;
	return (rate * 1000 * 1000);
}

static unsigned int get_ddr_rate(void)
{
	unsigned int ddrcdr  = cpm_inl(CPM_DDRCDR);

	switch ((ddrcdr >> 30) & 3) {
	case 1:
		return pll_get_rate(APLL) / ((ddrcdr & 0xf) + 1);
	case 2:
		return pll_get_rate(MPLL) / ((ddrcdr & 0xf) + 1);
	}
	return 0;
}

static unsigned int get_cclk_rate(void)
{
	unsigned int cpccr  = cpm_inl(CPM_CPCCR);

	switch ((cpccr >> 28) & 3) {
	case 1:
		return pll_get_rate(APLL) / ((cpccr & 0xf) + 1);
	case 2:
		return pll_get_rate(MPLL) / ((cpccr & 0xf) + 1);
	}
	return 0;
}
#if defined(CONFIG_JZ_MMC_MSC0) || defined(CONFIG_JZ_MMC_MSC1)
static unsigned int get_msc_rate(unsigned int xcdr)
{
	unsigned int msc0cdr  = cpm_inl(CPM_MSC0CDR);
	unsigned int mscxcdr  = cpm_inl(xcdr);
	unsigned int ret = 1;

	switch (msc0cdr >> 30) {
	case 0:
		ret = pll_get_rate(APLL) / (((mscxcdr & 0xff) + 1) * 4);
		break;
	case 1:
		ret = pll_get_rate(MPLL) / (((mscxcdr & 0xff) + 1) * 4);
		break;
	default:
		break;
	}

	return ret;
}
#endif

unsigned int cpm_get_h2clk(void)
{
	int h2clk_div;
	unsigned int cpccr  = cpm_inl(CPM_CPCCR);

	h2clk_div = (cpccr >> 12) & 0xf;

	switch ((cpccr >> 24) & 3) {
		case 1:
			return pll_get_rate(APLL) / (h2clk_div + 1);
		case 2:
			return pll_get_rate(MPLL) / (h2clk_div + 1);
	}
	return 0;
}

unsigned int clk_get_rate(int clk)
{
	switch (clk) {
	case APLL:
		return pll_get_rate(APLL);
	case MPLL:
		return pll_get_rate(MPLL);
	case DDR:
		return get_ddr_rate();
	case CPU:
		return get_cclk_rate();
	case H2CLK:
		return cpm_get_h2clk();
#ifdef CONFIG_JZ_MMC_MSC0
	case MSC0:
		return get_msc_rate(CPM_MSC0CDR);
#endif
#ifdef CONFIG_JZ_MMC_MSC1
	case MSC1:
		return get_msc_rate(CPM_MSC1CDR);
#endif
	}

	return 0;
}

void clk_set_rate(int clk_id, unsigned long rate)
{
	unsigned int cdr;
	unsigned int pll_rate;
	struct clk_cgu_setting *cgu = NULL;
	unsigned regval = 0, reg = 0;

	if(clk_id >= CGU_CNT) {
		/* printf("set clk id error\n"); */
		return;
	}

	cgu = &cgusetting[clk_id];
	reg = cgu->addr;
	pll_rate = pll_get_rate(cgu->sel_src);

	if(!pll_rate) {
		debug("clk id %d: get pll error\n", clk_id);
		return;
	}

	if(clk_id == MSC0 || clk_id == MSC1)
		cdr = (((pll_rate + rate - 1)/rate)/4 - 1)& 0xff;
	else
		cdr = ((pll_rate + rate - 1)/rate - 1 ) & 0xff;
	/* debug("pll_rate = %d, rate = %d, cdr = %d\n",pll_rate,rate,cdr); */

	regval = readl(reg);
	regval &= ~(3 << cgu->stop | 0xff);
	regval |= ((1 << cgu->ce) | cdr);
	writel(regval, reg);
	while (readl(reg) & (1 << cgu->busy))
		;
#ifdef DUMP_CGU_SELECT
	printf("%s(0x%x) :0x%x\n",clk_name[clk_id] ,reg,  readl(reg));
#endif
	return;
}
void clk_init(void)
{
	unsigned int reg_clkgr = cpm_inl(CPM_CLKGR);
	unsigned int gate = CPM_CLKGR_DDR
#ifdef CONFIG_JZ_MMC_MSC0
		| CPM_CLKGR_MSC0
#endif
#ifdef CONFIG_JZ_MMC_MSC1
		| CPM_CLKGR_MSC1
#endif
#ifdef CONFIG_JZ_LCD_V14
		| CPM_CLKGR_LCD
#endif
#ifdef CONFIG_JZ_SFC
		| CPM_CLKGR_SFC
#endif
#ifdef CONFIG_JZ_SPI0
		| CPM_CLKGR_SSI0
#endif
#ifdef CONFIG_JZ_SPI1
		| CPM_CLKGR_SSI1
#endif
#ifdef CONFIG_NET_GMAC
		| CPM_CLKGR_MAC
#endif
		;

	reg_clkgr &=  ~gate;
	cpm_outl(reg_clkgr,CPM_CLKGR);
	cgu_clks_set();
}

void enable_uart_clk(void)
{
	unsigned int clkgr = cpm_inl(CPM_CLKGR);

	switch (gd->arch.gi->uart_idx) {
#define _CASE(U, N) case U: clkgr &= ~N; break
		_CASE(0, CPM_CLKGR_UART0);
		_CASE(1, CPM_CLKGR_UART1);
		_CASE(2, CPM_CLKGR_UART2);
	default:
		break;
	}
	cpm_outl(clkgr, CPM_CLKGR);
}

void otg_phy_init(enum otg_mode_t mode, unsigned extclk) {
#ifndef CONFIG_SPL_BUILD
	printf("ERR, usb clk need rewriting!!\n");
#if 0
	int ext_sel = 0;
	int tmp_reg = 0;
	int timeout = 0x7fffff;

	tmp_reg = cpm_inl(CPM_USBPCR1);
	tmp_reg &= ~(USBPCR1_REFCLKSEL_MSK | USBPCR1_REFCLKDIV_MSK);
	tmp_reg |= USBPCR1_REFCLKSEL_CORE | USBPCR1_WORD_IF_16_30;
	switch (extclk/1000000) {
	case 12:
		tmp_reg |= USBPCR1_REFCLKDIV_12M;
		break;
	case 19:
		tmp_reg |= USBPCR1_REFCLKDIV_19_2M;
		break;
	case 48:
		tmp_reg |= USBPCR1_REFCLKDIV_48M;
		break;
	default:
		ext_sel = 1;
	case 24:
		tmp_reg |= USBPCR1_REFCLKDIV_24M;
		break;
	}
	cpm_outl(tmp_reg,CPM_USBPCR1);

	/*set usb cdr clk*/
	tmp_reg = cpm_inl(CPM_USBCDR);
	tmp_reg &= ~USBCDR_UCS_PLL;
	cpm_outl(tmp_reg, CPM_USBCDR);
	if (ext_sel) {
		unsigned int pll_rate = pll_get_rate(APLL);	//FIXME: default apll
		unsigned int cdr = pll_rate/24000000;
		cdr = cdr ? cdr - 1 : cdr;
		tmp_reg |= (cdr & USBCDR_USBCDR_MSK) | USBCDR_CE_USB;
		tmp_reg &= ~USBCDR_USB_STOP;
		cpm_outl(tmp_reg, CPM_USBCDR);
		while ((cpm_inl(CPM_USBCDR) & USBCDR_USB_BUSY) || timeout--);
		tmp_reg = cpm_inl(CPM_USBCDR);
		tmp_reg &= ~USBCDR_UPCS_MPLL;
		tmp_reg |= USBCDR_UCS_PLL;
		cpm_outl(tmp_reg, CPM_USBCDR);
	} else {
		tmp_reg |= USBCDR_USB_STOP;
		cpm_outl(tmp_reg, CPM_USBCDR);
		while ((cpm_inl(CPM_USBCDR) & USBCDR_USB_BUSY) || timeout--);
	}
	if (!timeout)
		printf("USBCDR wait busy bit failed\n");

	tmp_reg = cpm_inl(CPM_USBPCR);
	switch (mode) {
	case OTG_MODE:
	case HOST_ONLY_MODE:
		tmp_reg |= USBPCR_USB_MODE_ORG;
		tmp_reg &= ~(USBPCR_VBUSVLDEXTSEL|USBPCR_VBUSVLDEXT|USBPCR_OTG_DISABLE);
		break;
	case DEVICE_ONLY_MODE:
		tmp_reg &= ~USBPCR_USB_MODE_ORG;
		tmp_reg |= USBPCR_VBUSVLDEXTSEL|USBPCR_VBUSVLDEXT|USBPCR_OTG_DISABLE;
	}
	cpm_outl(tmp_reg, CPM_USBPCR);

	tmp_reg = cpm_inl(CPM_OPCR);
	tmp_reg |= OPCR_SPENDN;
	cpm_outl(tmp_reg, CPM_OPCR);

	tmp_reg = cpm_inl(CPM_USBPCR);
	tmp_reg |= USBPCR_POR;
	cpm_outl(tmp_reg, CPM_USBPCR);
	udelay(30);
	tmp_reg = cpm_inl(CPM_USBPCR);
	tmp_reg &= ~USBPCR_POR;
	cpm_outl(tmp_reg, CPM_USBPCR);
	udelay(300);

	tmp_reg = cpm_inl(CPM_CLKGR);
	tmp_reg &= ~CPM_CLKGR_OTG;
	cpm_outl(tmp_reg, CPM_CLKGR);
#endif
#endif
}

void print_clock()
{
/* #ifndef CONFIG_BURNER */
/* 	unsigned int apll = 0, mpll = 0; */
/* 	unsigned int cclk = 0, l2clk = 0; */
/* 	unsigned int h0clk = 0, h2clk = 0; */
/* 	unsigned int pclk = 0; */
/* 	unsigned int pll_tmp = 0, cpccr = 0; */
/* 	int chose_tmp, div, div1; */

/* 	/\* apll = pll_get_rate(APLL); *\/ */
/* 	/\* mpll = pll_get_rate(MPLL); *\/ */
/* 	apll = CONFIG_SYS_APLL_FREQ; */
/* 	mpll = CONFIG_SYS_MPLL_FREQ; */

/* 	cpccr = cpm_inl(CPM_CPCCR); */

/* 	chose_tmp=(cpccr  >> 28) & 3; */
/* 	div = (cpccr>>4) & 0xf + 1; */
/* 	if(chose_tmp==1){ */
/* 		l2clk=apll/div; */
/* 	}else if(chose_tmp==2){ */
/* 		l2clk=mpll/div; */
/* 	} */

/* 	chose_tmp=( cpccr  >> 26) & 3; */
/* 	div = (cpccr>>8) & 0xf + 1; */
/* 	if(chose_tmp==1){ */
/* 		h0clk=apll/div; */
/* 	}else if(chose_tmp==2){ */
/* 		h0clk=mpll/div; */
/* 	} */
/* 	chose_tmp=( cpccr  >> 24) & 3; */
/* 	div = (cpccr>>12) & 0xf + 1; */
/* 	div1 = (cpccr>>16) & 0xf + 1; */
/* 	if(chose_tmp==1){ */
/* 		h2clk=apll/div; */
/* 		pclk=apll/div1; */
/* 	}else if(chose_tmp==2){ */
/* 		h2clk=mpll/div; */
/* 		pclk=mpll/div1; */
/* 	} */
/* 	printf("apll = %d\n mpll = %d\n", apll, mpll); */

/* 	printf("ddrfreq = %d\n cpufreq = %d\n l2cache = %d\n",\ */
/* 	       gd->arch.gi->ddrfreq, gd->arch.gi->cpufreq, l2clk); */
/* 	printf("AHB0freq= %d\nAHB2freq= %d\npclk %d\n",h0clk,h2clk,pclk); */
/* #else */
	/* printf("apll = %d\n mpll = %d\n", pll_get_rate(APLL), pll_get_rate(MPLL)); */
	/* printf("cpccr = %x\n", cpm_inl(CPM_CPCCR)); */
/* #endif */
}
