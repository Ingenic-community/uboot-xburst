/*
 * JZ LCD PANEL DATA
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

#include <config.h>
#include <serial.h>
#include <common.h>
#include <lcd.h>
#include <linux/list.h>
#include <asm/types.h>
#include <asm/arch-m200/tcu.h>
#include <asm/arch-m200/lcdc.h>
#include <asm/arch-m200/gpio.h>
#include <asm/arch-x1000/tcu.h>
#include <asm/arch-x1000/lcdc.h>
#include <asm/arch-x1000/gpio.h>
#include <regulator.h>
#include <jz_lcd/nhd_160128ugc.h>
#include <jz_lcd/jz_lcd_v13.h>

extern struct jzfb_config_info lcd_config_info;
extern struct smart_lcd_data_table nhd_160128ugc_data_table[];

struct nhd_160128ugc_data nhd_160128ugc_pdata;

vidinfo_t panel_info = { 160, 128, LCD_BPP, }; //LCD_BPP

void panel_pin_init(void)
{
	int ret = 0;
	ret = gpio_request(nhd_160128ugc_pdata.gpio_lcd_cs, "lcd_cs");
	if(ret){
		/*printf("canot request gpio lcd_cs\n");*/
	}

	ret = gpio_request(nhd_160128ugc_pdata.gpio_lcd_rd, "lcd_rd");
	if(ret){
		/*printf("canot request gpio lcd_rd\n");*/
	}

	ret = gpio_request(nhd_160128ugc_pdata.gpio_lcd_rst, "lcd_rst");
	if(ret){
		/*printf("canot request gpio lcd_rst\n");*/
	}

	ret = gpio_request(nhd_160128ugc_pdata.gpio_lcd_bl, "lcd_bl");
	if(ret){
		/*printf("canot request gpio lcd_bl\n");*/
	}
	serial_puts("nhd 160128ugc panel display pin init\n");
}

#ifdef CONFIG_GPIO_LCD_FLAG
void panel_power_on(void)
{
	printf("-----------%d---------------->\n",__LINE__);
	unsigned int value=0;
	unsigned int pull_value;

	pull_value = *((volatile unsigned int*)(0xb0010300+0x70)) &(1 << 3);
	if(pull_value)
		gpio_enable_pull(CONFIG_GPIO_LCD_FLAG);

	gpio_direction_output(nhd_160128ugc_pdata.gpio_lcd_cs, 1);
	gpio_direction_output(nhd_160128ugc_pdata.gpio_lcd_rd, 1);

	/*power reset*/
	gpio_direction_output(nhd_160128ugc_pdata.gpio_lcd_rst, 0);
	mdelay(20);
	gpio_direction_output(nhd_160128ugc_pdata.gpio_lcd_rst, 1);
	mdelay(10);

	gpio_direction_output(nhd_160128ugc_pdata.gpio_lcd_cs, 0);
	gpio_direction_output(nhd_160128ugc_pdata.gpio_lcd_bl, 1);

	value = gpio_get_value(CONFIG_GPIO_LCD_FLAG);
	if(!value) {
		lcd_config_info.smart_config.length_data_table += 1;
		lcd_config_info.smart_config.data_table = nhd_160128ugc_data_table;
	}
	if(pull_value)
		gpio_disable_pull(CONFIG_GPIO_LCD_FLAG);

	serial_puts("nhd 160128ugc panel display on\n");
}
#else
void panel_power_on(void)
{
	printf("-----------%d---------------->\n",__LINE__);
	gpio_direction_output(nhd_160128ugc_pdata.gpio_lcd_cs, 1);
	gpio_direction_output(nhd_160128ugc_pdata.gpio_lcd_rd, 1);

	/*power reset*/
	gpio_direction_output(nhd_160128ugc_pdata.gpio_lcd_rst, 0);
	mdelay(20);
	gpio_direction_output(nhd_160128ugc_pdata.gpio_lcd_rst, 1);
	mdelay(10);

	gpio_direction_output(nhd_160128ugc_pdata.gpio_lcd_cs, 0);
	gpio_direction_output(nhd_160128ugc_pdata.gpio_lcd_bl, 1);

	serial_puts("nhd 160128ugc panel display on\n");
}
#endif

void panel_power_off(void)
{
	gpio_direction_output(nhd_160128ugc_pdata.gpio_lcd_cs, 0);
	gpio_direction_output(nhd_160128ugc_pdata.gpio_lcd_rd, 0);
	gpio_direction_output(nhd_160128ugc_pdata.gpio_lcd_rst, 0);
	gpio_direction_output(nhd_160128ugc_pdata.gpio_lcd_bl, 0);
	serial_puts("nhd 160128ugc panel display off\n");
}

struct fb_videomode jzfb1_videomode = {
	.name = "160x128",
	.refresh = 60,
	.xres = 160,
	.yres = 128,
	.pixclock = KHZ2PICOS(30000),
	.left_margin = 0,
	.right_margin = 0,
	.upper_margin = 0,
	.lower_margin = 0,
	.hsync_len = 0,
	.vsync_len = 0,
	.sync = FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT,
	.vmode = FB_VMODE_NONINTERLACED,
	.flag = 0,
};
