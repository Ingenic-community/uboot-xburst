/*
 * JZ4775 OST(TCU) definitions
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

#ifndef __OST_H__
#define __OST_H__

#include <asm/arch/base.h>

#define OSTCCR		0x00
#define OSTER		0x04
#define OSTESR		0x34
#define OSTECR		0x38
#define OSTCR		0x08
#define OSTFR		0x0c
#define OSTMR		0x10
#define OST1DFR		0x14
#define OST1CNT		0x18
#define OST2CNTH	0x1C
#define OST2CNTL	0x20
#define OSTCNT2HBUF 0x24

#define TER_OSTEN			(1 << 15)

#define OSTCSR_CNT_MD			(1 << 15)
#define OSTCSR_SD			(1 << 9)

#define OSTCSR_PRESCALE1 		(0)
#define OSTCSR_PRESCALE2 		(3)
#define OST_DIV_1				0
#define OST_DIV_4				1
#define OST_DIV_16				2
#define OSTCSR_PRESCALE(n, o)		(n << o)

#define OST2ENS (1 << 1)
#define OST1ENS (1 << 0)
#define OST2ENC (1 << 1)
#define OST1ENC (1 << 0)

#define OST2CLR (1 << 1)
#define OST1CLR (1 << 0)


#define OSTCSR_EXT_EN			(1 << 2)

#endif /* __OST_H__ */
