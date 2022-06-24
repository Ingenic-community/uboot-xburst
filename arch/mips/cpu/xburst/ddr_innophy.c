
/*
 * DDR driver for inno DDR PHY.
 * Used by x1xxx
 *
 * Copyright (C) 2017 Ingenic Semiconductor Co.,Ltd
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

/* #define DEBUG */
#include <config.h>
#include <common.h>
#include <ddr/ddr_common.h>
#include <generated/ddr_reg_values.h>
#include <asm/arch/clk.h>
#include "ddr_innophy.h"
#include "ddr_debug.h"

DECLARE_GLOBAL_DATA_PTR;
extern unsigned int sdram_size(int cs, struct ddr_params *p);


int current_ddr_type;


struct ddr_params *ddr_params_p = NULL;
extern void reset_dll(void);
#define BYPASS_ENABLE       1
#define BYPASS_DISABLE      0
#define IS_BYPASS_MODE(x)     (((x) & 1) == BYPASS_ENABLE)
#define DDR_TYPE_MODE(x)     (((x) >> 1) & 0xf)

static void dump_ddrc_register(void)
{
#ifdef CONFIG_DWC_DEBUG
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
	printf("DDRC_TIMING6        0x%x\n", ddr_readl(DDRC_TIMING(6)));
	printf("DDRC_REFCNT         0x%x\n", ddr_readl(DDRC_REFCNT));
	printf("DDRC_MMAP0          0x%x\n", ddr_readl(DDRC_MMAP0));
	printf("DDRC_MMAP1          0x%x\n", ddr_readl(DDRC_MMAP1));
	printf("DDRC_REMAP1         0x%x\n", ddr_readl(DDRC_REMAP(1)));
	printf("DDRC_REMAP2         0x%x\n", ddr_readl(DDRC_REMAP(2)));
	printf("DDRC_REMAP3         0x%x\n", ddr_readl(DDRC_REMAP(3)));
	printf("DDRC_REMAP4         0x%x\n", ddr_readl(DDRC_REMAP(4)));
	printf("DDRC_REMAP5         0x%x\n", ddr_readl(DDRC_REMAP(5)));
	printf("DDRC_AUTOSR_EN      0x%x\n", ddr_readl(DDRC_AUTOSR_EN));
	printf("INNO_DQ_WIDTH   :%X\n",phy_readl(INNO_DQ_WIDTH));
	printf("INNO_PLL_FBDIV  :%X\n",phy_readl(INNO_PLL_FBDIV));
	printf("INNO_PLL_PDIV   :%X\n",phy_readl(INNO_PLL_PDIV));
	printf("INNO_MEM_CFG    :%X\n",phy_readl(INNO_MEM_CFG));
	printf("INNO_PLL_CTRL   :%X\n",phy_readl(INNO_PLL_CTRL));
	printf("INNO_CHANNEL_EN :%X\n",phy_readl(INNO_CHANNEL_EN));
	printf("INNO_CWL        :%X\n",phy_readl(INNO_CWL));
	printf("INNO_CL         :%X\n",phy_readl(INNO_CL));
#endif
}

static void reset_controller(void)
{
	ddr_writel(0xf << 20, DDRC_CTRL);
	mdelay(5);
	ddr_writel(0x8 << 20, DDRC_CTRL);
	mdelay(5);
}

static void ddrc_post_init(void)
{
	ddr_writel(DDRC_REFCNT_VALUE, DDRC_REFCNT);
	debug("DDRC_STATUS: %x\n",ddr_readl(DDRC_STATUS));
	ddr_writel(DDRC_CTRL_VALUE, DDRC_CTRL);
}

static void ddrc_prev_init(void)
{
	dwc_debug("DDR Controller init\n");
//	ddr_writel(DDRC_CTRL_CKE | DDRC_CTRL_ALH, DDRC_CTRL);
//	ddr_writel(0, DDRC_CTRL);
	/* DDRC CFG init*/
//	ddr_writel(DDRC_CFG_VALUE, DDRC_CFG);
	/* DDRC timing init*/
	ddr_writel(DDRC_TIMING1_VALUE, DDRC_TIMING(1));
	ddr_writel(DDRC_TIMING2_VALUE, DDRC_TIMING(2));
	ddr_writel(DDRC_TIMING3_VALUE, DDRC_TIMING(3));
	ddr_writel(DDRC_TIMING4_VALUE, DDRC_TIMING(4));
	ddr_writel(DDRC_TIMING5_VALUE, DDRC_TIMING(5));
	ddr_writel(DDRC_TIMING6_VALUE, DDRC_TIMING(6));

	/* DDRC memory map configure*/
	ddr_writel(DDRC_MMAP0_VALUE, DDRC_MMAP0);
	ddr_writel(DDRC_MMAP1_VALUE, DDRC_MMAP1);
//	ddr_writel(DDRC_CTRL_CKE | DDRC_CTRL_ALH, DDRC_CTRL);
//	ddr_writel(DDRC_REFCNT_VALUE, DDRC_REFCNT);
	ddr_writel(DDRC_CTRL_VALUE & 0xffff8fff, DDRC_CTRL);
}

