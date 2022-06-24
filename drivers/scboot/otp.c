#include <common.h>
#include <asm/io.h>
#include <asm/errno.h>
#include <asm/gpio.h>
#include "secall.h"
#include "pdma.h"
#include "aes.h"
#include "otp.h"
#include "test_nku.h"
#include <cloner/cloner.h>

unsigned int rsakey[128];
unsigned int rsakeylen;

static void set_rsakey(unsigned int *idata, unsigned int length)
{
	unsigned int iLoop;

	memset(rsakey, 0, sizeof(rsakey));
	for(iLoop = 0; iLoop < length / 4; iLoop++)
		rsakey[iLoop] = idata[iLoop];

	rsakeylen = length / 2;
}

int get_rsakeylen(void)
{
	return rsakeylen;
}

static void gpio_output_value(int gpio, int value)
{
	gpio_direction_output(gpio, value);
}


static int efuse_update_state(void)
{
	volatile unsigned int *reg_ctrl = (volatile unsigned int *)EFUSE_REG_CTRL;
	volatile unsigned int *reg_stat = (volatile unsigned int *)EFUSE_REG_STAT;
	*reg_ctrl = (EFUSE_ADDR_PROT << EFUSE_REGOFF_CRTL_ADDR) | (0x1 << EFUSE_REGOFF_CRTL_LENG) | EFUSE_REG_CTRL_RDEN;
	while(!(*reg_stat & EFUSE_REG_STAT_RDDONE));
	printf("xxxxxxx state updated: %x\n", *reg_stat);
}

static int efuse_config(void)
{
	/*efuse register*/
	volatile unsigned int *reg_ctrl = (volatile unsigned int *)EFUSE_REG_CTRL;
	volatile unsigned int *reg_cfg = (volatile unsigned int *)EFUSE_REG_CFG;
	volatile unsigned int *reg_stat = (volatile unsigned int *)EFUSE_REG_STAT;
	/*cpm register*/
	volatile unsigned int *reg_cpccr = (volatile unsigned int *)0xb0000000;
	volatile unsigned int *reg_cpapcr = (volatile unsigned int *)0xb0000010;
	volatile unsigned int *reg_cpmpcr = (volatile unsigned int *)0xb0000014;

//	printf("xxxxxxx reg_cfg = %x\n",*reg_cfg);
//	printf("xxxxxxx reg_cpccr = %x\n",*reg_cpccr);
//	printf("xxxxxxx reg_cpmpcr = %x\n",*reg_cpmpcr);
//	printf("xxxxxxx reg_stat: = %x\n", *reg_stat);

	int cfg = 0;
	int h2div = ((*reg_cpccr & 0xf<<12)>>12) + 1;
	int sel_a = 0;
	if(((*reg_cpccr >> 24) & 0x3) == 1) {
		sel_a = 1;
	} else if (((*reg_cpccr >> 24) & 0x3) == 2) {
		sel_a = 2;
	}

	int pll = 0;
	if(sel_a == 1) {
		int apll_m = ((*reg_cpapcr & 0x7f<<24)>>24) + 1;
		int apll_n = ((*reg_cpapcr & 0x1f<<18)>>18) + 1;
		int apll_o = ((*reg_cpapcr & 0x3<<16)>>16) + 1;

		pll = 24*apll_m/(apll_n * apll_o);
//		printf(" xxxx AHB2 select APLL : %d\n", pll);
	} else if(sel_a == 2) {
		int mpll_m = ((*reg_cpmpcr & 0x7f<<24)>>24) + 1;
		int mpll_n = ((*reg_cpmpcr & 0x1f<<18)>>18) + 1;
		int mpll_o = ((*reg_cpmpcr & 0x3<<16)>>16) + 1;

		pll = 24*mpll_m/(mpll_n*mpll_o);
//		printf(" xxxx AHB2 select MPLL : %d\n", pll);
	}

	int ahb2 = pll/h2div;

	int ahb2_cycle= 1000/ahb2; //ns

	int wr_adj = 0;
	int rd_adj = 0;
	while(1) {
		if((wr_adj + 1) * ahb2_cycle > 2) {
//			printf("-----wr_adj = %x --\n",wr_adj);
			break;
		}
		wr_adj ++;
		rd_adj ++;
	}

	int wr_strobe = 0;
	while(1) {
		if(((ahb2_cycle * (wr_adj+916 + wr_strobe)) > 4000) && ((ahb2_cycle * (wr_adj+916 + wr_strobe))< 6000)) {
//			printf("-----wr_strobe = %x --\n",wr_strobe);
			break;
		}
		if(ahb2_cycle * (wr_adj+916 + wr_strobe) > 6000) {
			printf("!!!!!!!!!!!! efuse can't run in bad AHB2 Frequency!!!!!!!\n");
			break;
		}
		wr_strobe++;
	}


	int rd_strobe = 0;
	while(1) {
		if(((rd_adj + 3 + rd_strobe) * ahb2_cycle) > 15) {
//			printf("-----rd_strobe = %x --\n",rd_strobe);
			break;
		}

		rd_strobe++;
	}

	//rd_adj = 100;
	//rd_strobe = 100;
	*reg_cfg = (rd_adj << 19) | (rd_strobe << 16) | (wr_adj<<12) | wr_strobe;
//	printf("xxxxxxx reg_cfg = %x\n",*reg_cfg);
//	printf("xxxxxxx mpll = %d\n",pll);
//	printf("xxxxxxx ahb2 = %d\n",ahb2);

}

