/*
 * Ingenic dorado lcd code
 *
 * Copyright (c) 2014 Ingenic Semiconductor Co.,Ltd
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
#include <jz_lcd/jz_lcd_v13.h>
#include <jz_lcd/nhd_160128ugc.h>
#include <lcd.h>

//#define CONFIG_SLCD_TRULY_18BIT
#define CONFIG_LCD_REGULATOR    "RICOH619_LDO5"

unsigned long nhd_cmd_buf[]= {
    /* 0x00440044,	    //for 9bit */
    0x22222222,	    //for 8bit
};

#ifdef  CONFIG_REGULATOR
void board_set_lcd_power_on(void)
{
	printf("-----------%d---------------->\n",__LINE__);
	char *id = CONFIG_LCD_REGULATOR;
	struct regulator *lcd_regulator = regulator_get(id);

	regulator_set_voltage(lcd_regulator,3300000, 3300000);
	regulator_enable(lcd_regulator);
}
#endif

struct smart_lcd_data_table nhd_160128ugc_data_table[] = {
	//{SMART_CONFIG_CMD,0x0001},{SMART_CONFIG_DATA,0x0001},{SMART_CONFIG_UDELAY,10000},
	{SMART_CONFIG_CMD,0x06},{SMART_CONFIG_DATA,0x00},
	{SMART_CONFIG_CMD,0x04},{SMART_CONFIG_DATA,0x01}, {SMART_CONFIG_UDELAY,10000},
	{SMART_CONFIG_CMD,0x04},{SMART_CONFIG_DATA,0x00}, {SMART_CONFIG_UDELAY,10000},
	{SMART_CONFIG_CMD,0x02},{SMART_CONFIG_DATA,0x01},
	{SMART_CONFIG_CMD,0x03},{SMART_CONFIG_DATA,0x30},
	{SMART_CONFIG_CMD,0x80},{SMART_CONFIG_DATA,0x00},
	{SMART_CONFIG_CMD,0x08},{SMART_CONFIG_DATA,0x01},
	{SMART_CONFIG_CMD,0x09},{SMART_CONFIG_DATA,0x01},
	{SMART_CONFIG_CMD,0x0a},{SMART_CONFIG_DATA,0x01},
	{SMART_CONFIG_CMD,0x0b},{SMART_CONFIG_DATA,0x0a},
	{SMART_CONFIG_CMD,0x0c},{SMART_CONFIG_DATA,0x0a},
	{SMART_CONFIG_CMD,0x0d},{SMART_CONFIG_DATA,0x0a},
	{SMART_CONFIG_CMD,0x10},{SMART_CONFIG_DATA,0x52},
	{SMART_CONFIG_CMD,0x11},{SMART_CONFIG_DATA,0x38},
	{SMART_CONFIG_CMD,0x12},{SMART_CONFIG_DATA,0x3a},
	{SMART_CONFIG_CMD,0x13},{SMART_CONFIG_DATA,0x00},
	{SMART_CONFIG_CMD,0x14},{SMART_CONFIG_DATA,0x01},
	/* {SMART_CONFIG_CMD,0x16},{SMART_CONFIG_DATA,0x56},   //for 9bit */
	{SMART_CONFIG_CMD,0x16},{SMART_CONFIG_DATA,0x66},   //for 8bit
	{SMART_CONFIG_CMD,0x17},{SMART_CONFIG_DATA,0x00},
	{SMART_CONFIG_CMD,0x18},{SMART_CONFIG_DATA,0x9f},
	{SMART_CONFIG_CMD,0x19},{SMART_CONFIG_DATA,0x00},
	{SMART_CONFIG_CMD,0x1a},{SMART_CONFIG_DATA,0x7f},
	{SMART_CONFIG_CMD,0x20},{SMART_CONFIG_DATA,0x00},
	{SMART_CONFIG_CMD,0x21},{SMART_CONFIG_DATA,0x00},
	{SMART_CONFIG_CMD,0x28},{SMART_CONFIG_DATA,0x7f},
	{SMART_CONFIG_CMD,0x29},{SMART_CONFIG_DATA,0x00},
	{SMART_CONFIG_CMD,0x2e},{SMART_CONFIG_DATA,0x00},
	{SMART_CONFIG_CMD,0x2f},{SMART_CONFIG_DATA,0x00},
	{SMART_CONFIG_CMD,0x33},{SMART_CONFIG_DATA,0x00},
	{SMART_CONFIG_CMD,0x34},{SMART_CONFIG_DATA,0x9f},
	{SMART_CONFIG_CMD,0x35},{SMART_CONFIG_DATA,0x00},
	{SMART_CONFIG_CMD,0x36},{SMART_CONFIG_DATA,0x7f},
	{SMART_CONFIG_CMD,0x06},{SMART_CONFIG_DATA,0x01},

};


struct jzfb_config_info jzfb1_init_data = {
	.num_modes = 1,
	.modes = &jzfb1_videomode,
	.lcd_type = LCD_TYPE_SLCD,
	.bpp    = NBITS(LCD_BPP),
	.pinmd  = 0,

	.smart_config.rsply_cmd_high       = 0,
	.smart_config.csply_active_high    = 0,
	/* write graphic ram command, in word, for example 8-bit bus, write_gram_cmd=C3C2C1C0. */
	.smart_config.newcfg_fmt_conv =  1,
	/* .smart_config.newcfg_cmd_9bit =  1, */
	.smart_config.write_gram_cmd = nhd_cmd_buf,
	.smart_config.length_cmd = ARRAY_SIZE(nhd_cmd_buf),
	.smart_config.bus_width = 8,
	.smart_config.length_data_table =  ARRAY_SIZE(nhd_160128ugc_data_table),
	.smart_config.data_table = nhd_160128ugc_data_table,
	.dither_enable = 0,
};

struct nhd_160128ugc_data nhd_160128ugc_pdata = {
	.gpio_lcd_rd  = CONFIG_GPIO_LCD_RD,
	.gpio_lcd_rst = CONFIG_GPIO_LCD_RST,
	.gpio_lcd_cs  = CONFIG_GPIO_LCD_CS,
	.gpio_lcd_bl  = CONFIG_GPIO_LCD_BL,
};
