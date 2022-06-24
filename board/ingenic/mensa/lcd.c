/*
 * Ingenic mensa lcd code
 *
 * Copyright (c) 2013 Ingenic Semiconductor Co.,Ltd
 * Author: Huddy <hyli@ingenic.cn>
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

#include <regulator.h>
#include <asm/gpio.h>
#include <jz_lcd/jz4775_lcd.h>

void board_set_lcd_power_on(void)
{
#ifndef CONFIG_MENSA_V20
	char *id = "OUT7";
	struct regulator *lcd_regulator = regulator_get(id);
	regulator_set_voltage(lcd_regulator, 3300000, 3300000);
	regulator_enable(lcd_regulator);
#else
	unsigned int LCD_PW_EN = GPIO_PE(3);
	gpio_direction_output(LCD_PW_EN, 1);
#endif
}
#ifdef CONFIG_VIDEO_KFM701A_21_1A
static struct smart_lcd_data_table kfm701a21_1a_data_table[] = {
	/* soft reset */
	{0x0600, 0x0001, 0, 10000},
	/* soft reset */
	{0x0600, 0x0000, 0, 10000},
	{0x0606, 0x0000, 0, 10000},
	{0x0007, 0x0001, 0, 10000},
	{0x0110, 0x0001, 0, 10000},
	{0x0100, 0x17b0, 0, 0},
	{0x0101, 0x0147, 0, 0},
	{0x0102, 0x019d, 0, 0},
	{0x0103, 0x8600, 0, 0},
	{0x0281, 0x0010, 0, 10000},
	{0x0102, 0x01bd, 0, 10000},
	/* initial */
	{0x0000, 0x0000, 0, 0},
	{0x0001, 0x0000, 0, 0},
	{0x0002, 0x0400, 0, 0},
	/* up:0x1288 down:0x12B8 left:0x1290 right:0x12A0 */
	{0x0003, 0x12b8, 0, 0}, /* BGR */
	//{0x0003, 0x02b8, 0, 0}, /* RGB */
	{0x0006, 0x0000, 0, 0},
	{0x0008, 0x0503, 0, 0},
	{0x0009, 0x0001, 0, 0},
	{0x000b, 0x0010, 0, 0},
	{0x000c, 0x0000, 0, 0},
	{0x000f, 0x0000, 0, 0},
	{0x0007, 0x0001, 0, 0},
	{0x0010, 0x0010, 0, 0},
	{0x0011, 0x0202, 0, 0},
	{0x0012, 0x0300, 0, 0},
	{0x0020, 0x021e, 0, 0},
	{0x0021, 0x0202, 0, 0},
	{0x0022, 0x0100, 0, 0},
	{0x0090, 0x0000, 0, 0},
	{0x0092, 0x0000, 0, 0},
	{0x0100, 0x16b0, 0, 0},
	{0x0101, 0x0147, 0, 0},
	{0x0102, 0x01bd, 0, 0},
	{0x0103, 0x2c00, 0, 0},
	{0x0107, 0x0000, 0, 0},
	{0x0110, 0x0001, 0, 0},
	{0x0210, 0x0000, 0, 0},
	{0x0211, 0x00ef, 0, 0},
	{0x0212, 0x0000, 0, 0},
	{0x0213, 0x018f, 0, 0},
	{0x0280, 0x0000, 0, 0},
	{0x0281, 0x0001, 0, 0},
	{0x0282, 0x0000, 0, 0},
	/* gamma corrected value table */
	{0x0300, 0x0101, 0, 0},
	{0x0301, 0x0b27, 0, 0},
	{0x0302, 0x132a, 0, 0},
	{0x0303, 0x2a13, 0, 0},
	{0x0304, 0x270b, 0, 0},
	{0x0305, 0x0101, 0, 0},
	{0x0306, 0x1205, 0, 0},
	{0x0307, 0x0512, 0, 0},
	{0x0308, 0x0005, 0, 0},
	{0x0309, 0x0003, 0, 0},
	{0x030a, 0x0f04, 0, 0},
	{0x030b, 0x0f00, 0, 0},
	{0x030c, 0x000f, 0, 0},
	{0x030d, 0x040f, 0, 0},
	{0x030e, 0x0300, 0, 0},
	{0x030f, 0x0500, 0, 0},
	/* secorrect gamma2 */
	{0x0400, 0x3500, 0, 0},
	{0x0401, 0x0001, 0, 0},
	{0x0404, 0x0000, 0, 0},
	{0x0500, 0x0000, 0, 0},
	{0x0501, 0x0000, 0, 0},
	{0x0502, 0x0000, 0, 0},
	{0x0503, 0x0000, 0, 0},
	{0x0504, 0x0000, 0, 0},
	{0x0505, 0x0000, 0, 0},
	{0x0600, 0x0000, 0, 0},
	{0x0606, 0x0000, 0, 0},
	{0x06f0, 0x0000, 0, 0},
	{0x07f0, 0x5420, 0, 0},
	{0x07f3, 0x288a, 0, 0},
	{0x07f4, 0x0022, 0, 0},
	{0x07f5, 0x0001, 0, 0},
	{0x07f0, 0x0000, 0, 0},
	/* end of gamma corrected value table */
	{0x0007, 0x0173, 0, 0},
	/* Write Data to GRAM */
	{0, 0x0202, 1, 10000},
	/* Set the start address of screen, for example (0, 0) */
	{0x200,0, 0, 1},
	{0x201,0, 0, 1},
	{0, 0x202, 1, 100}
};
#endif
struct jzfb_config_info jzfb1_init_data = {
#if defined(CONFIG_VIDEO_BYD_BM8766U)
	.modes = &jzfb1_videomode,
	.lcd_type = LCD_TYPE_GENERIC_24_BIT,
	.bpp = 24,

