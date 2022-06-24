#include <stdio.h>
#include <config.h>

#define APLL 1
#define MPLL 2

#define out_error(fmt,y...) do{						\
		printf("#error "fmt,##y);				\
		printf("please check %s %d\n", __FILE__ ,__LINE__);	\
	}while(0)
#define out_warn(fmt,y...) printf("/* "fmt "*/\n\n",##y)



/* #define CONFIG_SYS_APLL_FREQ		1104000000	/\*If APLL not use mast be set 0*\/ */
/* #define CONFIG_SYS_MPLL_FREQ		600000000	/\*If MPLL not use mast be set 0*\/ */
/* #define CONFIG_CPU_SEL_PLL		APLL */
/* #define CONFIG_DDR_SEL_PLL		MPLL */
/* #define CONFIG_SYS_CPU_FREQ		1104000000 */
/* #define CONFIG_SYS_MEM_FREQ		200000000 */

/* #define CONFIG_SYS_EXTAL		24000000	/\* EXTAL freq: 24 MHz *\/ */
/* #define CONFIG_SYS_HZ			1000 /\* incrementer freq *\/ */


#define M(x) (x * 1000 * 1000)
#define MAX_H0_CLK M(200)
#define MAX_H2_CLK M(200)
#define MAX_P_CLK  M(150)
#define MAX_L2_CLK M(600)

struct pll_setting
{
	unsigned int apll_freq;
	unsigned int mpll_freq;
	unsigned int cpu_freq;
	unsigned int cpu_sel_pll;
	unsigned int ddr_sel_pll;
};
struct div_setting
{
	int cdiv;
	int l2div;
	int h0div;
	int h2div;
	int pdiv;
	int sel_h2pll;
	int sel_h0pll;
	int sel_cpll;
	int sel_src;
};

struct apll_control_register
{
	unsigned int pllst:8;
	unsigned int pllen:1;
	unsigned int pllbp:1;
	unsigned int pllon:1;
	unsigned int reserve2:4;
	unsigned int lock0:1;
	unsigned int pllod:2;
	unsigned int plln:5;
	unsigned int reserve3:1;
	unsigned int pllm:7;
	unsigned int bs:1;
};

struct mpll_control_register
{
	unsigned int pllon:1;
	unsigned int lock:1;
	unsigned int reserve1:4;
	unsigned int pllbp:1;
	unsigned int pllen:1;
	unsigned int reserve2:8;
	unsigned int pllod:2;
	unsigned int plln:5;
	unsigned int reserve3:1;
	unsigned int pllm:7;
	unsigned int bs:1;
};

struct pll_control
{
	unsigned int reserve:16;
	unsigned int pllod:2;
	unsigned int plln:5;
	unsigned int reseve1:1;
	unsigned int pllm:7;
	unsigned int bs:1;
};
static unsigned int gen_pll_regester_nfro(unsigned int extern_clk,
					  unsigned int pll_out_clk,struct pll_control* p)
{
	unsigned int nrok = 0;
	unsigned int mnod = 0;
	int nf = 1, nr = 0, no = 1;
	int fvco = 0, bs = 0;
	int fin = extern_clk/1000000;
	int fout = pll_out_clk/1000000;

	if ((fin < 10) || (fin > 50) || (fout < 36))
		goto err;

	do {
		nrok++;
		nf = (fout * nrok)/fin;
		if ((nf > 128)) goto err;

		if (fin * nf != fout * nrok)
			continue;

		if (nrok <= 64) {
			no = 0;
			nr = nrok;
			fvco = fout * 1;
		} else if (nrok <= 128 && nf%2 == 0) {
			if (nf%2) goto err;
			no = 1;
			nr = nrok/2;
			fvco = fout * 2;
		} else if (nrok <= 256 && nf%4 == 0) {
			if (nf%4) goto err;
			no = 2;
			nr = nrok/4;
			fvco = fout * 4;
		} else if (nrok <= 512 && nf%8 == 0) {
			no = 3;
			nr = nrok/8;
			fvco = fout * 8;
		} else {
			goto err;
		}

		if (fout >= 63 && fvco >= 500) {
			bs = 1;
			break;
		} else if (fout >= 36 && fvco >= 300 && fvco <= 600) {
			bs = 0;
			break;
		}
	} while (1);

	mnod = (bs << 31) | ((nf - 1) << 24) | ((nr - 1) << 18) | (no << 16);
	if(p){
		p->bs = bs;
		p->pllm = nf - 1;
		p->plln = nr - 1;
		p->pllod = no;
	}
	return mnod;
err:
	out_error("no adjust parameter to the fout:%dM fin: %d check %s %d\n",
		  fout, fin,__FILE__,__LINE__);
	return 0;
}
static int find_min_div(unsigned int src_clk,unsigned int max_clk)
{
	int div = 1;
	while(src_clk / div > max_clk)
		div++;
	return div;
}
static void gen_sys_div(struct pll_setting *setting,struct div_setting *div)
{
	unsigned int pll_freq[] ={
		setting->apll_freq,
		setting->mpll_freq
	};
	unsigned int cpll = setting->cpu_sel_pll == APLL ? 0:1;
	unsigned int periph_pll = setting->ddr_sel_pll == APLL ? 0:1;
	div->sel_src = 2;
	div->sel_cpll = (cpll + 1);
	div->sel_h0pll = periph_pll + 1;
	div->sel_h2pll = periph_pll + 1;

	if(pll_freq[cpll] < setting->cpu_freq){
		out_error("%cpll_freq[%d] should be more than cpu_freq[%d]\n",
			  cpll == 0 ?'a':'m',pll_freq[cpll],setting->cpu_freq);
	}

	if((pll_freq[cpll] % setting->cpu_freq) != 0){
		out_error("%cpll_freq[%d] should be divided by cpu_freq[%d]\n",
			  cpll == 0 ?'a':'m',pll_freq[cpll],setting->cpu_freq);
	}

	div->cdiv = pll_freq[cpll] / setting->cpu_freq - 1;
	if((div->cdiv < 0) && (div->cdiv >16))
		out_error("cpu div[%d] is out of range.",div->cdiv);

	div->l2div = find_min_div(pll_freq[cpll],MAX_L2_CLK) - 1;
	if((div->l2div < 0) && (div->l2div >16))
		out_error("l2cache div[%d] is out of range.",div->l2div);

	div->sel_h0pll = periph_pll + 1;
	div->sel_h2pll = periph_pll + 1;

	div->h0div = find_min_div(pll_freq[periph_pll],MAX_H0_CLK) - 1;
	if((div->h0div < 0) && (div->h0div >16))
		out_error("h0 div[%d] is out of range.",div->h0div);\

	div->h2div = find_min_div(pll_freq[periph_pll] ,MAX_H2_CLK) - 1;
	if((div->h2div < 0) && (div->h2div >16))
		out_error("h2 div[%d] is out of range.",div->h2div);
	else {
		unsigned int h2_freq = pll_freq[periph_pll] / (div->h2div + 1);
		unsigned int h2_pdiv = find_min_div(h2_freq,MAX_P_CLK);
		unsigned int p_freq = h2_freq / h2_pdiv;
		div->pdiv = pll_freq[periph_pll] / p_freq - 1;
		if((div->pdiv < 0) && (div->pdiv >16))
			out_error("pdiv[%d] is out of range.",div->pdiv);
	}
}
static void file_head_print(void)
{
	printf("/*\n");
	printf(" * DO NOT MODIFY.\n");
	printf(" *\n");
	printf(" * This file was generated by pll_params_creator\n");
	printf(" *\n");
	printf(" */\n");
	printf("\n");

	printf("#ifndef __PLL_REG_VALUES_H__\n");
	printf("#define __PLL_REG_VALUES_H__\n\n");
}

