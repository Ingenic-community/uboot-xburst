/*
 * Startup Code for MIPS32 XBURST M200 CPU-core
 *
 * Copyright (c) 2013 Ingenic Semiconductor Co.,Ltd
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

.macro SETUP_SPL_MMC_HEADER reg=0
#if defined(CONFIG_SPL_MMC_SUPPORT) || defined(CONFIG_SPL_JZMMC_SUPPORT)
	.word 0x4d53504c
	.space 508, 0
	RESERVED_FOR_SC(0)
#endif
.endm


.macro SETUP_SPL_SFC_NAND_HEADER reg=0
#ifdef	CONFIG_SPL_SFC_NAND
	.word 0x03040506
	.word 0x55aa5502
#define SSI_PPB (CONFIG_SPI_NAND_PPB / 32)
#define SSI_BPP (CONFIG_SPI_NAND_BPP / 1024)
        .word (0x00000000 | (SSI_PPB<<16) | (SSI_BPP<<24))
        .word 0x00000000
        .space (512-16),0
	RESERVED_FOR_SC(0)
#endif
.endm

.macro SETUP_SPL_SFC_NOR_HEADER reg=0
#ifdef CONFIG_SPL_SFC_NOR
	.word 0x03040506
	.word 0x55aa5502
	.word 0xffff00aa
    .word 0x00000000
#ifdef CONFIG_SPL_VERSION
	.word (0x00000000 | CONFIG_SPL_VERSION)
	.space (512-20),0
#else
	.space (512-16),0
#endif
	RESERVED_FOR_SC(0)
#endif
.endm


.macro SETUP_SPL_NOR_HEADER	reg=0
#ifdef CONFIG_SPL_NOR_SUPPORT
	.word 0
#endif /* CONFIG_SPL_NOR_SUPPORT */
.endm


	.set noreorder
	.set	mips32
	.globl _start
	.section .start_section

_start:
	/* Setup spl header(params data) for different bootup media. */
	SETUP_SPL_MMC_HEADER()
	SETUP_SPL_SFC_NOR_HEADER()
	SETUP_SPL_SFC_NAND_HEADER()
	SETUP_SPL_NOR_HEADER()


	/* === Real Executable Code Starts at 2048 offset, below === */

	/*
	 * CU0=UM=EXL=IE=0, BEV=ERL=1, IP2~7=1
	 */
	li	t0, 0x0040FC04
	mtc0	t0, CP0_STATUS

	/* CAUSE register */
	/*
	 * DC=1, Disable Count Register.
	 * IV=1, use the specical interrupt vector (0x200)
	 * */
	mfc0	t0, CP0_CAUSE
	li	t1, 0x00800000
	or	t0, t0, t1
	mtc0	t0, CP0_CAUSE


	/* Dsiabel IFU Small Buffer */
	/* Fix Bug: on X2000-MPW */
	mfc0 	t0, $16, 7
	ori 	t0, t0, (1<<3)
	mtc0 	t0, $16, 7
	nop

init_caches:
        li      $2, 3                   // cacheable for kseg0 access
        mtc0    $2, $16                 // CP0_CONFIG
        nop



#if 0
	la	$25, 0x80000000
        ori     $2, $25, 0              // start address
        ori     $3, $2, 0x7fe0          // end address, total 32KB
        mtc0    $0, $28, 0              // CP0_TAGLO
        mtc0    $0, $28, 1              // CP0_DATALO
cache_clear_a_line:
        cache   0x8, 0($2)              // Index_Store_Tag_I
        cache   0x9, 0($2)              // Index_Store_Tag_D
        bne     $2, $3, cache_clear_a_line
        addiu   $2, $2, 32              // increment CACHE_LINE_SIZE
        ori     $2, $25, 0              // start address
        ori     $3, $2, 0x7fe0          // end address, total 32KB spl stack space
        la      $4, 0x1ffff000          // physical address and 4KB page mask
cache_alloc_a_line:
        and     $5, $2, $4
        ori     $5, $5, 1               // V bit of the physical tag
        mtc0    $5, $28, 0              // CP0_TAGLO
        cache   0x8, 0($2)              // Index_Store_Tag_I
        cache   0x9, 0($2)              // Index_Store_Tag_D
        bne     $2, $3, cache_alloc_a_line
        addiu   $2, $2, 32              // increment CACHE_LINE_SIZE
#endif

	sync
	nop


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

	li sp, 0x80001000
#endif

	j board_init_f
	nop

