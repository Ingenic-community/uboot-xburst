#ifndef X1XXX_PHY
#define X1XXX_PHY
#include <asm/io.h>

#define PHY_BASE						(0xb3011000)
#define APB_BASE						(0xb3012000)
#define AHB_BASE						(0xb34f0000)

/**********          PHY ADDR  ********************/
#define INNO_CHANNEL_EN		0x0
#define INNO_MEM_CFG		0x04
#define INNO_TRAINING_CTRL	0x08
#define INNO_WL_MODE1		0x0C
#define INNO_WL_MODE2		0x10
#define INNO_CL			0x14
#define INNO_AL			0x18
#define INNO_CWL		0x1C
#define INNO_DQ_WIDTH		0x7C

#define INNO_PLL_FBDIV		0x80
#define INNO_PLL_CTRL		0x84
#define INNO_PLL_PDIV		0x88

#define INNO_WL_DONE		0xC0
#define INNO_PLL_LOCK		0xC8
#define INNO_CALIB_DONE		0xCC

#define INNO_INIT_COMP		0xD0

#define DDR_APB_PHY_INIT	(APB_BASE + 0x8c)

#define phy_writel(value, reg)  writel((value), PHY_BASE+reg)
#define phy_readl(reg)          readl(PHY_BASE+reg)
#endif

