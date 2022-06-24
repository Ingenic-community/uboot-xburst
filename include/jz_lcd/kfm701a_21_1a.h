/*
 * M200 common routines
 *
 * Copyright (c) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Huddy <hyli@ingenic.cn>
 * Based on: xboot/boot/lcd/jz4775_android_lcd.h
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

#ifndef __KFM701A_21_1A_H__
#define __KFM701A_21_1A_H__

struct kfm701a21_1a_data {
	unsigned int gpio_lcd_rst;
	unsigned int gpio_lcd_cs;
};

extern struct kfm701a21_1a_data kfm701a21_1a_pdata ;
extern void board_set_lcd_power_on(void);

#endif /* __TRULY_TFT240240_2_E_H__ */