void ddr_inno_phy_init(void)
{
	u32 reg = 0;
	/*
	 * ddr phy pll initialization
	 */
	phy_writel(0x14, INNO_PLL_FBDIV);
	phy_writel(0x5, INNO_PLL_PDIV);
	phy_writel(0x1a, INNO_PLL_CTRL);
	phy_writel(0x18, INNO_PLL_CTRL);
	printf("ddrp pll lock 0x%x\n", phy_readl(INNO_PLL_LOCK));
	while(!(readl(DDR_APB_PHY_INIT) & (1<<2))); //polling pll lock

	/*
	 * ddr phy register cfg
	 */
	phy_writel(0x3, INNO_DQ_WIDTH);

	if(current_ddr_type == DDR3) {
		phy_writel(DDRP_MEMCFG_VALUE, INNO_MEM_CFG);
		phy_writel(DDRP_CWL_VALUE, INNO_CWL);
		phy_writel(DDRP_CL_VALUE, INNO_CL);

	} else if(current_ddr_type == DDR2) {
		phy_writel(0x11,INNO_MEM_CFG);  // MEMSEL  =  DDR2  ,    BURSEL = burst8
		phy_writel(0x0d,INNO_CHANNEL_EN);
		phy_writel(((DDR_MR0_VALUE&0xf0)>>4)-1, INNO_CWL);
		reg = ((DDR_MR0_VALUE&0xf0)>>4);
		phy_writel(reg, INNO_CL);
	}
	phy_writel(0x0, INNO_AL);

	printf("CWL = 0x%x\n", phy_readl(INNO_CWL));
	printf("CL = 0x%x\n", phy_readl(INNO_CL));
	printf("AL = 0x%x\n", phy_readl(INNO_AL));
}

