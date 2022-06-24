/*
 * Ingenic halley2 setup code
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
	[0] = {.pname = "bootloader", .offset = 0*1024, .size =  256*1024},
	[1] = {.pname = "kernel", .offset = 256*1024, .size = 3072*1024},
	[2] = {.pname = "rootfs", .offset = 3456*1024, .size = 8192*1024},
};

struct board_info board_info = {
	.product = "halley2",
	.variant = "halley2",
	.hardware_ver = "V2.0 20151103",
	.max_download_size = CONFIG_FASTBOOT_MAX_DOWNLOAD_SIZE,
};