	.pixclk_falling_edge = 0,
	.date_enable_active_low = 0,

	.lvds = 0,
	.dither_enable = 0,
#elif defined(CONFIG_VIDEO_BM347WV_F_8991FTGF)
	.modes = &jzfb1_videomode,
	.lcd_type = LCD_TYPE_GENERIC_24_BIT,
	.bpp = 24,

	.pixclk_falling_edge = 1,
	.date_enable_active_low = 0,

	.lvds = 0,
	.dither_enable = 0,
#elif defined(CONFIG_VIDEO_KFM701A_21_1A)
	.num_modes = 1,
	.modes = &jzfb1_videomode,

	.lcd_type = LCD_TYPE_LCM,
	.bpp = 18,
//	.width = 39,
//	.height = 65,
	.pinmd = 0,

	.pixclk_falling_edge = 0,
	.date_enable_active_low = 0,

//	.alloc_vidmem = 1,
	.lvds = 0,

	.smart_config.smart_type = SMART_LCD_TYPE_PARALLEL,
	.smart_config.cmd_width = SMART_LCD_CWIDTH_18_BIT_ONCE,  /* KFM701A21_1A: 18-bit? 16-bit? */
	.smart_config.data_width = SMART_LCD_DWIDTH_18_BIT_ONCE_PARALLEL_SERIAL,
	.smart_config.data_width2 = SMART_LCD_DWIDTH_18_BIT_ONCE_PARALLEL_SERIAL,
	.smart_config.clkply_active_rising = 0,
	.smart_config.rsply_cmd_high = 0,
	.smart_config.csply_active_high = 0,

//	.smart_config.continuous_dma = 0, /* 1: auto continuous dma. 0: trigger DMA_RESTART per-frame dma. */

	.smart_config.write_gram_cmd = 0x00000804, /* 0x00000202(16bit) -->> 0x00000804(18bit) ? */
	.smart_config.bus_width = 18,
	.smart_config.length_data_table = ARRAY_SIZE(kfm701a21_1a_data_table),
	.smart_config.data_table = kfm701a21_1a_data_table,
//	.smart_config.init = 0, /* smart lcd special initial function */

	.dither_enable = 1,
	.dither.dither_red = 1, /* 6bit */
	.dither.dither_red = 1, /* 6bit */
	.dither.dither_red = 1, /* 6bit */
#else
#error "Please add the board data!!!"
#endif
};
#ifdef CONFIG_VIDEO_BYD_BM8766U
#include <jz_lcd/byd_bm8766u.h>
struct byd_bm8766u_data byd_bm8766u_pdata = {
	.gpio_lcd_disp = GPIO_PB(30),
	.gpio_lcd_de = 0,	//GPIO_PC(9),   /* chose sync mode */
	.gpio_lcd_vsync = 0,	//GPIO_PC(19),
	.gpio_lcd_hsync = 0,	//GPIO_PC(18),
};
#endif /* CONFIG_LCD_BYD_BM8766U */

#ifdef CONFIG_VIDEO_KFM701A_21_1A
#include <jz_lcd/kfm701a_21_1a.h>
struct kfm701a21_1a_data kfm701a21_1a_pdata = {
	.gpio_lcd_cs = GPIO_PC(21),
	.gpio_lcd_rst = GPIO_PB(28),
};
#endif

#ifdef CONFIG_VIDEO_BM347WV_F_8991FTGF
#include <jz_lcd/byd_8991.h>
struct byd_8991_data byd_8991_pdata = {
	.gpio_lcd_disp = GPIO_PB(30),
	.gpio_lcd_de = 0,
	.gpio_lcd_vsync = 0,
	.gpio_lcd_hsync = 0,
	.gpio_spi_cs = GPIO_PC(0),
	.gpio_spi_clk = GPIO_PC(1),
	.gpio_spi_mosi = GPIO_PC(10),
	.gpio_spi_miso = GPIO_PC(11),
	.gpio_lcd_back_sel = GPIO_PC(20),
};
#endif /* CONFIG_VIDEO_BM347WV_F_8991FTGF */
