/*
 * DDR driver for Synopsys DWC DDR PHY.
 * Used by Jz4775, JZ4780...
 *
 * Copyright (C) 2013 Ingenic Semiconductor Co.,Ltd
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

#define DEBUG
/* #define DEBUG_READ_WRITE */
#include <config.h>
#include <common.h>
#include <ddr/ddr_common.h>
#ifndef CONFIG_BURNER
#include <generated/ddr_reg_values.h>
#undef DDRC_CFG_VALUE
#undef DDRC_MMAP0_VALUE
#undef DDRC_MMAP1_VALUE
#undef DDRC_TIMING4_VALUE
#undef DDRC_AUTOSR_EN_VALUE
#define DDRC_CFG_VALUE          (gd->arch.gi->ddr_change_param.ddr_cfg)
#define DDRC_MMAP0_VALUE        (gd->arch.gi->ddr_change_param.ddr_mmap0)
#define DDRC_MMAP1_VALUE        (gd->arch.gi->ddr_change_param.ddr_mmap1)
#define DDRC_TIMING4_VALUE      (gd->arch.gi->ddr_change_param.ddr_timing4)
#define DDRC_AUTOSR_EN_VALUE    (gd->arch.gi->ddr_change_param.ddr_autosr)
#define remap_array             (gd->arch.gi->ddr_change_param.ddr_remap_array)
#else
#include "ddr_reg_data.h"
#define remap_array REMMAP_ARRAY
#endif
#include <asm/io.h>
#include <asm/arch/clk.h>
#define CONFIG_DWC_DEBUG 0
#define ddr_hang() do{						\
		printf("%s %d\n",__FUNCTION__,__LINE__);	\
		hang();						\
	}while(0)

DECLARE_GLOBAL_DATA_PTR;

#ifdef  CONFIG_DWC_DEBUG
#define FUNC_ENTER() printf("%s enter.\n",__FUNCTION__);
#define FUNC_EXIT() printf("%s exit.\n",__FUNCTION__);

static void dump_ddrc_register(void)
{
	printf("DDRC_STATUS         0x%x\n", ddr_readl(DDRC_STATUS));
	printf("DDRC_CFG            0x%x\n", ddr_readl(DDRC_CFG));
	printf("DDRC_CTRL           0x%x\n", ddr_readl(DDRC_CTRL));
	printf("DDRC_LMR            0x%x\n", ddr_readl(DDRC_LMR));
	printf("DDRC_DLP            0x%x\n", ddr_readl(DDRC_DLP));
	printf("DDRC_TIMING1        0x%x\n", ddr_readl(DDRC_TIMING(1)));
	printf("DDRC_TIMING2        0x%x\n", ddr_readl(DDRC_TIMING(2)));
	printf("DDRC_TIMING3        0x%x\n", ddr_readl(DDRC_TIMING(3)));
	printf("DDRC_TIMING4        0x%x\n", ddr_readl(DDRC_TIMING(4)));
	printf("DDRC_TIMING5        0x%x\n", ddr_readl(DDRC_TIMING(5)));
	printf("DDRC_REFCNT         0x%x\n", ddr_readl(DDRC_REFCNT));
	printf("DDRC_AUTOSR_CNT     0x%x\n", ddr_readl(DDRC_AUTOSR_CNT));
	printf("DDRC_AUTOSR_EN      0x%x\n", ddr_readl(DDRC_AUTOSR_EN));
	printf("DDRC_MMAP0          0x%x\n", ddr_readl(DDRC_MMAP0));
	printf("DDRC_MMAP1          0x%x\n", ddr_readl(DDRC_MMAP1));
	printf("DDRC_REMAP1         0x%x\n", ddr_readl(DDRC_REMAP(1)));
	printf("DDRC_REMAP2         0x%x\n", ddr_readl(DDRC_REMAP(2)));
	printf("DDRC_REMAP3         0x%x\n", ddr_readl(DDRC_REMAP(3)));
	printf("DDRC_REMAP4         0x%x\n", ddr_readl(DDRC_REMAP(4)));
	printf("DDRC_REMAP5         0x%x\n", ddr_readl(DDRC_REMAP(5)));
	printf("DDRC_DWCFG          0x%x\n", ddr_readl(DDRC_DWCFG));
	printf("DDRC_HREGPRO        0x%x\n", ddr_readl(DDRC_HREGPRO));
	printf("DDRC_PREGPRO        0x%x\n", ddr_readl(DDRC_PREGPRO));
	printf("DDRC_CGUC0        0x%x\n", ddr_readl(DDRC_CGUC0));
	printf("DDRC_CGUC1        0x%x\n", ddr_readl(DDRC_CGUC1));

	printf("#define timing1_tWL         %d\n", timing1_tWL);
	printf("#define timing1_tWR         %d\n", timing1_tWR);
	printf("#define timing1_tWTR        %d\n", timing1_tWTR);
	printf("#define timing1_tWDLAT      %d\n", timing1_tWDLAT);

	printf("#define timing2_tRL         %d\n", timing2_tRL);
	printf("#define timing2_tRTP        %d\n", timing2_tRTP);
	printf("#define timing2_tRTW        %d\n", timing2_tRTW);
	printf("#define timing2_tRDLAT      %d\n", timing2_tRDLAT);

	printf("#define timing3_tRP         %d\n", timing3_tRP);
	printf("#define timing3_tCCD        %d\n", timing3_tCCD);
	printf("#define timing3_tRCD        %d\n", timing3_tRCD);
	printf("#define timing3_ttEXTRW     %d\n", timing3_ttEXTRW);

	printf("#define timing4_tRRD        %d\n", timing4_tRRD);
	printf("#define timing4_tRAS        %d\n", timing4_tRAS);
	printf("#define timing4_tRC         %d\n", timing4_tRC);
	printf("#define timing4_tFAW        %d\n", timing4_tFAW);

	printf("#define timing5_tCKE        %d\n", timing5_tCKE);
	printf("#define timing5_tXP         %d\n", timing5_tXP);
	printf("#define timing5_tCKSRE      %d\n", timing5_tCKSRE);
	printf("#define timing5_tCKESR      %d\n", timing5_tCKESR);
	printf("#define timing5_tXS         %d\n", timing5_tXS);
}

