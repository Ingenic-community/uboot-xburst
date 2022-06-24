/*
 * Ingenic mensa setup code
 *
 * Copyright (c) 2013 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
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

#include <common.h>
#include <fastboot.h>

struct partition_info partition_info[PARTITION_NUM] = {
	[0] = {.pname = "bootloader", .offset = 0*1024*1024,.size = 3*1024*1024},
	[1] = {.pname = "boot",.offset = 3*1024*1024,.size = 8*1024*1024},
	[2] = {.pname = "recovery",.offset = 12*1024*1024,.size = 16*1024*1024},
	[3] = {.pname = "pretest",.offset = 28*1024*1024,.size = 16*1024*1024},
	[4] = {.pname = "reserved",.offset = 44*1024*1024,.size = 52*1024*1024},
	[5] = {.pname = "misc",.offset = 96*1024*1024,.size = 4*1024*1024},
	[6] = {.pname = "cache",.offset = 100*1024*1024,.size = 100*1024*1024},
	[7] = {.pname = "system",.offset = 200*1024*1024,.size = 700*1024*1024},
	[8] = {.pname = "data",.offset = 900*1024*1024,.size = 2048*1024*1024},
};

struct board_info board_info = {
	.product = "dorado",
	.variant = "dorado",
	.hardware_ver = "V2.2 20141117",
	.max_download_size = CONFIG_FASTBOOT_MAX_DOWNLOAD_SIZE,
};
