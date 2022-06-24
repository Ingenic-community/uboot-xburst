/*
 * Copyright (C) 2015 Ingenic Electronics
 *
 * EDO 1.44 400*400 MIPI LCD Driver (driver's Header File part)
 *
 * Model : E1393AM1.A
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

#ifndef _EDO_E1392AM1_H_
#define _EDO_E1392AM1_H_

#include <jz_lcd/jz_dsim.h>

struct edo_e1392am1_platform_data {
	unsigned int gpio_rest;
	unsigned int gpio_lcd_bl;
};

extern struct edo_e1392am1_platform_data edo_e1392am1_pdata;
extern void edo_e1392am1_sleep_in(struct dsi_device *dsi);
extern void edo_e1392am1_sleep_out(struct dsi_device *dsi);
extern void edo_e1392am1_display_on(struct dsi_device *dsi);
extern void edo_e1392am1_display_off(struct dsi_device *dsi);
extern void edo_e1392am1_set_pixel_off(struct dsi_device *dsi); /* set_pixels_off */
extern void edo_e1392am1_set_pixel_on(struct dsi_device *dsi); /* set_pixels_on */
extern void edo_e1392am1_set_brightness(struct dsi_device *dsi, unsigned int brightness); /* set brightness */


#endif /* _EDO_E1392AM1_H_ */