void otp_init(void)
{
	gpio_output_value(debug_args->efuse_gpio, 1);
	efuse_config();
	efuse_update_state();
	*(volatile unsigned int *)(MCU_TCSM_RETVAL) = SC_ERR_SUCC;
}

int cpu_wtotp(int opera)
{
	unsigned int ret;
	unsigned int iLoop;
	volatile struct sc_args *args;
	args = (volatile struct sc_args *)GET_SC_ARGS();
	volatile unsigned int *reg_ctrl = (volatile unsigned int *)EFUSE_REG_CTRL;

//	printf("xxxxxxxxxxx func : %s\n",__func__);

	*reg_ctrl |= EFUSE_REG_CTRL_PGEN; /*pg en*/
	gpio_output_value(debug_args->efuse_gpio, 0);

	args->arg[0] = opera;
	ret = secall(args, SC_FUNC_WTOTP, 0, 1);

	gpio_output_value(debug_args->efuse_gpio, 1);
	*reg_ctrl &= ~EFUSE_REG_CTRL_PGEN;

	if (*(volatile unsigned int *)(MCU_TCSM_RETVAL) != SC_ERR_SUCC) {
		printf("write otp err, ret val %x\n",*(volatile unsigned int *)(MCU_TCSM_RETVAL));
		return -ESEC;
	}

	efuse_update_state();

	return 0;
}

int cpu_burn_rckey(void)
{
	unsigned int ret;
	volatile struct sc_args *args;
	args = (volatile struct sc_args *)GET_SC_ARGS();
	volatile unsigned int *reg_ctrl = (volatile unsigned int *)EFUSE_REG_CTRL;

//	printf("xxxxxxxxxxx func : %s\n",__func__);
	return 0;
	if(EFUSTATE_NKU_PRT)
		return 0;

	if(cpu_get_rn() < 0)
		return -ESEC;

	*reg_ctrl |= EFUSE_REG_CTRL_PGEN; /*pg en*/
	gpio_output_value(debug_args->efuse_gpio, 0);

	secall(args, SC_FUNC_BURNRKCK, 0, 1);

	gpio_output_value(debug_args->efuse_gpio, 1);
	*reg_ctrl &= ~EFUSE_REG_CTRL_PGEN;

	if (*(volatile unsigned int *)(MCU_TCSM_RETVAL) != SC_ERR_SUCC) {
		printf("burn rckey err, ret val %x\n",*(volatile unsigned int *)(MCU_TCSM_RETVAL));
		return -ESEC;
	}

	return 0;
}

