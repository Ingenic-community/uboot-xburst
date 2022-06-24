/*
 * Ingenic dorado epd code
 *
 * Copyright (c) 2014 Ingenic Semiconductor Co.,Ltd
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
#include <jz_pca953x.h>
#include <jz_epd/jz_epd.h>
#include <jz_epd/lcd-ET017QG1.h>

#ifdef CONFIG_PMU_RICOH6x
#define CONFIG_LCD_REGULATOR	"RICOH619_LDO9"
#elif defined CONFIG_PMU_D2041
#define CONFIG_LCD_REGULATOR	""
#endif



void board_set_lcd_power_on(void)
{
	char *id = CONFIG_LCD_REGULATOR;
	struct regulator *epd_regulator = regulator_get(id);

	regulator_set_voltage(epd_regulator, 3300000,3300000);
	regulator_enable(epd_regulator);
}

