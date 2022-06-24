#ifndef __JZ_SDHCI_REGS_H
#define __JZ_SDHCI_REGS_H

#define BIT(nr)         (1UL << (nr))

#define JZ_SDHCI_HOST_CTRL2_R		(0x3e)
#define JZ_SDHCI_HOST_VER4_ENABLE_BIT		BIT(12)

/* Clock Control Register */
#define JZ_SDHCI_PLL_ENABLE_BIT				BIT(3)

#ifdef CONFIG_FPGA
#define CPM_MSC0_CLK_R						(0xB0000068)
#define CPM_MSC1_CLK_R						(0xB00000a4)
#define MSC_CLK_H_FREQ						(0x1 << 20)
#endif //CONFIG_FPGA

#endif //__JZ_SDHCI_REGS_H
