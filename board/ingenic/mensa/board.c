/*
 * Ingenic mensa setup code
 *
 * Copyright (c) 2013 Ingenic Semiconductor Co.,Ltd
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

#include <common.h>
#include <nand.h>
#include <net.h>
#include <netdev.h>
#include <asm/gpio.h>
#include <asm/arch/cpm.h>
#include <asm/arch/nand.h>
#include <asm/arch/mmc.h>

//#define CONFIG_DWC_DEBUG 1

#ifdef  CONFIG_DWC_DEBUG
#include <asm/io.h>
#include <asm/ddr_dwc.h>
/* #define FUNC_ENTER() printf("%s enter.\n",__FUNCTION__); */
/* #define FUNC_EXIT() printf("%s exit.\n",__FUNCTION__); */
#define FUNC_ENTER()
#define FUNC_EXIT()

static void cpm_regs_print(void)
{
	printf("APLL   0xb0000010: 0x%08lx\n", *(unsigned long *)(0xb0000010));
	printf("MPLL   0xb0000014: 0x%08lx\n", *(unsigned long *)(0xb0000014));
	printf("CPCCR  0xb0000000: 0x%08lx\n", *(unsigned long *)(0xb0000000));
	printf("DDRCDR 0xb0000000: 0x%08lx\n", *(unsigned long *)(0xb000002c));
	printf("\n");
}

static void dump_ddrc_register(void)
{
	printf("DDRC_STATUS             0x%08x\n", ddr_readl(DDRC_STATUS));
	printf("DDRC_CFG                0x%08x\n", ddr_readl(DDRC_CFG));
	printf("DDRC_CTRL               0x%08x\n", ddr_readl(DDRC_CTRL));
	printf("DDRC_LMR                0x%08x\n", ddr_readl(DDRC_LMR));
	printf("DDRC_TIMING1            0x%08x\n", ddr_readl(DDRC_TIMING(1)));
	printf("DDRC_TIMING2            0x%08x\n", ddr_readl(DDRC_TIMING(2)));
	printf("DDRC_TIMING3            0x%08x\n", ddr_readl(DDRC_TIMING(3)));
	printf("DDRC_TIMING4            0x%08x\n", ddr_readl(DDRC_TIMING(4)));
	printf("DDRC_TIMING5            0x%08x\n", ddr_readl(DDRC_TIMING(5)));
	printf("DDRC_TIMING6            0x%08x\n", ddr_readl(DDRC_TIMING(6)));
	printf("DDRC_REFCNT             0x%08x\n", ddr_readl(DDRC_REFCNT));
	printf("DDRC_MMAP0              0x%08x\n", ddr_readl(DDRC_MMAP0));
	printf("DDRC_MMAP1              0x%08x\n", ddr_readl(DDRC_MMAP1));
	printf("DDRC_DLP                0x%08x\n", ddr_readl(DDRC_DLP));
	printf("DDRC_REMAP1             0x%08x\n", ddr_readl(DDRC_REMAP(1)));
	printf("DDRC_REMAP2             0x%08x\n", ddr_readl(DDRC_REMAP(2)));
	printf("DDRC_REMAP3             0x%08x\n", ddr_readl(DDRC_REMAP(3)));
	printf("DDRC_REMAP4             0x%08x\n", ddr_readl(DDRC_REMAP(4)));
	printf("DDRC_REMAP5             0x%08x\n", ddr_readl(DDRC_REMAP(5)));
	printf("DDRC_STRB               0x%08x\n", ddr_readl(DDRC_STRB));
	printf("DDRC_WCMDCTRL1          0x%08x\n", ddr_readl(DDRC_WCMDCTRL1));
	printf("DDRC_RCMDCTRL0          0x%08x\n", ddr_readl(DDRC_RCMDCTRL0));
	printf("DDRC_RCMDCTRL1          0x%08x\n", ddr_readl(DDRC_RCMDCTRL1));
	printf("DDRC_WDATTHD0           0x%08x\n", ddr_readl(DDRC_WDATTHD0));
	printf("DDRC_WDATTHD1           0x%08x\n", ddr_readl(DDRC_WDATTHD1));
	printf("DDRC_IPORTPRI           0x%08x\n", ddr_readl(DDRC_IPORTPRI));
	printf("DDRC_IPORTWPRI          0x%08x\n", ddr_readl(DDRC_IPORTWPRI));
	printf("DDRC_IPORTRPRI          0x%08x\n", ddr_readl(DDRC_IPORTRPRI));
	printf("DDRC_AUTOSR_EN          0x%08x\n", ddr_readl(DDRC_AUTOSR_EN));
	printf("DDRC_AUTOSR_CNT         0x%08x\n", ddr_readl(DDRC_AUTOSR_CNT));
	printf("DDRC_CLKSTP_CFG         0x%08x\n", ddr_readl(DDRC_CLKSTP_CFG));

	/* CHxWDOS */
	{
		int iii;
		for (iii=0; iii<7; iii++) {
			printf("DDRC_CHxWDOS(%d)         0x%08x\n", iii, ddr_readl(0x200+4*iii));
		}
	}
	/* CHxRDOS */
	{
		int iii;
		for (iii=0; iii<7; iii++) {
			printf("DDRC_CHxRDOS(%d)         0x%08x\n", iii, ddr_readl(0x220+4*iii));
		}
	}


	/* CPM_DRCG */
	printf("CPM_DRCG(0xb00000D0)             0x%08lx\n", *(unsigned long *)(0xb00000D0));

	//printf("DDRC_             0x%08x\n", ddr_readl(DDRC_));
}