static void file_end_print(void)
{
	printf("\n#endif /* __PLL_REG_VALUES_H__ */\n");
}
int main(int argc, char *argv[])
{
	struct pll_setting	pllsetting;
	struct div_setting  divsetting;
	unsigned int apll;
	unsigned int mpll;
	struct pll_control apll_ctrl;
	struct pll_control mpll_ctrl;
	pllsetting.apll_freq = CONFIG_SYS_APLL_FREQ;
	pllsetting.mpll_freq = CONFIG_SYS_MPLL_FREQ;
	pllsetting.cpu_freq = CONFIG_SYS_CPU_FREQ;
	pllsetting.cpu_sel_pll = CONFIG_CPU_SEL_PLL;
	pllsetting.ddr_sel_pll = CONFIG_DDR_SEL_PLL;

	apll = gen_pll_regester_nfro(CONFIG_SYS_EXTAL,CONFIG_SYS_APLL_FREQ,&apll_ctrl);
#ifdef CONFIG_SYS_MPLL_FREQ
	mpll = gen_pll_regester_nfro(CONFIG_SYS_EXTAL,CONFIG_SYS_MPLL_FREQ,&mpll_ctrl);
#else
	mpll = 0;
#endif
	gen_sys_div(&pllsetting,&divsetting);
	file_head_print();
	printf("#define APLL_BS_VALUE \t\t 0x%08x\n",apll_ctrl.bs);
	printf("#define APLL_M_VALUE \t\t 0x%08x\n",apll_ctrl.pllm);
	printf("#define APLL_N_VALUE \t\t 0x%08x\n",apll_ctrl.plln);
	printf("#define APLL_OD_VALUE \t\t 0x%08x\n",apll_ctrl.pllod);
	printf("#define APLL_EN_VALUE \t\t 0x%08x\n",apll == 0? 0:1);

	printf("#define MPLL_BS_VALUE \t\t 0x%08x\n",mpll_ctrl.bs);
	printf("#define MPLL_M_VALUE \t\t 0x%08x\n",mpll_ctrl.pllm);
	printf("#define MPLL_N_VALUE \t\t 0x%08x\n",mpll_ctrl.plln);
	printf("#define MPLL_OD_VALUE \t\t 0x%08x\n",mpll_ctrl.pllod);
	printf("#define MPLL_EN_VALUE \t\t 0x%08x\n",mpll == 0? 0:1);

	printf("#define CDIV_REG_VALUE\t\t 0x%08x\n",divsetting.cdiv);
	printf("#define L2DIV_REG_VALUE\t\t 0x%08x\n",divsetting.l2div);
	printf("#define H0DIV_REG_VALUE\t\t 0x%08x\n",divsetting.h0div);
	printf("#define H2DIV_REG_VALUE\t\t 0x%08x\n",divsetting.h2div);
	printf("#define PDIV_REG_VALUE\t\t 0x%08x\n",divsetting.pdiv);

	printf("#define SEL_H2PLL_REG_VALUE\t\t 0x%08x\n",divsetting.sel_h2pll);
	printf("#define SEL_H0PLL_REG_VALUE\t\t 0x%08x\n",divsetting.sel_h0pll);
	printf("#define SEL_CPLL_REG_VALUE\t\t 0x%08x\n",divsetting.sel_cpll);

	printf("#define SEL_SRC_REG_VALUE\t\t 0x%08x\n",divsetting.sel_src);
	file_end_print();
	return 0;
}
