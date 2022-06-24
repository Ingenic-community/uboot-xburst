/*
 * drivers/regulator/ricoh619-regulator.c
 *
 * Regulator driver for RICOH R5T619 power management chip.
 *
 * Copyright (C) 2012-2014 RICOH COMPANY,LTD
 *
 * Based on code
 *	Copyright (C) 2011 NVIDIA Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/* #define DEBUG			1 */
/*#define VERBOSE_DEBUG		1*/
#include <config.h>
#include <common.h>
#include <linux/err.h>
#include <linux/list.h>
#include <regulator.h>
#include <ingenic_soft_i2c.h>

#define EA3056_I2C_ADDR    0x35

#define EA3056_DC1  0x01
#define EA3056_DC2  0x02
#define EA3056_DC3  0x03
#define	EA3056_LDO1 0x04

static struct i2c ea3056_i2c;
static struct i2c *i2c;

static int ea3056_write_reg(u8 reg, u8 *val)
{
	unsigned int  ret;

	ret = i2c_write(i2c, EA3056_I2C_ADDR, reg, 1, val, 1);
	if(ret) {
		debug("EA3056 write register error\n");
		return -EIO;
	}
	return 0;
}

#ifdef CONFIG_SPL_BUILD
int spl_regulator_init()
{
	int ret;

	ea3056_i2c.scl = CONFIG_EA3056_I2C_SCL;
	ea3056_i2c.sda = CONFIG_EA3056_I2C_SDA;
	i2c = &ea3056_i2c;
	i2c_init(i2c);

	ret = i2c_probe(i2c, EA3056_I2C_ADDR);

	return ret;
}

int spl_regulator_set_voltage(enum regulator_outnum outnum, int vol_mv)
{
	char reg;
	u8 vid;
	u8 regvalue;

	switch(outnum) {
		case REGULATOR_CORE:
			reg = EA3056_DC3;
			if ((vol_mv < 1000) || (vol_mv >1400)) {
				debug("voltage for core is out of range\n");
				return -EINVAL;
			}
			break;
		case REGULATOR_MEM:
			reg = EA3056_DC2;
			if ((vol_mv < 1700) || (vol_mv >1900)) {
				debug("voltage for mem is out of range\n");
				return -EINVAL;
			}
			break;
		case REGULATOR_IO:
			reg = EA3056_DC1;
			if ((vol_mv < 3000) || (vol_mv >3400)) {
				debug("voltage for mem is out of range\n");
				return -EINVAL;
			}
			break;
		default:return -EINVAL;
	}

	vid = (vol_mv / CONFIG_SPL_CORE_VOLTAGE_RATIO - 580) / 5;

	if ((vid < 0) || (vid > 45)) {
		debug("unsupported voltage\n");
		return -EINVAL;
	} else {
		regvalue = vid + 0x40;
	}

	return ea3056_write_reg(reg, &regvalue);
}
#endif