static void dump_ddrp_register(void)
{
	printf("DDRP_PIR                0x%08x\n", ddr_readl(DDRP_PIR));
	printf("DDRP_PGCR               0x%08x\n", ddr_readl(DDRP_PGCR));
	printf("DDRP_PGSR               0x%08x\n", ddr_readl(DDRP_PGSR));
	printf("DDRP_PTR0               0x%08x\n", ddr_readl(DDRP_PTR0));
	printf("DDRP_PTR1               0x%08x\n", ddr_readl(DDRP_PTR1));
	printf("DDRP_PTR2               0x%08x\n", ddr_readl(DDRP_PTR2));
	printf("DDRP_DCR                0x%08x\n", ddr_readl(DDRP_DCR));
	printf("DDRP_DTPR0              0x%08x\n", ddr_readl(DDRP_DTPR0));
	printf("DDRP_DTPR1              0x%08x\n", ddr_readl(DDRP_DTPR1));
	printf("DDRP_DTPR2              0x%08x\n", ddr_readl(DDRP_DTPR2));
	printf("DDRP_MR0                0x%08x\n", ddr_readl(DDRP_MR0));
	printf("DDRP_MR1                0x%08x\n", ddr_readl(DDRP_MR1));
	printf("DDRP_MR2                0x%08x\n", ddr_readl(DDRP_MR2));
	printf("DDRP_MR3                0x%08x\n", ddr_readl(DDRP_MR3));
	printf("DDRP_ODTCR              0x%08x\n", ddr_readl(DDRP_ODTCR));
	printf("DDRP_DXCCR              0x%08x\n", ddr_readl(DDRP_DXCCR));
	printf("DDRP_ZQXSR0             0x%08x\n", ddr_readl(DDRP_ZQXSR0(0)));

	int i=0;
	/* DDRP_ZQXCR0, ZQXCR1, ZQXSR0, ZQXSR1 */
	for(i=0;i<4;i++) {
		printf("ZQX%dCR0:                0x%08x\n", i, ddr_readl(DDRP_ZQXCR0(i)));
		printf("ZQX%dCR1:                0x%08x\n", i, ddr_readl(DDRP_ZQXCR1(i)));
		printf("ZQX%dSR0:                0x%08x\n", i, ddr_readl(DDRP_ZQXSR0(i)));
		printf("ZQX%dSR1:                0x%08x\n", i, ddr_readl(DDRP_ZQXSR1(i)));
	}

	for(i=0;i<4;i++) {
		printf("DX%dGSR0:                0x%08x\n", i, ddr_readl(DDRP_DXGSR0(i)));
		printf("@pas:DXDQSTR(%d)=        0x%08x\n", i,ddr_readl(DDRP_DXDQSTR(i)));
	}

	printf("\n");
}

