/*
 * Ingenic Fastboot Command Explain CMD
 *
 *  Copyright (C) 2013 Ingenic Semiconductor Co., LTD.
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
#include <command.h>
#include <asm/errno.h>
#include <g_fastboot.h>
#include <fastboot.h>

__attribute__ ((weak)) struct partition_info partition_info[16] = {
	[0] = {.pname = "bootloader",.offset = 0*1024*1024,.size = 3*1024*1024},
	[1] = {.pname = "boot",.offset = 3*1024*1024,.size = 8*1024*1024},
	[2] = {.pname = "recovery",.offset = 11*1024*1024,.size = 8*1024*1024},
	[3] = {.pname = "misc",.offset = 19*1024*1024,.size = 4*1024*1024},
	[4] = {.pname = "battery",.offset = 23*1024*1024,.size = 1*1024*1024},
	[5] = {.pname = "cache",.offset = 24*1024*1024,.size = 30*1024*1024},
	[6] = {.pname = "device_id",.offset = 54*1024*1024,.size = 2*1024*1024},
	[7] = {.pname = "system",.offset = 56*1024*1024,.size = 512*1024*1024},
	[8] = {.pname = "data",.offset = 568*1024*1024,.size = 1024*1024*1024},
	[9] = {.pname = "storage1",.offset = 1592*1024*1024,.size = 1024*1024*1024},
	[10] = {.pname = "cmdline",.offset = 2616*1024*1024,.size = 3*1024*1024},
};
 __attribute__ ((weak)) struct board_info board_info = {
	.product = "N/A",
	.variant = "N/A",
	.hardware_ver = "N/A",
	.max_download_size = CONFIG_SYS_MALLOC_LEN,
};

static int do_fastboot(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char *s = "fastboot";

	if (argc > 1)
		return CMD_RET_USAGE;

	g_fastboot_register(s);

	handle_fastboot_cmd();

	g_fastboot_unregister();

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	fastboot, 1, 1, do_fastboot,
	"enter fastboot mode",
	"enter fastboot mode"
);
