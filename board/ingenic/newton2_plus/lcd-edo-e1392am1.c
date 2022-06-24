/*
 * Copyright (C) 2015 Ingenic Electronics
 *
 * EDO 1.44 400*400 MIPI LCD Driver (driver's data part)
 *
 * Model : E1392AM1.A
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

#include <regulator.h>
#include <lcd.h>
#include <asm/gpio.h>

#include <jz_lcd/jz_lcd_v12.h>
#include <jz_lcd/jz_dsim.h>
#include <jz_lcd/edo_e1392am1.h>

#ifdef CONFIG_ACRAB
#define GPIO_LCD_BLK_EN GPIO_PC(9)
#define MIPI_RST_N     GPIO_PC(16)
#define LCD_VDD_1V8   "RICOH619_LDO9"
#define LCD_VCI_2V8   "RICOH619_LDO10"
#elif defined(CONFIG_IN901)
#define MIPI_RST_N     GPIO_PC(19)
#define LCD_VDD_1V8   "RICOH619_LDO4"
#define LCD_VCI_2V8   "RICOH619_LDO6"
#define BUCK5_3_0V    "RICOH619_DC5"
#elif defined(CONFIG_X3)
#define GPIO_LCD_BLK_EN GPIO_PC(23)
#define MIPI_RST_N     GPIO_PC(19)
#define LCD_VDD_1V8   "RICOH619_LDO4"
#define LCD_VCI_2V8   "RICOH619_LDO6"
#define BUCK5_3_0V    "RICOH619_DC5"
#elif defined(CONFIG_NEWTON2_PLUS)
#define GPIO_LCD_BLK_EN GPIO_PD(0)
#define MIPI_RST_N     GPIO_PD(03)
#define LCD_VDD_1V8   "SM5007_LDO2"
#define LCD_VCI_2V8   "SM5007_LDO4"
#define BUCK5_3_0V    "SM5007_BUCK4"
#else
#define GPIO_LCD_BLK_EN -1
#define MIPI_RST_N     -1
#define LCD_VDD_1V8     NULL
#define LCD_VCI_2V8     NULL
#endif


vidinfo_t panel_info = { 400, 400, LCD_BPP,};

void board_set_lcd_power_on(void)
{
	struct regulator *lcd_vddio_1v8 = NULL;
	struct regulator *lcd_vci_2v8 = NULL;
	struct regulator *buck5_3_0v = NULL;

	lcd_vddio_1v8 = regulator_get(LCD_VDD_1V8);
	if (lcd_vddio_1v8 == NULL)
		return;

	lcd_vci_2v8 = regulator_get(LCD_VCI_2V8);
	if (lcd_vci_2v8 == NULL)
		return;
#if defined(CONFIG_IN901) || defined(CONFIG_X3) || defined(CONFIG_NEWTON2_PLUS)
	buck5_3_0v = regulator_get(BUCK5_3_0V);
	if (buck5_3_0v == NULL)
		return;

	regulator_set_voltage(buck5_3_0v, 3300000, 3300000);
	regulator_enable(buck5_3_0v);
#endif
	regulator_set_voltage(lcd_vddio_1v8, 1800000, 1800000);
	regulator_set_voltage(lcd_vci_2v8, 2800000, 2800000);

	regulator_enable(lcd_vddio_1v8);
	regulator_enable(lcd_vci_2v8);
}

struct edo_e1392am1_platform_data edo_e1392am1_pdata =
{
	.gpio_rest = MIPI_RST_N,
#if defined(CONFIG_ACRAB) || defined(CONFIG_X3) || defined(CONFIG_NEWTON2_PLUS)
	.gpio_lcd_bl = GPIO_LCD_BLK_EN,
#endif
};

struct fb_videomode jzfb1_videomode = {
	.name = "edo_e1392am1-lcd",
	.refresh = 60,
	.xres = 400,
	.yres = 400,
	.pixclock = KHZ2PICOS(9600), //PCLK Frequency: 9.6MHz 400*400*60/1000
	.left_margin  = 0,
	.right_margin = 0,
	.upper_margin = 0,
	.lower_margin = 0,
	.hsync_len = 0,
	.vsync_len = 0,
	.sync = ~FB_SYNC_HOR_HIGH_ACT & ~FB_SYNC_VERT_HIGH_ACT,
	.vmode = FB_VMODE_NONINTERLACED,
	.flag = 0,
};

struct video_config jz_dsi_video_config={
	.no_of_lanes = 1,
	.virtual_channel = 0,
	.color_coding = COLOR_CODE_24BIT,
	.video_mode   = VIDEO_BURST_WITH_SYNC_PULSES,
	.receive_ack_packets = 0,	/* enable receiving of ack packets */
	.is_18_loosely    = 0,
	.data_en_polarity = 1,
};

struct dsi_config jz_dsi_config={
	.max_lanes = 1,
	.max_hs_to_lp_cycles = 100,
	.max_lp_to_hs_cycles = 40,
	.max_bta_cycles = 4095,
	.color_mode_polarity = 1,
	.shut_down_polarity  = 1,
	.auto_clklane_ctrl = 0,
};

struct dsi_device jz_dsi = {
	.dsi_config   = &jz_dsi_config,
	.video_config = &jz_dsi_video_config,
	.max_bps = 500, /* 500Mbps */
};

struct jzfb_config_info jzfb1_init_data = {
	.modes = &jzfb1_videomode,

    .lcd_type = LCD_TYPE_SLCD,
    .bpp = 24,

    .smart_config.smart_type = SMART_LCD_TYPE_PARALLEL,
    .smart_config.clkply_active_rising = 0,
    .smart_config.rsply_cmd_high    = 0,
    .smart_config.csply_active_high = 0,
    .smart_config.bus_width = 8,
    .dither_enable = 1,
    .dither.dither_red   = 1,	/* 6bit */
    .dither.dither_green = 1,	/* 6bit */
    .dither.dither_blue  = 1,	/* 6bit */
};

