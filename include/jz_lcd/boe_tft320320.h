/*
 * Copyright (C) 2015 Ingenic Electronics
 *
 * BOE 1.54 320*320 TFT LCD Driver (driver's Header File part)
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

#ifndef _BOE_TFT320320_H_
#define _BOE_TFT320320_H_

#include <jz_lcd/jz_dsim.h>

struct boe_tft320320_platform_data {
	unsigned int gpio_rest;
	unsigned int pwm_lcd_brightness;
};

extern struct boe_tft320320_platform_data boe_tft320320_pdata;
extern void boe_tft320320_sleep_in(struct dsi_device *dsi);
extern void boe_tft320320_sleep_out(struct dsi_device *dsi);
extern void boe_tft320320_display_on(struct dsi_device *dsi);
extern void boe_tft320320_display_off(struct dsi_device *dsi);
extern void boe_tft320320_set_pixel_off(struct dsi_device *dsi); /* set_pixels_off */
extern void boe_tft320320_set_pixel_on(struct dsi_device *dsi); /* set_pixels_on */
extern void boe_tft320320_set_brightness(struct dsi_device *dsi, unsigned int brightness); /* set brightness */


#endif /* _BOE_TFT320320_H_ */
