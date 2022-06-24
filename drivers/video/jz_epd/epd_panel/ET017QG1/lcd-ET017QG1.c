/*
 * JZ LCD PANEL DATA
 *
 * Copyright (c) 2015 Ingenic Semiconductor Co.,Ltd
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
#include <asm/types.h>
#include <asm/arch-m200/tcu.h>
#include <asm/arch-m200/epdc.h>
#include <asm/arch-m200/gpio.h>
#include <regulator.h>
#include <jz_epd/lcd-ET017QG1.h>

#define GPIO_EPD_EN             GPIO_PC(23)
#define GPIO_EPD_ENOP           GPIO_PC(24)

vidinfo_t panel_info = {320,240,LCD_BPP,};

static struct jz_epd_power_pin power_pin[] = {
#if 0
	{ "epd enable", 0, GPIO_EPD_SIG_CTRL_N, 0, 0 },
	{ "epd pwr0",   1, GPIO_EPD_PWR0,     100, 0 },
	{ "epd pwr1",   1, GPIO_EPD_PWR1,    1000, 0 },
	{ "epd pwr3",   1, GPIO_EPD_PWR3,       0, 0 },
	{ "epd pwr2",   1, GPIO_EPD_PWR2,     100, 0 },
	{ "epd pwr4",   1, GPIO_EPD_PWR4,    1000, 0 }
#endif
#if 1
	{ "EN",           1, GPIO_EPD_EN,         0, 0 },
	{ "ENOP",         1, GPIO_EPD_ENOP,         0, 0 },
#endif
};

static void dorado_epd_power_init(void)
{
	int i = 0;
	for ( i = 0; i < ARRAY_SIZE(power_pin); i++ ) {
		gpio_request(power_pin[i].pwr_gpio, power_pin[i].name);
	}
}

//EPD power up sequence function for epd driver
static void epd_power_on(void)
{
	int i = 0;
	for ( i = 0; i < ARRAY_SIZE(power_pin); i++ ) {
		gpio_direction_output(power_pin[i].pwr_gpio, power_pin[i].active_level);
		udelay(power_pin[i].pwr_on_delay);
	}
}

//EPD power down sequence function for epd driver
static void epd_power_off(void)
{
	int i = 0;
	for ( i = 0; i < ARRAY_SIZE(power_pin); i++ ) {
		gpio_direction_output(power_pin[i].pwr_gpio, power_pin[i].active_level);
		udelay(power_pin[i].pwr_off_delay);
	}
}

struct jz_epd_power_ctrl epd_power_ctrl ={
	.epd_power_init = dorado_epd_power_init,
	.epd_power_on = epd_power_on,
	.epd_power_off = epd_power_off,
};

