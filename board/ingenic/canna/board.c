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
#ifdef CONFIG_BOOT_ANDROID
extern void boot_mode_select(void);
#endif

#if defined(CONFIG_CMD_BATTERYDET) && defined(CONFIG_BATTERY_INIT_GPIO)
static void battery_init_gpio(void)
{
}
#endif

#ifdef CONFIG_REGULATOR
int regulator_init(void)
{
	int ret;
#ifdef CONFIG_PMU_RICOH6x
	ret = ricoh61x_regulator_init();
#endif
	return ret;
}
#endif /* CONFIG_REGULATOR */

int board_early_init_f(void)
{
	return 0;
}

int board_early_init_r(void)
{

#ifdef CONFIG_REGULATOR
	regulator_init();
#endif
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
	puts("Board: Canna (Ingenic XBurst X1000 SoC)\n");
	return 0;
}

#ifdef CONFIG_SPL_BUILD

void spl_board_init(void)
{
}

extern int mmc_block_read(u32 start, u32 blkcnt, u32 *dst);
extern void sfc_nand_load(long offs,long size,void *dst);
extern void sfc_nor_load(unsigned int src_addr, unsigned int count,unsigned int dst_addr);

#ifdef CONFIG_GET_WIFI_MAC
static char *board_process_wifimac_arg(char *arg)
{
	char *wifi_mac_str = NULL;
	unsigned int mac_addr[512] = {};

#if defined(CONFIG_SPL_SFC_NOR)
	sfc_nor_load(WIFI_MAC_READ_ADDR, WIFI_MAC_READ_COUNT, mac_addr);
#elif defined(CONFIG_SPL_SFC_NAND)
	sfc_nand_load(WIFI_MAC_READ_ADDR, WIFI_MAC_READ_COUNT, mac_addr)
#elif defined(CONFIG_SPL_JZMMC_SUPPORT)
	mmc_block_read(WIFI_MAC_READ_ADDR / 0x200, 1, mac_addr);
#endif

	wifi_mac_str = strstr(arg, "wifi_mac");
	if (wifi_mac_str != NULL)
		memcpy(wifi_mac_str + 9, mac_addr, WIFI_MAC_READ_COUNT);

	return arg;
}
#endif

#ifdef CONFIG_GET_BAT_PARAM
static char *board_process_bat_arg(char *arg)
{
	char *bat_param_str = NULL;
	unsigned char *bat_str = "4400";
	unsigned char buf[512] = {};

#if defined(CONFIG_SPL_SFC_NOR)
	sfc_nor_load(BAT_PARAM_READ_ADDR, BAT_PARAM_READ_COUNT, buf);
#elif defined(CONFIG_SPL_SFC_NAND)
	sfc_nand_load(BAT_PARAM_READ_ADDR, BAT_PARAM_READ_COUNT, buf)
#elif defined(CONFIG_SPL_JZMMC_SUPPORT)
	mmc_block_read(BAT_PARAM_READ_ADDR / 0x200, 1, buf);
#endif

	bat_param_str = strstr(arg, "bat");
	/* [0x69, 0xaa, 0x55] new battery's flag in nv */
	if((bat_param_str != NULL) && (buf[0] == 0x69) && (buf[1] == 0xaa)
			&& (buf[2] ==0x55))
		memcpy(bat_param_str + 4, bat_str, 4);

	return arg;
}
#endif

char *spl_board_process_bootargs(char *arg)
{
#ifdef CONFIG_GET_WIFI_MAC
	arg = board_process_wifimac_arg(arg);
#endif

#ifdef CONFIG_GET_BAT_PARAM
	arg = board_process_bat_arg(arg);
#endif

	return arg;
}

int spl_prepare_for_load_image(void)
{
	return 0;
}

#endif /* CONFIG_SPL_BUILD */
