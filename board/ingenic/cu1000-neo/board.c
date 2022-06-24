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
#include <asm/arch/clk.h>
#include <asm/arch/mmc.h>

extern int jz_net_initialize(bd_t *bis);

int board_early_init_f(void)
{
	return 0;
}

int board_early_init_r(void)
{
	return 0;
}

int misc_init_r(void)
{
#if 0 /* TO DO */
	uint8_t mac[6] = { 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc };

	/* set MAC address */
	eth_setenv_enetaddr("ethaddr", mac);
#endif

	return 0;
}

#ifdef CONFIG_MMC
int board_mmc_init(bd_t *bd)
{
	jz_mmc_init();
	return 0;
}
#endif

#ifdef CONFIG_SYS_NAND_SELF_INIT
void board_nand_init(void)
{
	return 0;
}
#endif

int board_eth_init(bd_t *bis)
{
	int rv;
#ifndef  CONFIG_USB_ETHER
	/* reset grus DM9000 */
#ifdef CONFIG_NET_GMAC
	rv = jz_net_initialize(bis);
#endif
#else
	rv = usb_eth_initialize(bis);
#endif
	return rv;
}

#ifdef CONFIG_SPL_NOR_SUPPORT
int spl_start_uboot(void)
{
	return 1;
}
#endif

/* U-Boot common routines */
int checkboard(void)
{
	puts("Board: CU1000-Neo (Ingenic XBurst X1000E SoC)\n");
	//(*(volatile unsigned int *)0xB0010718) = 1 << 10;
	//(*(volatile unsigned int *)0xB0010724) = 1 << 10;
	//(*(volatile unsigned int *)0xB0010738) = 1 << 10;
	//(*(volatile unsigned int *)0xB0010744) = 1 << 10;
	//(*(volatile unsigned int *)0xB00107F0) = 0x0;
	//puts("Force Pull-up PA10 for eMMC RSTN Signal\n");
	return 0;
}

#ifdef CONFIG_SPL_BUILD
void spl_board_init(void)
{
}
#endif /* CONFIG_SPL_BUILD */