int cpu_load_nku(unsigned int *idata, unsigned int length)
{
	unsigned int ret;
	unsigned int iLoop;
	volatile struct sc_args *args;
	args = (volatile struct sc_args *)GET_SC_ARGS();
	volatile unsigned int *nku = (volatile unsigned int *)MCU_TCSM_NKU;

//	printf("xxxxxxxxxxx func : %s\n",__func__);

	set_rsakey(idata+2,length-8);

	nku[0] = rsakeylen * 8;
	nku[1] = rsakeylen * 8;

	for (iLoop = 0; iLoop < 32; iLoop++)
		nku[iLoop + 2] = rsakey[iLoop];
	for (iLoop = 0; iLoop < 32; iLoop++)
		nku[iLoop + 2 + 32] = rsakey[iLoop + rsakeylen / 4];

	args->arg[0] = MCU_TCSM_PADDR(nku);
	ret = secall(args, SC_FUNC_BURNNKU, 0, 1);

	if (*(volatile unsigned int *)(MCU_TCSM_RETVAL) != SC_ERR_SUCC) {
		printf("burn nku err, ret val %x\n",*(volatile unsigned int *)(MCU_TCSM_RETVAL));
		return -ESEC;
	}

	return 0;
}

int cpu_burn_nku(void *idata,unsigned int length)
{
	if(EFUSTATE_NKU_PRT)
		return 0;

	if(cpu_load_nku(idata, length) < 0)
		return -ESEC;

	if(cpu_wtotp(WT_OTP_NKU) < 0)
		return -ESEC;

	return 0;
}

int cpu_get_enckey(unsigned int *odata)
{
	unsigned int ret;
	unsigned int iLoop;
	volatile struct sc_args *args;
	args = (volatile struct sc_args *)GET_SC_ARGS();
	volatile unsigned int *enckey = (volatile unsigned int *)(MCU_TCSM_RSAENCKEY);
	volatile unsigned int *enckey_len = (volatile unsigned int *)(MCU_TCSM_RSAENCKEYLEN);
	volatile unsigned int *nku = (volatile unsigned int *)(MCU_TCSM_NKU);

	printf("xxxxxxxxxxx func : %s\n",__func__);

	nku[0] = rsakeylen * 8;
	nku[1] = rsakeylen * 8;

	for (iLoop = 0; iLoop < 32; iLoop++)
		nku[iLoop + 2] = rsakey[iLoop];
	for (iLoop = 0; iLoop < 32; iLoop++)
		nku[iLoop + 2 + 32] = rsakey[iLoop + rsakeylen / 4];

	args->arg[0] = MCU_TCSM_PADDR(nku);
	ret = secall(args, SC_FUNC_RSAENCK, 0, 1);

	if (*(volatile unsigned int *)(MCU_TCSM_RETVAL) != SC_ERR_SUCC) {
		printf("get rsa enckey err, ret val %x\n",*(volatile unsigned int *)(MCU_TCSM_RETVAL));
		return -ESEC;
	}

	for(iLoop = 0; iLoop < *enckey_len; iLoop++)
		odata[iLoop] = enckey[iLoop];

	for(iLoop = 0; iLoop < *enckey_len; iLoop++)
		printf("enckey[%d]: %x\n", iLoop, odata[iLoop]);

	return 0;
}