static void dump_ddrp_register_all(void)
{
	printf("%s()\n", __FUNCTION__);

	int i=0;
	for(i=0;i<256;i++) {
		printf("DDRP(%02x):        0x%08x\n", i, ddr_readl(DDR_PHY_OFFSET + 0x4*i));
	}
}


#else
#define FUNC_ENTER()
#define FUNC_EXIT()

#define dump_ddrc_register()
#define dump_ddrp_register()
#endif


extern int act8600_regulator_init(void);
extern int jz_net_initialize(bd_t *bis);
#ifdef CONFIG_BOOT_ANDROID
extern void boot_mode_select(void);
#endif

#if defined(CONFIG_CMD_BATTERYDET) && defined(CONFIG_BATTERY_INIT_GPIO)
static void battery_init_gpio(void)
{
}
#endif

int board_early_init_f(void)
{
	/* Power on TF-card */
	gpio_direction_output(GPIO_PB(3), 1);
	act8600_regulator_init();

	return 0;
}

#ifdef CONFIG_USB_GADGET
int jz_udc_probe(void);
void board_usb_init(void)
{
	printf("USB_udc_probe\n");
	jz_udc_probe();
}
#endif /* CONFIG_USB_GADGET */

int misc_init_r(void)
{
#if 0 /* TO DO */
	uint8_t mac[6] = { 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc };

	/* set MAC address */
	eth_setenv_enetaddr("ethaddr", mac);
#endif
#ifdef CONFIG_BOOT_ANDROID
	boot_mode_select();
#endif

#if defined(CONFIG_CMD_BATTERYDET) && defined(CONFIG_BATTERY_INIT_GPIO)
	battery_init_gpio();
#endif
	return 0;
}

#ifdef CONFIG_MTD_NAND_JZ
#ifdef CONFIG_SYS_NAND_SELF_INIT
extern void mtd_nand_probe(void);
void board_nand_init(void)
{
	mtd_nand_probe();
}
#else
extern int mtd_nand_init(struct nand_chip *nand);
int board_nand_init(struct nand_chip *nand)
{
	mtd_nand_init(nand);
	return 0;
}
#endif
#else
int board_nand_init(struct nand_chip *nand)
{
	return 0;
}
#endif


#ifdef CONFIG_MMC
int board_mmc_init(bd_t *bd)
{
	jz_mmc_init();
	return 0;
}
#endif

int board_eth_init(bd_t *bis)
{
	int rv;
#ifndef  CONFIG_USB_ETHER
	/* reset grus DM9000 */
	rv = jz_net_initialize(bis);
#else
	rv = usb_eth_initialize(bis);
#endif
	return rv;
}

/* U-Boot common routines */
int checkboard(void)
{
	puts("Board: mensa (Ingenic XBurst JZ4775 SoC)\n");

#ifdef  CONFIG_DWC_DEBUG
#ifdef CONFIG_SPL_CORE_VOLTAGE
	printf("CPU CORE_VOLTAGE CONFIG_SPL_CORE_VOLTAGE=: %d mV\n", CONFIG_SPL_CORE_VOLTAGE);
#else
	printf("not config CONFIG_SPL_CORE_VOLTAGE, use default CPU CORE_VOLTAGE 1200 mV\n");
#endif

	printf("CONFIG_SYS_MEM_DIV: %d\n", CONFIG_SYS_MEM_DIV);
	printf("MEM Clock: %d MHz\n", CONFIG_SYS_APLL_FREQ/CONFIG_SYS_MEM_DIV);
	//printf("MEM Clock: %d MHz\n", gd->mem_clk/1000000);
	//gd->mem_clk = __cpm_get_mclk();
	cpm_regs_print();
	dump_ddrc_register();
	dump_ddrp_register();
	dump_ddrp_register_all();
#endif  //CONFIG_DWC_DEBUG

	return 0;
}

#ifdef CONFIG_SPL_BUILD

void spl_board_init(void)
{
}

#endif /* CONFIG_SPL_BUILD */