void ddrc_dfi_init(void)
{
	u32 reg = 0;

	writel(1, DDR_APB_PHY_INIT); //start high
	writel(0, DDR_APB_PHY_INIT); //start low
	while(!(readl(DDR_APB_PHY_INIT) & (1<<1))); //polling dfi init comp
	printf("ddr_inno_phy_init ..! 11:  %X\n", readl(DDR_APB_PHY_INIT));

	ddr_writel(0, DDRC_CTRL);
	ddr_writel(DDRC_CFG_VALUE, DDRC_CFG);
	ddr_writel(0x2, DDRC_CTRL);

	if(current_ddr_type == DDR3) {
#define DDRC_LMR_MR(n)						\
	DDRC_DLMR_VALUE | 0x1 | (2 << 3) |			\
		((DDR_MR##n##_VALUE & 0xffff) << 12) |		\
		(((DDR_MR##n##_VALUE >> 16) & 0x7) << 8)

	printf("MR0 : 0x%x\n", DDRC_LMR_MR(0));
	printf("MR1 : 0x%x\n", DDRC_LMR_MR(1));
	printf("MR2 : 0x%x\n", DDRC_LMR_MR(2));
	printf("MR3 : 0x%x\n", DDRC_LMR_MR(3));
	printf("ZQCL : 0x%x\n", DDRC_DLMR_VALUE | (0x4 << 3) | 0x1);

	ddr_writel(DDRC_LMR_MR(0)/*0x1a30011*/, DDRC_LMR); //MR0
	ddr_writel(DDRC_LMR_MR(1)/*0x6111*/, DDRC_LMR); //MR1
	ddr_writel(DDRC_LMR_MR(2)/*0x8211*/, DDRC_LMR); //MR2
	ddr_writel(DDRC_LMR_MR(3)/*0x311*/, DDRC_LMR); //MR3
	ddr_writel(DDRC_DLMR_VALUE | (0x4 << 3) | 0x1/*0x19*/, DDRC_LMR);

#undef DDRC_LMR_MR
	} else {
		/*DDR2*/

		ddr_writel(0x211,DDRC_LMR);
		printf("DDRC_LMR: %x\n",ddr_readl(DDRC_LMR));
		ddr_writel(0,DDRC_LMR);

		ddr_writel(0x311,DDRC_LMR);
		printf("DDRC_LMR: %x\n", ddr_readl(DDRC_LMR));
		ddr_writel(0,DDRC_LMR);

		ddr_writel(0x111,DDRC_LMR);
		printf("DDRC_LMR: %x\n", ddr_readl(DDRC_LMR));
		ddr_writel(0,DDRC_LMR);

		reg = ((DDR_MR0_VALUE)<<12)|0x011;
		ddr_writel(reg, DDRC_LMR);
		printf("DDRC_LMR, MR0: %x\n", reg);
		ddr_writel(0,DDRC_LMR);

	}
}

void ddrp_wl_training(void)
{

	if(current_ddr_type == DDR3) {
		//write level
		printf("WL_MODE1 : 0x%x\n", DDR_MR1_VALUE & 0xff);
		phy_writel(DDR_MR1_VALUE & 0xff, INNO_WL_MODE1);
		phy_writel(0x40, INNO_WL_MODE2);
		phy_writel(0xa4, INNO_TRAINING_CTRL);
		while (0x3 != phy_readl(INNO_WL_DONE));
		phy_writel(0xa1, INNO_TRAINING_CTRL);
	}

	/* ???? */
	if(current_ddr_type == DDR3)
		phy_writel(0x50, INNO_MEM_CFG);
	else
		phy_writel(0x51, INNO_MEM_CFG);

	writel(0x24,0xb3011028);
}

/*
 * Name     : phy_calibration()
 * Function : control the RX DQS window delay to the DQS
 * */
void phy_calibration(void)
{
	int m = phy_readl(INNO_TRAINING_CTRL);
	printf("INNO_TRAINING_CTRL 1: %x\n", phy_readl(INNO_TRAINING_CTRL));
	m = 0xa1;
	phy_writel(m,INNO_TRAINING_CTRL);
	printf("INNO_TRAINING_CTRL 2: %x\n", phy_readl(INNO_TRAINING_CTRL));
	while (0x3 != phy_readl(INNO_CALIB_DONE));
	printf("calib done: %x\n", phy_readl(INNO_CALIB_DONE));
	phy_writel(0xa0,INNO_TRAINING_CTRL);
	printf("INNO_TRAINING_CTRL 3: %x\n", phy_readl(INNO_TRAINING_CTRL));
}

int get_ddr_type(void)
{
	int type;
#ifndef CONFIG_MULT_DDR_PARAMS_CREATOR
#ifdef CONFIG_DDR_TYPE_DDR3
	type = DDR3;
#else /*CONFIG_DDR_TYPE_DDR2*/
	type = DDR2;
#endif

#else

#define EFUSE_BASE	0xb3540000
#define EFUSE_CTRL		0x0
#define EFUSE_STATE		0x8
#define EFUSE_DATA		0xC
#define DDR_INFO_ADDR	0xE

	unsigned int val, data;

	writel(0, EFUSE_BASE + EFUSE_STATE);

	val = DDR_INFO_ADDR << 21 | 1;
	writel(val, EFUSE_BASE + EFUSE_CTRL);

	while(!(writel(val, EFUSE_BASE + EFUSE_STATE) & 1));
	data = readl(EFUSE_BASE + EFUSE_DATA);
	val = data & 0xFFFF;

	if(val == 0x1111)
		type = DDR2;
	else if(val == 0x2222)
		type = DDR3;
	else
		type = DDR3;

	get_ddr_params(type);
#endif

	return type;

}

/* DDR sdram init */
void sdram_init(void)
{

	unsigned int mode;
	unsigned int bypass = 0;
	unsigned int rate;

	dwc_debug("sdram init start\n");

	current_ddr_type = get_ddr_type();

	clk_set_rate(DDR, CONFIG_SYS_MEM_FREQ);
	reset_dll();
	rate = clk_get_rate(DDR);
	if(rate != CONFIG_SYS_MEM_FREQ)
		dwc_debug("sdram set ddr freq failed\n");

	reset_controller();

#ifdef CONFIG_DDR_AUTO_SELF_REFRESH
	ddr_writel(0x0 ,DDRC_AUTOSR_EN);
#endif

	ddr_inno_phy_init();
	ddrc_dfi_init();

	ddrp_wl_training();
        /* DDR Controller init*/
	ddrc_prev_init();
	phy_calibration();
	dwc_debug("DDR PHY init OK\n");
	ddrc_post_init();

	ddr_writel(ddr_readl(DDRC_STATUS) & ~DDRC_DSTATUS_MISS, DDRC_STATUS);

#ifdef CONFIG_DDR_AUTO_SELF_REFRESH
	if(!bypass)
		ddr_writel(0 , DDRC_DLP);
	ddr_writel(0x1 ,DDRC_AUTOSR_EN);
#endif
	ddr_writel(0 , DDRC_DLP);

	dump_ddrc_register();
	dwc_debug("sdram init finished\n");
#undef DDRTYPE
	(void)rate;(void)bypass;(void)mode;
}

phys_size_t initdram(int board_type)
{
#ifndef EMC_LOW_SDRAM_SPACE_SIZE
#define EMC_LOW_SDRAM_SPACE_SIZE 0x10000000 /* 256M */
#endif /* EMC_LOW_SDRAM_SPACE_SIZE */
        unsigned int ram_size;

	get_ddr_type();

        ram_size = (unsigned int)(DDR_CHIP_0_SIZE) + (unsigned int)(DDR_CHIP_1_SIZE);
        if (ram_size > EMC_LOW_SDRAM_SPACE_SIZE)
                ram_size = EMC_LOW_SDRAM_SPACE_SIZE;

        return ram_size;
}
