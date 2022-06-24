#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <asm/errno.h>
#include <asm/arch/cpm.h>
#include <asm/arch/clk.h>
#include <generated/pll_reg_values.h>

#define CONFIG_PLL_STABILIZE   0x20
static void pll_sets(void)
{
	unsigned int val;
	unsigned int timeout = 0;

	val = cpm_readl(CPM_CPAPCR);
	val &= ~(1 << 8);
	cpm_writel(val,CPM_CPAPCR);

	val = cpm_readl(CPM_CPMPCR);
	val &= ~(1 << 7);
	cpm_writel(val,CPM_CPMPCR);

	if(APLL_EN_VALUE){
		val = (APLL_EN_VALUE << 8)  | (APLL_M_VALUE << 24) | (APLL_N_VALUE << 18) |
			(APLL_OD_VALUE << 16) | (APLL_BS_VALUE << 31) | CONFIG_PLL_STABILIZE;
		cpm_writel(val,CPM_CPAPCR);
	}
	if(MPLL_EN_VALUE) {
		val = (MPLL_EN_VALUE << 7)  | (MPLL_M_VALUE << 24) | (MPLL_N_VALUE << 18) |
			(MPLL_OD_VALUE << 16) | (MPLL_BS_VALUE << 31);
		cpm_writel(val,CPM_CPMPCR);
	}
	if(APLL_EN_VALUE){
		timeout = 0x10000;
		while((!(cpm_readl(CPM_CPAPCR) & (1 << 10))) && --timeout);
		if(timeout == 0) {
			debug("apll[0x%x] setting timeout!\n",val);
		}
		printf("CPM_CPAPCR:%x\n",cpm_readl(CPM_CPAPCR));
	}
	if(MPLL_EN_VALUE){
		timeout = 0x10000;
		while((!(cpm_readl(CPM_CPMPCR) & 1)) && --timeout);
		if(timeout == 0) {
			debug("mpll[0x%x] setting timeout!\n",val);
		}
		printf("CPM_CPMPCR:%x\n",cpm_readl(CPM_CPMPCR));
	}
}
static void cpccr_default(void)
{
	cpm_writel(0x55700000,CPM_CPCCR);
	while((cpm_readl(CPM_CPCSR) & 0xf0000007) != 0xf0000000);
}
static void cpccr_sets(void)
{
	unsigned int val;
	val = 0x55700000 |
		(CDIV_REG_VALUE  <<  0) |
		(L2DIV_REG_VALUE <<  4) |
		(H0DIV_REG_VALUE <<  8) |
		(H2DIV_REG_VALUE << 12) |
		(PDIV_REG_VALUE <<  16);
	cpm_writel(val,CPM_CPCCR);
	while(cpm_readl(CPM_CPCSR) & 7);
	val = (SEL_SRC_REG_VALUE   << 30) |
		(SEL_CPLL_REG_VALUE  << 28) |
		(SEL_H0PLL_REG_VALUE << 26) |
		(SEL_H2PLL_REG_VALUE << 24) |
		(CDIV_REG_VALUE  <<  0) |
		(L2DIV_REG_VALUE <<  4) |
		(H0DIV_REG_VALUE <<  8) |
		(H2DIV_REG_VALUE << 12) |
		(PDIV_REG_VALUE <<  16);
	cpm_writel(val,CPM_CPCCR);
	while(!(cpm_readl(CPM_CPCSR) & 0xf0000000));
	printf("CPM_CPCCR:%x\n",cpm_readl(CPM_CPCCR));
}
int pll_init(void)
{
	cpccr_default();
	pll_sets();
	cpccr_sets();
}
