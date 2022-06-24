/*
 * Copyright (C) 2015 Ingenic Electronics
 *
 * BOE 1.54 320*320 TFT LCD Driver (driver's operation part)
 *
 * Model : BV015Z2M-N00-2B00
 *
 * Author: MaoLei.Wang <maolei.wang@ingenic.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

//#define DEBUG
#include <config.h>
#include <serial.h>
#include <common.h>
#include <lcd.h>
#include <linux/list.h>
#include <linux/fb.h>
#include <asm/types.h>
#include <asm/arch/tcu.h>
#include <asm/arch/lcdc.h>
#include <asm/arch/gpio.h>
#include <regulator.h>

#include <jz_lcd/jz_dsim.h>
#include <jz_lcd/boe_tft320320.h>

#include "../jz_mipi_dsi/jz_mipi_dsih_hal.h"

void panel_pin_init(void)
{
#ifdef DEBUG
	int ret= 0;

	ret = gpio_request(boe_tft320320_pdata.gpio_rest, "lcd mipi panel rest");
	if (!ret) {
		debug("request boe_tft320320 lcd mipi panel rest gpio failed\n");
		return;
	}

	ret = gpio_request(boe_tft320320_pdata.pwm_lcd_brightness,
			"lcd mipi brightness power eanble");
	if (!ret) {
		debug("request boe_tft320320 lcd brightness power eanble gpio failed\n");
		return;
	}
#endif

	debug("boe_tft320320 panel display pin init\n");

	return;
}

void panel_power_on(void)
{
	gpio_direction_output(boe_tft320320_pdata.gpio_rest, 1);
	mdelay(10);//10
	gpio_direction_output(boe_tft320320_pdata.gpio_rest, 0);  //reset active low
	mdelay(1);//1
	gpio_direction_output(boe_tft320320_pdata.gpio_rest, 1);
	mdelay(120);//120
	debug("boe_tft320320 panel display on\n");

	return;
}

void boe_tft320320_sleep_in(struct dsi_device *dsi) /* enter sleep */
{
	struct dsi_cmd_packet data_to_send = {0x05, 0x10, 0x00};

	write_command(dsi, data_to_send);
}

void boe_tft320320_sleep_out(struct dsi_device *dsi) /* exit sleep */
{
	struct dsi_cmd_packet data_to_send = {0x05, 0x11, 0x00};

	write_command(dsi, data_to_send);
}

void boe_tft320320_display_on(struct dsi_device *dsi) /* display on */
{
	struct dsi_cmd_packet data_to_send = {0x05, 0x29, 0x00};

	write_command(dsi, data_to_send);
}

void boe_tft320320_display_off(struct dsi_device *dsi) /* display off */
{
	struct dsi_cmd_packet data_to_send = {0x05, 0x28, 0x00};

	write_command(dsi, data_to_send);
}

/*
 * display inversion on
 */
void boe_tft320320_display_inversion_on(struct dsi_device *dsi)
{
	struct dsi_cmd_packet data_to_send = {0x05, 0x21, 0x00};

	write_command(dsi, data_to_send);
}

/*
 * display inversion off
 */
void boe_tft320320_display_inversion_off(struct dsi_device *dsi)
{
	struct dsi_cmd_packet data_to_send = {0x05, 0x20, 0x00};

	write_command(dsi, data_to_send);
}

/**
 * lcd_open_backlight() - Overwrite the weak function defined at common/lcd.c
 */
void lcd_open_backlight(void)
{
	gpio_direction_output(boe_tft320320_pdata.pwm_lcd_brightness, 1);

	return;
}

/**
 * lcd_close_backlight() - Overwrite the weak function defined at common/lcd.c
 */
__weak void lcd_close_backlight(void)
{
	gpio_direction_output(boe_tft320320_pdata.pwm_lcd_brightness, 0);

	return;
}

void panel_display_on(struct dsi_device *dsi)
{
	return;
}

void panel_init_set_sequence(struct dsi_device *dsi)
{
	int  i;
	struct dsi_cmd_packet boe_tft320320_cmd_list[] = {
		{0x39, 0x02, 0x00, {0xF0, 0xC3}},
		{0x39, 0x02, 0x00, {0xF0, 0x96}},
		{0x39, 0x04, 0x00, {0xb6, 0x8A, 0x07, 0x3b}},
		{0x39, 0x05, 0x00, {0xb5, 0x20, 0x20, 0x00, 0x20}},
		{0x39, 0x03, 0x00, {0xb1, 0x80, 0x10}},
		{0x39, 0x02, 0x00, {0xb4, 0x00}},

		{0x39, 0x02, 0x00, {0xc5, 0x28}},
		{0x39, 0x02, 0x00, {0xc1, 0x04}},
#ifdef CONFIG_BOE_TFT320320_ROTATION_180
		{0x39, 0x02, 0x00, {0x36, 0x88}},
		{0x39, 0x05, 0x00, {0x2a, 0x00,     0x00,       319 >> 8, 319 & 0xff}},
		{0x39, 0x05, 0x00, {0x2b, 160 >> 8, 160 & 0xff, 479 >> 8, 479 & 0xff}},
		{0x39, 0x03, 0x00, {0x44, 320 >> 8, 320 & 0xff}},
#else // 0 angle rotation
		{0x39, 0x02, 0x00, {0x36, 0x48}},
		{0x39, 0x05, 0x00, {0x2a, 0x00, 0x00, 319 >> 8, 319 & 0xff}},
		{0x39, 0x05, 0x00, {0x2b, 0x00, 0x00, 319 >> 8, 319 & 0xff}},
		{0x39, 0x03, 0x00, {0x44, 0x00, 0x00}},
#endif
		{0x05, 0x21, 0x00},
		{0x39, 0x09, 0x00, {0xE8, 0x40, 0x84, 0x1b, 0x1b, 0x10, 0x03, 0xb8, 0x33}},

		{0x39, 0x0f, 0x00, {0xe0, 0x00, 0x03, 0x07, 0x07, 0x07, 0x23, 0x2b, 0x33, 0x46, 0x1a, 0x19, 0x19, 0x27, 0x2f}},
		{0x39, 0x0f, 0x00, {0xe1, 0x00, 0x03, 0x06, 0x07, 0x04, 0x22, 0x2f, 0x54, 0x49, 0x1b, 0x17, 0x15, 0x25, 0x2d}},
		{0x05, 0x12, 0x00},
		{0x39, 0x05, 0x00, {0x30, 0x00, 0x00, 0x01, 0x3f}},
		{0x05, 0x35, 0x00},
		{0x39, 0x02, 0x00, {0xe4, 0x31}},
		{0x39, 0x02, 0x00, {0xb9, 0x02}},
		{0x39, 0x02, 0x00, {0x3a, 0x77}},
		{0x39, 0x02, 0x00, {0xe4, 0x31}},
		{0x39, 0x02, 0x00, {0xF0, 0x3c}},
		{0x39, 0x02, 0x00, {0xF0, 0x69}},
		{0x05, 0x29, 0x00},
	};

	boe_tft320320_sleep_out(dsi);
	mdelay(10);
	for(i = 0; i < ARRAY_SIZE(boe_tft320320_cmd_list); i++) {
		write_command(dsi, boe_tft320320_cmd_list[i]);
	}


	debug("init BOE 1.54 320*320 TFT LCD...\n");

	return;
}