int cpu_burn_ukey(void *idata)
{
	unsigned int ret;
	unsigned int iLoop;
	unsigned int encukey[4] = {0};
	volatile struct sc_args *args;
	args = (volatile struct sc_args *)GET_SC_ARGS();
	volatile unsigned int *ukey = (volatile unsigned int *)MCU_TCSM_PUTUKEY;
	unsigned int *rsaukey = (unsigned int *)idata;

	printf("xxxxxxxxxxx func : %s\n",__func__);

	if(EFUSTATE_UK_PRT)
		return 0;

//	do_rsa(rsaukey, rsakeylen, encukey, rsakey, rsakeylen);
//	for(iLoop = 0; iLoop < 4; iLoop++)
//		printf("encukey[%d]: %x\n", iLoop, encukey[iLoop]);

//#define BURN_UKEY_DEBUG
#ifdef BURN_UKEY_DEBUG
	aes(encukey, ukey, 16, AES_BY_CKEY, 1);
	for(iLoop = 0; iLoop < 4; iLoop++)
		printf("ukey[%d]: %x\n", iLoop, ukey[iLoop]);
	args->arg[0] = 0;
#else
	for(iLoop = 0; iLoop < 4; iLoop++)
		ukey[iLoop] = rsaukey[iLoop] /*encukey[iLoop]*/;
	args->arg[0] = 0;
#endif
	args->arg[1] = MCU_TCSM_PADDR(ukey);
	ret = secall(args, SC_FUNC_BURNUK, 0, 1);

	if (*(volatile unsigned int *)(MCU_TCSM_RETVAL) != SC_ERR_SUCC) {
		printf("burn ukey err, ret val %x\n",*(volatile unsigned int *)(MCU_TCSM_RETVAL));
		return -ESEC;
	}

	if(cpu_wtotp(WT_OTP_UK) < 0)
		return -ESEC;

	return 0;
}

int cpu_burn_secboot_enable(void)
{
	/*efuse config*/
	volatile unsigned int *reg_ctrl = (volatile unsigned int *)EFUSE_REG_CTRL;
	volatile unsigned int *reg_stat = (volatile unsigned int *)EFUSE_REG_STAT;
	volatile unsigned int *reg_data1 = (volatile unsigned int *)EFUSE_REG_DAT1;

	*reg_ctrl |= EFUSE_REG_CTRL_PGEN; /*pg en*/
	gpio_output_value(debug_args->efuse_gpio, 0);

	*reg_data1 = (1 << EFUSE_PTCOFF_SEC); /* security boot enable, security boot enable protected */
	*reg_ctrl &= ~(0x7f << EFUSE_REGOFF_CRTL_ADDR | 0x1f << EFUSE_REGOFF_CRTL_LENG); /* clean address ,length*/
	*reg_ctrl |= EFUSE_ADDR_PROT << EFUSE_REGOFF_CRTL_ADDR; /* set address */
	*reg_ctrl |= EFUSE_REG_CTRL_WTEN; /*write en*/

	while(!(*reg_stat & EFUSE_REG_STAT_WTDONE));

	gpio_output_value(debug_args->efuse_gpio, 1);
	*reg_ctrl &= ~EFUSE_REG_CTRL_PGEN;

	efuse_update_state();

	*reg_ctrl |= EFUSE_REG_CTRL_PGEN; /*pg en*/
	gpio_output_value(debug_args->efuse_gpio, 0);

	*reg_data1 = (1 << EFUSE_PTCOFF_SCB); /* security boot enable, security boot enable protected */
	*reg_ctrl &= ~(0x7f << EFUSE_REGOFF_CRTL_ADDR | 0x1f << EFUSE_REGOFF_CRTL_LENG); /* clean address ,length*/
	*reg_ctrl |= EFUSE_ADDR_PROT << EFUSE_REGOFF_CRTL_ADDR; /* set address */
	*reg_ctrl |= EFUSE_REG_CTRL_WTEN; /*write en*/

	while(!(*reg_stat & EFUSE_REG_STAT_WTDONE));

	gpio_output_value(debug_args->efuse_gpio, 1);
	*reg_ctrl &= ~EFUSE_REG_CTRL_PGEN;

	efuse_update_state();

	return 0;
}
