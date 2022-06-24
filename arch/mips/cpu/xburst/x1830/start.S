/*
 * Startup Code for MIPS32 XBURST X1830 CPU-core
 *
 * Copyright (c) 2017 Ingenic Semiconductor Co.,Ltd
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
#include <version.h>
#include <asm/regdef.h>
#include <asm/mipsregs.h>
#include <asm/addrspace.h>
#include <asm/cacheops.h>
#include <asm/arch/base.h>

#define RESERVED_FOR_SC(x) .space 1536, x

	.set noreorder
	.globl _start
	.section .start_section
_start:
#ifdef CONFIG_SPL_JZMMC_SUPPORT
	/* magic value ("MSPL") */
	.word 0x4d53504c
	.space 508, 0
	RESERVED_FOR_SC(0)
#elif defined(CONFIG_SPL_SFC_SUPPORT)
	.word 0x03040506
	.word 0x55aa5502
  #ifdef CONFIG_SPL_SFC_NOR
	.word 0x000000aa
  #else /*CONFIG_SPL_SFC_NAND*/
     #define SSI_PPB	(CONFIG_SPI_NAND_PPB / 32)
     #define SSI_BPP (CONFIG_SPI_NAND_BPP / 1024)
	.word (0x00000000 | (SSI_PPB<<16) | (SSI_BPP<<24))
  #endif
	.word 0x00000000
  #ifdef CONFIG_SPL_VERSION
	.word (0x00000000 | CONFIG_SPL_VERSION)
	.space (512-20),0
  #else
	.space (512-16),0
  #endif
	RESERVED_FOR_SC(0)
#elif defined(CONFIG_SPL_NOR_SUPPORT)
	.word 0
#endif /* CONFIG_SPL_NOR_SUPPORT */

	/* Invalidate BTB */
	mfc0	v0, CP0_CONFIG, 7
	nop
	ori	v0, v0, 2
	mtc0	v0, CP0_CONFIG, 7
	nop

        mfc0	v0, $12, 2
        nop
        li	v1, 0x7fffffff
        and	v0, v0, v1
        mtc0	v0, $12, 2
        nop

	/*
	 * CU0=UM=EXL=IE=0, BEV=ERL=1, IP2~7=1
	*/
	li	t0, 0x0040FC04
	mtc0	t0, CP0_STATUS

	/* CAUSE register */
	/* IV=1, use the specical interrupt vector (0x200) */
	li	t1, 0x00800000
	mtc0	t1, CP0_CAUSE

#ifdef CONFIG_SPL_NOR_SUPPORT
	.extern __data_start
	.extern __data_end
	/*Init data section, nor spl*/
	la	t0, __data_start
	la	t1, __data_end
	la	t2, 0x80000000
1:
	lw	t3, 0(t0)
	addiu	t0, t0, 4
	sw	t3, 0(t2)
	addiu	t2, t2, 4
	bne	t0, t1, 1b
	nop
#endif
        j	board_init_f
        nop

#if 0
        .globl led_test
        .set push
        .set noreorder
        /* ARG0 ==> GPIO, ARG1 ==> LOOP COUNT*/
led_test:
        move    t4, a0
        li      t1, 0x1
        andi    t3, t4, 0x1f
        sll     t1, t1, t3       /*get pin mask*/
        srl     t4, t4, 5
        sll     t4, t4, 12       /*get port offset*/
        move    t3, a1           /*get loop count*/
        lui     t0, %hi(GPIO_BASE)
        ori     t0, t0, %lo(GPIO_BASE)
        or      t0, t0, t4
1:
        sw      t1, 0x18(t0)
        sw      t1, 0x24(t0)
        sw      t1, 0x38(t0)
        sw      t1, 0x44(t0)
        li      t2, 0x10000000     /*576M EXCLK*/
2:
        subu    t2, t2, 1
        nop
        bgtz    t2, 2b
        nop
        sw      t1, 0x48(t0)
        li      t2, 0x10000000     /*576M EXCLK*/
3:
        subu    t2, t2, 1
        nop
        bgtz    t2, 3b
        nop
        subu    t3, t3, 1
        bgtz    t3, 1b
        nop
        jr      ra
        .set pop
#endif
