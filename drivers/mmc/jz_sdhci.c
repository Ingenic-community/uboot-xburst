/*
 * Ingenic JZ MMC driver
 * wqshao  <wangquan.shao@ingenic.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <common.h>
#include <malloc.h>
#include <sdhci.h>
#include <asm/arch/clk.h>

#include "jz_sdhci_regs.h"

#ifdef CONFIG_SPL_BUILD
/* SPL will only use a single MMC device (CONFIG_JZ_MMC_SPLMSC) */
struct sdhci_host jz_sdhci_host[1];
#endif

static char *JZ_NAME = "MSC";


static void jz_set_mmc_clk(struct sdhci_host *host, unsigned int clock)
{
	unsigned int val;

#ifdef CONFIG_FPGA
  #ifdef CONFIG_JZ_MMC_MSC0
	#define CPM_MSC_CLK_R   CPM_MSC0_CLK_R
  #endif
  #ifdef CONFIG_JZ_MMC_MSC1
	#define CPM_MSC_CLK_R   CPM_MSC1_CLK_R
  #endif
	if (clock > 1000000) {
		val = readl(CPM_MSC_CLK_R);
		val |= MSC_CLK_H_FREQ;
		writel(val, CPM_MSC_CLK_R);
	} else {
		val = readl(CPM_MSC_CLK_R);
		val &= ~MSC_CLK_H_FREQ;
		writel(val, CPM_MSC_CLK_R);
	}
	printf("%s: clk: %d, CPM_MSC0_CLK_R: 0x%x\n", __func__, clock, readl(CPM_MSC0_CLK_R));
#else
  #ifdef CONFIG_JZ_MMC_MSC0
	#define CPM_MSC	MSC0
  #endif
  #ifdef CONFIG_JZ_MMC_MSC1
	#define CPM_MSC	MSC1
  #endif
	/* set clk */
	clk_set_rate(CPM_MSC, clock);
	printf("%s : clk_id[%d], set clk[%d], clk_get_rate=%d\n", __func__,
			CPM_MSC, clock, clk_get_rate(CPM_MSC));
#endif
}

static int jz_sdhci_init(u32 regbase, int index)
{
	struct sdhci_host *host = NULL;

#ifdef CONFIG_SPL_BUILD
	host = &jz_sdhci_host;
#else
	host = (struct sdhci_host *)malloc(sizeof(struct sdhci_host));
#endif
	if (!host) {
		printf("sdhci__host malloc fail!\n");
		return 1;
	}

	host->name = JZ_NAME;
	host->ioaddr = (void *)regbase;
	host->quirks = SDHCI_QUIRK_NO_HISPD_BIT | SDHCI_QUIRK_BROKEN_VOLTAGE
		| SDHCI_QUIRK_WAIT_SEND_CMD | SDHCI_QUIRK_BROKEN_R1B;

	host->voltages = MMC_VDD_32_33 | MMC_VDD_33_34 | MMC_VDD_165_195;
	host->version = sdhci_readw(host, SDHCI_HOST_VERSION);

	host->set_control_reg = NULL;
	host->set_clock = &jz_set_mmc_clk;
	host->index = index;

	host->host_caps = MMC_MODE_HC; //for emmc OCR

	return add_sdhci(host, 24000000, 300000);
}

void jz_mmc_init(void)
{
#if defined(CONFIG_JZ_MMC_MSC0) && (!defined(CONFIG_SPL_BUILD) || defined(CONFIG_JZ_MMC_SPLMSC))
	jz_sdhci_init(MSC0_BASE, 0);
#endif
#if defined(CONFIG_JZ_MMC_MSC1) && (!defined(CONFIG_SPL_BUILD) || defined(CONFIG_JZ_MMC_SPLMSC))
	jz_sdhci_init(MSC1_BASE, 1);
#endif
}

