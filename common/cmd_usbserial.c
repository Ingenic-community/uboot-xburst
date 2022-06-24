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

extern int jz_usb_serial_register(const char *type);
extern int usb_gadget_handle_interrupts(void);
extern void jz_usb_serial_unregister(void);
bool jz_usb_serial_flag = 0;

static int do_gser(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char *s = "gser";

	if (argc > 1)
		return CMD_RET_USAGE;

	jz_usb_serial_register(s);
	jz_usb_serial_flag = 0;

	while(!jz_usb_serial_flag)
	{
		usb_gadget_handle_interrupts();
	}

	jz_usb_serial_unregister();

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	gser, 1, 1, do_gser,
	"enter gser mode",
	"enter gser mode"
);



