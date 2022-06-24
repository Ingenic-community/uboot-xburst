/*
 * Notkia 168x board setup code
 *
 * Copyright (c) 2013 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
 *
 * Copyright (c) 2022 SudoMaker
 * Author: Reimu NotMoe <reimu@sudomaker.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
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
#include <asm/gpio.h>
#include <asm/arch/cpm.h>
#include <asm/arch/clk.h>
#include <asm/arch/mmc.h>

int board_early_init_f() {
	return 0;
}

int board_early_init_r() {
	return 0;
}

int misc_init_r() {
	return 0;
}


int spl_start_uboot() {
	return 1;
}

int checkboard() {
	puts("Board: SudoMaker Notkia 168x (Ingenic XBurst X1000E SoC)\n");
	return 0;
}

#ifdef CONFIG_SPL_BUILD
void spl_board_init() {
}
#endif /* CONFIG_SPL_BUILD */