static void dump_ddrp_register(void)
{
	printf("DDRP_INNOPHY_PHY_RST		0x%x\n", ddr_readl(DDRP_INNOPHY_PHY_RST));
	printf("DDRP_INNOPHY_MEM_CFG		0x%x\n", ddr_readl(DDRP_INNOPHY_MEM_CFG));
	printf("DDRP_INNOPHY_DQ_WIDTH		0x%x\n", ddr_readl(DDRP_INNOPHY_DQ_WIDTH));
	printf("DDRP_INNOPHY_CL			0x%x\n", ddr_readl(DDRP_INNOPHY_CL));
	printf("DDRP_INNOPHY_CWL		0x%x\n", ddr_readl(DDRP_INNOPHY_CWL));
	printf("DDRP_INNOPHY_PLL_FBDIV		0x%x\n", ddr_readl(DDRP_INNOPHY_PLL_FBDIV));
	printf("DDRP_INNOPHY_PLL_CTRL		0x%x\n", ddr_readl(DDRP_INNOPHY_PLL_CTRL));
	printf("DDRP_INNOPHY_PLL_PDIV		0x%x\n", ddr_readl(DDRP_INNOPHY_PLL_PDIV));
	printf("DDRP_INNOPHY_PLL_LOCK		0x%x\n", ddr_readl(DDRP_INNOPHY_PLL_LOCK));
	printf("DDRP_INNOPHY_TRAINING_CTRL	0x%x\n", ddr_readl(DDRP_INNOPHY_TRAINING_CTRL));
	printf("DDRP_INNOPHY_CALIB_DONE		0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_DONE));
	printf("DDRP_INNOPHY_CALIB_DELAY_AL	0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_DELAY_AL));
	printf("DDRP_INNOPHY_CALIB_DELAY_AH	0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_DELAY_AH));
	printf("DDRP_INNOPHY_CALIB_BYPASS_AL	0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_BYPASS_AL));
	printf("DDRP_INNOPHY_CALIB_BYPASS_AH	0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_BYPASS_AH));
	printf("DDRP_INNOPHY_INIT_COMP		0x%x\n", ddr_readl(DDRP_INNOPHY_INIT_COMP));
}

#else
#define FUNC_ENTER()
#define FUNC_EXIT()

#define dump_ddrc_register()
#define dump_ddrp_register()
#endif

static void mem_remap(void)
{
	int i;
	unsigned int *remap = remap_array;
	for(i = 0;i < ARRAY_SIZE(remap_array);i++)
	{
		ddr_writel(remap[i],DDRC_REMAP(i+1));
	}
}

static void ddrc_post_init(void)
{
	ddr_writel(DDRC_REFCNT_VALUE, DDRC_REFCNT);
	mem_remap();
	debug("DDRC_STATUS: %x\n",ddr_readl(DDRC_STATUS));
	ddr_writel(DDRC_CTRL_VALUE, DDRC_CTRL);

	ddr_writel(DDRC_CGUC0_VALUE, DDRC_CGUC0);
	ddr_writel(DDRC_CGUC1_VALUE, DDRC_CGUC1);

}
static void ddrc_prev_init(void)
{
	FUNC_ENTER();
	/* DDRC CFG init*/
	/* /\* DDRC CFG init*\/ */
	/* ddr_writel(DDRC_CFG_VALUE, DDRC_CFG); */
	/* DDRC timing init*/
	ddr_writel(DDRC_TIMING1_VALUE, DDRC_TIMING(1));
	ddr_writel(DDRC_TIMING2_VALUE, DDRC_TIMING(2));
	ddr_writel(DDRC_TIMING3_VALUE, DDRC_TIMING(3));
	ddr_writel(DDRC_TIMING4_VALUE, DDRC_TIMING(4));
	ddr_writel(DDRC_TIMING5_VALUE, DDRC_TIMING(5));

	/* DDRC memory map configure*/
	ddr_writel(DDRC_MMAP0_VALUE, DDRC_MMAP0);
	ddr_writel(DDRC_MMAP1_VALUE, DDRC_MMAP1);

	/* ddr_writel(DDRC_CTRL_CKE, DDRC_CTRL); */
	ddr_writel(DDRC_CTRL_VALUE & ~(7 << 12), DDRC_CTRL);

	FUNC_EXIT();
}

static enum ddr_type get_ddr_type(void)
{
	int type;
	ddrc_cfg_t ddrc_cfg;
	ddrc_cfg.d32 = DDRC_CFG_VALUE;
	switch(ddrc_cfg.b.TYPE){
	case 3:
		type = LPDDR;
		break;
	case 4:
		type = DDR2;
		break;
	case 5:
		type = LPDDR2;
		break;
	case 6:
		type = DDR3;
		break;
	default:
		type = UNKOWN;
		debug("unsupport ddr type!\n");
		ddr_hang();
	}
	return type;
}

static void ddrc_reset_phy(void)
{
	FUNC_ENTER();
	ddr_writel(0xf << 20, DDRC_CTRL);
	mdelay(1);
	ddr_writel(0x8 << 20, DDRC_CTRL);  //dfi_reset_n low for innophy
	mdelay(1);
	FUNC_EXIT();
}

static struct jzsoc_ddr_hook *ddr_hook = NULL;
void register_ddr_hook(struct jzsoc_ddr_hook * hook)
{
	ddr_hook = hook;
}

static void ddrp_pll_init(void)
{
	unsigned int val;

	val = ddr_readl(DDRP_INNOPHY_PLL_FBDIV);
	val &= ~(0xff);
	val |= 0x14;
	ddr_writel(val, DDRP_INNOPHY_PLL_FBDIV);

	val = ddr_readl(DDRP_INNOPHY_PLL_PDIV);
	val &= ~(0xff);
	val |= 0x5;
	ddr_writel(val, DDRP_INNOPHY_PLL_PDIV);

	/* ddr_writel(0x14, DDRP_INNOPHY_PLL_FBDIV); */
	/* ddr_writel(0x5, DDRP_INNOPHY_PLL_PDIV); */
#ifdef DEBUG_READ_WRITE
	ddr_writel(ddr_readl(DDRP_INNOPHY_PLL_CTRL) | DDRP_PLL_CTRL_PLLPDEN, DDRP_INNOPHY_PLL_CTRL);
	ddr_writel(ddr_readl(DDRP_INNOPHY_PLL_CTRL) & ~DDRP_PLL_CTRL_PLLPDEN, DDRP_INNOPHY_PLL_CTRL);
#else
	ddr_writel(DDRP_PLL_CTRL_PLLPDEN, DDRP_INNOPHY_PLL_CTRL);
	ddr_writel(0, DDRP_INNOPHY_PLL_CTRL);
#endif

	while(!(ddr_readl(DDRP_INNOPHY_PLL_LOCK) & 1 << 3));
}

static void ddrp_register_cfg(void)
{
	unsigned int val;
#ifdef DEBUG_READ_WRITE
	val = ddr_readl(DDRP_INNOPHY_DQ_WIDTH);
	val &= ~(0x3);
	val |= DDRP_DQ_WIDTH_DQ_H | DDRP_DQ_WIDTH_DQ_L;
	ddr_writel(val, DDRP_INNOPHY_DQ_WIDTH);

	val = ddr_readl(DDRP_INNOPHY_MEM_CFG);
	val &= ~(0x3 | 1 << 4);
	val |= 1 << 4 | 3;
	ddr_writel(val, DDRP_INNOPHY_MEM_CFG);

	printf("ddr_readl(DDRP_INNOPHY_CL)  %x\n", ddr_readl(DDRP_INNOPHY_CL));
	printf("ddr_readl(DDRP_INNOPHY_CWL)  %x\n", ddr_readl(DDRP_INNOPHY_CWL));
#else
	ddr_writel(DDRP_DQ_WIDTH_DQ_H | DDRP_DQ_WIDTH_DQ_L, DDRP_INNOPHY_DQ_WIDTH);
	ddr_writel(DDRP_MEMCFG_VALUE, DDRP_INNOPHY_MEM_CFG);
#endif

	val = ddr_readl(DDRP_INNOPHY_CL);
	val &= ~(0xf);
	val |= DDRP_CL_VALUE;
	ddr_writel(val, DDRP_INNOPHY_CL);

	val = ddr_readl(DDRP_INNOPHY_CWL);
	val &= ~(0xf);
	val |= DDRP_CWL_VALUE;
	ddr_writel(val, DDRP_INNOPHY_CWL);

	val = ddr_readl(DDRP_INNOPHY_AL);
	val &= ~(0xf);
	ddr_writel(val, DDRP_INNOPHY_AL);

	printf("ddr_readl(DDRP_INNOPHY_CL)  %x\n", ddr_readl(DDRP_INNOPHY_CL));
	printf("ddr_readl(DDRP_INNOPHY_CWL)  %x\n", ddr_readl(DDRP_INNOPHY_CWL));
	printf("ddr_readl(DDRP_INNOPHY_AL)  %x\n", ddr_readl(DDRP_INNOPHY_AL));
}

union ddrp_calib {
	/** raw register data */
	uint8_t d8;
	/** register bits */
	struct {
		unsigned dllsel:3;
		unsigned ophsel:1;
		unsigned cyclesel:3;
		unsigned reserved7:1;
	} calib;					/* calib delay/bypass al/ah */
};

static void ddrp_hardware_calibration(void)
{
	unsigned int val;
	unsigned int timeout = 1000000;
	/* ddr_writel(ddr_readl(DDRP_INNOPHY_TRAINING_CTRL) | 1, DDRP_INNOPHY_TRAINING_CTRL); */
	ddr_writel(1, DDRP_INNOPHY_TRAINING_CTRL);
	do
	{
		val = ddr_readl(DDRP_INNOPHY_CALIB_DONE);
	} while (((val & 0xf) != 0x3) && timeout--);

	if(!timeout) {
		printf("timeout:INNOPHY_CALIB_DONE %x\n", ddr_readl(DDRP_INNOPHY_CALIB_DONE));
		hang();
	}

	ddr_writel(0, DDRP_INNOPHY_TRAINING_CTRL);

	{
		union ddrp_calib al, ah;
		al.d8 = ddr_readl(DDRP_INNOPHY_CALIB_DELAY_AL);
		printf("auto :CALIB_AL: dllsel %x, ophsel %x, cyclesel %x\n", al.calib.dllsel, al.calib.ophsel, al.calib.cyclesel);
		ah.d8 = ddr_readl(DDRP_INNOPHY_CALIB_DELAY_AH);
		printf("auto:CAHIB_AH: dllsel %x, ophsel %x, cyclesel %x\n", ah.calib.dllsel, ah.calib.ophsel, ah.calib.cyclesel);
	}
}

/*
 * Name     : ddrp_calibration_manual()
 * Function : control the RX DQS window delay to the DQS
 *
 * a_low_8bit_delay	= al8_2x * clk_2x + al8_1x * clk_1x;
 * a_high_8bit_delay	= ah8_2x * clk_2x + ah8_1x * clk_1x;
 *
 * */
static void ddrp_software_calibration(void)
{
#ifdef DEBUG_READ_WRITE
	ddr_writel(ddr_readl(DDRP_INNOPHY_TRAINING_CTRL) | DDRP_TRAINING_CTRL_DSCSE_BP, DDRP_INNOPHY_TRAINING_CTRL);
#else
	ddr_writel(DDRP_TRAINING_CTRL_DSCSE_BP, DDRP_INNOPHY_TRAINING_CTRL);
#endif

	int x, y;
	int c, o, d;
	unsigned int addr = 0xa0010000, val;
	unsigned int i, n, m = 0;
	union ddrp_calib calib_val[8 * 2 * 8];

	for(c = 0; c < 8; c ++)
		for(o = 0; o < 2; o++)
			for(d = 0; d < 8; d++) {
				x = c << 4 | o << 3 | d;
				y = c << 4 | o << 3 | d;
				ddr_writel(x, DDRP_INNOPHY_CALIB_BYPASS_AL);
				ddr_writel(y, DDRP_INNOPHY_CALIB_BYPASS_AH);
				mdelay(10);
				for(i = 0; i < 0xff; i ++) {
					val = 0;
					for(n = 0; n < 4; n++ ) {
						val |= i <<(n * 8);
					}
					*(volatile unsigned int *)(addr + i * 4) = val;
					if(*(volatile unsigned int *)(addr + i * 4) != val) {
						break;
					}
				}
				if(i == 0xff) {
					calib_val[m].calib.cyclesel = c;
					calib_val[m].calib.ophsel = o;
					calib_val[m].calib.dllsel = d;
					m++;
				}
			}

	if(!m) {
		printf("calib bypass fail\n");
		return ;
	}
	/* for(i = 0; i <= m; i++) */
	/* 	printf("byass CALIB_AL: dllsel %x, ophsel %x, cyclesel %x\n", calib_val[i].calib.dllsel, calib_val[i].calib.ophsel, calib_val[i].calib.cyclesel); */
	m /= 2;
	c = calib_val[m].calib.cyclesel;
	o = calib_val[m].calib.ophsel;
	d = calib_val[m].calib.dllsel;

	x = c << 4 | o << 3 | d;
	y = c << 4 | o << 3 | d;
	ddr_writel(x, DDRP_INNOPHY_CALIB_BYPASS_AL);
	ddr_writel(y, DDRP_INNOPHY_CALIB_BYPASS_AH);
	{
		union ddrp_calib al, ah;
		al.d8 = ddr_readl(DDRP_INNOPHY_CALIB_BYPASS_AL);
		printf("bypass :CALIB_AL: dllsel %x, ophsel %x, cyclesel %x\n", al.calib.dllsel, al.calib.ophsel, al.calib.cyclesel);
		ah.d8 = ddr_readl(DDRP_INNOPHY_CALIB_BYPASS_AH);
		printf("bypass:CAHIB_AH: dllsel %x, ophsel %x, cyclesel %x\n", ah.calib.dllsel, ah.calib.ophsel, ah.calib.cyclesel);
	}
}
static void ddrp_calibration(int bypass)
{
	if(bypass)
		ddrp_software_calibration();
	else
		ddrp_hardware_calibration();
}

static void ddr_phy_init(void)
{
	FUNC_ENTER();
	ddrp_pll_init();
	ddrp_register_cfg();
	FUNC_EXIT();
}

static void ddrc_dfi_init(enum ddr_type type, int bypass)
{
	FUNC_ENTER();
	ddr_writel(DDRC_DWCFG_DFI_INIT_START, DDRC_DWCFG); // dfi_init_start high
	ddr_writel(0, DDRC_DWCFG); // set buswidth 16bit
	while(!(ddr_readl(DDRC_DWSTATUS) & DDRC_DWSTATUS_DFI_INIT_COMP)); //polling dfi_init_complete

	ddr_writel(0, DDRC_CTRL); //set dfi_reset_n high
	ddr_writel(DDRC_CFG_VALUE, DDRC_CFG);
	ddr_writel(DDRC_CTRL_CKE, DDRC_CTRL); // set CKE to high

	switch(type) {
	case LPDDR2:
#define DDRC_LMR_MR(n)													\
		1 << 1| DDRC_DLMR_VALUE | DDRC_LMR_START | DDRC_LMR_CMD_LMR |	\
			((DDR_MR##n##_VALUE & 0xff) << 24) |						\
			(((DDR_MR##n##_VALUE >> 8) & 0xff) << (16))
		ddr_writel(DDRC_LMR_MR(63), DDRC_LMR); //set MRS reset
		ddr_writel(DDRC_LMR_MR(10), DDRC_LMR); //set IO calibration
		ddr_writel(DDRC_LMR_MR(1), DDRC_LMR); //set MR1
		ddr_writel(DDRC_LMR_MR(2), DDRC_LMR); //set MR2
		ddr_writel(DDRC_LMR_MR(3), DDRC_LMR); //set MR3
#undef DDRC_LMR_MR
		break;
	case DDR3:
#define DDRC_LMR_MR(n)												\
		DDRC_DLMR_VALUE | DDRC_LMR_START | DDRC_LMR_CMD_LMR |		\
		((DDR_MR##n##_VALUE & 0xffff) << DDRC_LMR_DDR_ADDR_BIT) |	\
			(((DDR_MR##n##_VALUE >> 16) & 0x7) << DDRC_LMR_BA_BIT)

		ddr_writel(DDRC_LMR_MR(0), DDRC_LMR); //MR0
		ddr_writel(DDRC_LMR_MR(1), DDRC_LMR); //MR1
		ddr_writel(DDRC_LMR_MR(2), DDRC_LMR); //MR2
		ddr_writel(DDRC_LMR_MR(3), DDRC_LMR); //MR3
		ddr_writel(DDRC_DLMR_VALUE | DDRC_LMR_START | DDRC_LMR_CMD_ZQCL_CS0, DDRC_LMR); //ZQCL
#undef DDRC_LMR_MR
		break;
	default:
		ddr_hang();
	}

	FUNC_EXIT();
}

struct ddr_calib_value {
	unsigned int rate;
	unsigned int refcnt;
	unsigned char bypass_al;
	unsigned char bypass_ah;
};

#define REG32(addr) *(volatile unsigned int *)(addr)
#define CPM_DDRCDR (0xb000002c)

static void ddr_calibration(struct ddr_calib_value *dcv, int div)
{
	unsigned int val;

	// Set change_en
	val = REG32(CPM_DDRCDR);
	val |= ((1 << 29) | (1 << 25));
	REG32(CPM_DDRCDR) = val;
	while((REG32(CPM_DDRCDR) & (1 << 24)))
		;
	/* // Set clock divider */
	val = REG32(CPM_DDRCDR);
	val &= ~(0xf);
	val |= div;
	REG32(CPM_DDRCDR) = val;

	// Polling PHY_FREQ_DONE
	while(((ddr_readl(DDRC_DWSTATUS) & (1 << 3 | 1 << 1)) & 0xf) != 0xa);
	ddrp_hardware_calibration();
	/* ddrp_software_calibration(); */

	dcv->bypass_al = ddr_readl(DDRP_INNOPHY_CALIB_DELAY_AL);
	/* printf("auto :CALIB_AL: dcv->bypss_al %x\n", dcv->bypass_al); */
	dcv->bypass_ah = ddr_readl(DDRP_INNOPHY_CALIB_DELAY_AH);
	/* printf("auto:CAHIB_AH: dcv->bypss_ah %x\n", dcv->bypass_ah); */

	// Set Controller Freq Exit
	val = ddr_readl(DDRC_DWCFG);
	val |= (1 << 2);
	ddr_writel(val, DDRC_DWCFG);

	// Clear Controller Freq Exit
	val = ddr_readl(DDRC_DWCFG);
	val &= ~(1 << 2);
	ddr_writel(val, DDRC_DWCFG);

	val = REG32(CPM_DDRCDR);
	val &= ~((1 << 29) | (1 << 25));
	REG32(CPM_DDRCDR) = val;
}

static void get_dynamic_calib_value(unsigned int rate)
{
	struct ddr_calib_value *dcv;
	unsigned int drate = 0;
	int div, n, cur_div;
#define CPU_TCSM_BASE (0xb2400000)
	dcv = (struct ddr_calib_value *)(CPU_TCSM_BASE + 2048);
	cur_div = REG32(CPM_DDRCDR) & 0xf;
	div = cur_div + 1;
	do {
		drate = rate / (div + 1);
		if(drate < 100000000) {
			dcv[cur_div].rate = rate;
			dcv[cur_div].refcnt = get_refcnt_value(cur_div);
			ddr_calibration(&dcv[cur_div], cur_div);
			break;
		}
		dcv[div].rate = drate;
		dcv[div].refcnt = get_refcnt_value(div);
		ddr_calibration(&dcv[div], div);
		div ++;
	} while(1);

	/* for(div = 6, n = 0; div > 0; div--, n++) { */
	/* 	dcv[div - 1].rate = rate / div; */
	/* 	if(dcv[div - 1].rate < 100000000) */
	/* 		break; */
	/* 	dcv[div - 1].refcnt = get_refcnt_value(div); */
	/* 	get_calib_value(&dcv[div - 1], div); */
	/* } */
}
void sdram_init(void)
{
	enum ddr_type type;
	unsigned int rate;
	int bypass = 0;

	debug("sdram init start\n");

	soc_ddr_init();

	type = get_ddr_type();
	clk_set_rate(DDR, gd->arch.gi->ddrfreq);
	if(ddr_hook && ddr_hook->prev_ddr_init)
		ddr_hook->prev_ddr_init(type);
	rate = clk_get_rate(DDR);
	debug("DDR clk rate %d\n", rate);

	ddrc_reset_phy();

	ddr_phy_init();
	dump_ddrp_register();

	ddrc_dfi_init(type, bypass);

	ddrc_prev_init();

	/**
	 * bypass  = 1 or calibration = 0
	 */
	bypass = 0;
	ddrp_calibration(bypass);

	ddrc_post_init();

	if(ddr_hook && ddr_hook->post_ddr_init)
		ddr_hook->post_ddr_init(type);

	get_dynamic_calib_value(rate);

	if(DDRC_AUTOSR_EN_VALUE) {
		/* ddr_writel(DDRC_AUTOSR_CNT_VALUE, DDRC_AUTOSR_CNT); */
		ddr_writel(1, DDRC_AUTOSR_EN);
	} else {
		ddr_writel(0, DDRC_AUTOSR_EN);
	}

	ddr_writel(DDRC_HREGPRO_VALUE, DDRC_HREGPRO);
	ddr_writel(DDRC_PREGPRO_VALUE, DDRC_PREGPRO);

	dump_ddrc_register();
	/* DDRC address remap configure*/
	debug("sdram init finished\n");
}

phys_size_t initdram(int board_type)
{
	/* SDRAM size was calculated when compiling. */
#ifndef EMC_LOW_SDRAM_SPACE_SIZE
#define EMC_LOW_SDRAM_SPACE_SIZE 0x10000000 /* 256M */
#endif /* EMC_LOW_SDRAM_SPACE_SIZE */

	unsigned int ram_size;
	ram_size = (unsigned int)(DDR_CHIP_0_SIZE) + (unsigned int)(DDR_CHIP_1_SIZE);

	if (ram_size > EMC_LOW_SDRAM_SPACE_SIZE)
		ram_size = EMC_LOW_SDRAM_SPACE_SIZE;

	return ram_size;
}
