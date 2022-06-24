/*
 * jz mtd nand driver probe interface
 *
 *  Copyright (C) 2013 Ingenic Semiconductor Co., LTD.
 *  Author: cli <chen.li@ingenic.com>
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
#include <malloc.h>
#include <errno.h>
#include <nand.h>
#include <linux/list.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <asm/arch/nand.h>
#include <ingenic_nand_mgr/nand_param.h>

extern int mtd_nand_auto_init(nand_flash_param* nand_param);

int mtd_nand_probe(void)
{
	struct mtd_nand_params  *mtd_nand_params = (struct mtd_nand_params *)CONFIG_BOARD_TCSM_BASE;
	nand_flash_param *nand_param = &mtd_nand_params->nand_params;
	return mtd_nand_auto_init(nand_param);
}
