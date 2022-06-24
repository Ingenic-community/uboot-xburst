/*
 * X2000 cpm definitions
 *
 * Copyright (c) 2016 Ingenic Semiconductor Co.,Ltd
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

#ifndef __CPM_H__
#define __CPM_H__

#include <asm/arch/base.h>

#define CPM_CPCCR		(0x00)
#define CPM_CPCSR		(0xD4)
#define CPM_DDRCDR		(0x2C)
#define CPM_MACCDR		(0x54)
#define CPM_I2S0CDR		(0x60)
#define CPM_I2S0CDR1		(0x70)
#define CPM_I2S1CDR		(0x80)
#define CPM_I2S1CDR1		(0x84)
#define CPM_I2S2CDR		(0xE0)
#define CPM_I2S2CDR1		(0x88)
#define CPM_I2S3CDR		(0x8C)
#define CPM_I2S3CDR1		(0xA0)
#define CPM_AUDIOCR		(0xAC)
#define CPM_LPCDR		(0x64)
#define CPM_MSC0CDR		(0x68)
#define CPM_MSC1CDR		(0xA4)
#define CPM_SSICDR		(0x74)
#define CPM_CIMCDR		(0x7C)
#define CPM_PWMCDR		(0x5C)
#define CPM_MACPHYC		(0xE8)
#define CPM_INTR		(0xB0)
#define CPM_INTRE		(0xB4)
#define CPM_DRCG		(0xD0)
#define CPM_CPSPR		(0x34)
#define CPM_CPSPPR		(0x38)
#define CPM_USBPCR		(0x3C)
#define CPM_USBRDT		(0x40)
#define CPM_USBVBFIL		(0x44)
#define CPM_USBPCR1		(0x48)
#define CPM_CPPCR		(0x0C)
#define CPM_CPAPCR		(0x10)
#define CPM_CPMPCR		(0x14)
#define CPM_CPEPCR		(0x58)

#define CPM_LCR			(0x04)
#define CPM_PSWC0ST		(0x90)
#define CPM_PSWC1ST		(0x94)
#define CPM_PSWC2ST		(0x98)
#define CPM_PSWC3ST		(0x9C)
#define CPM_CLKGR		(0x20)
#define CPM_CLKGR1		(0x28)
#define CPM_MESTSEL		(0xEC)
#define CPM_SRBC		(0xC4)
#define CPM_EXCLK_DS		(0xE0)
#define CPM_POC			(0xE4)
#define CPM_MSCR		(0xF0)
#define CPM_MPDCR		(0xF8)
#define CPM_SLBC		(0xC8)
#define CPM_SLPC		(0xCC)
#define CPM_OPCR		(0x24)
#define CPM_RSR			(0x08)


#define LCR_LPM_MASK		(0x3)
#define LCR_LPM_SLEEP		(0x1)

#define CPM_CLKGR_DDR		(1 << 31)
#define CPM_CLKGR_CPU1		(1 << 30)
#define CPM_CLKGR_AHB0		(1 << 29)
#define CPM_CLKGR_APB0		(1 << 28)
#define CPM_CLKGR_RTC		(1 << 27)
#define CPM_CLKGR_SSI1		(1 << 26)
#define CPM_CLKGR_MAC		(1 << 25)
#define CPM_CLKGR_AES		(1 << 24)
#define CPM_CLKGR_LCD		(1 << 23)
#define CPM_CLKGR_CIM		(1 << 22)
#define CPM_CLKGR_PDMA		(1 << 21)
#define CPM_CLKGR_OST		(1 << 20)
#define CPM_CLKGR_SSI0		(1 << 19)
#define CPM_CLKGR_TCU		(1 << 18)
#define CPM_CLKGR_DTRNG		(1 << 17)
#define CPM_CLKGR_UART2		(1 << 16)
#define CPM_CLKGR_UART1		(1 << 15)
#define CPM_CLKGR_UART0		(1 << 14)
#define CPM_CLKGR_SADC		(1 << 13)
#define CPM_CLKGR_JPEG		(1 << 12)
#define CPM_CLKGR_AUDIO		(1 << 11)
#define CPM_CLKGR_I2C3	        (1 << 10)
#define CPM_CLKGR_I2C2		(1 << 9)
#define CPM_CLKGR_I2C1		(1 << 8)
#define CPM_CLKGR_I2C0		(1 << 7)
#define CPM_CLKGR_SCC		(1 << 6)
#define CPM_CLKGR_MSC1		(1 << 5)
#define CPM_CLKGR_MSC0		(1 << 4)
#define CPM_CLKGR_OTG		(1 << 3)
#define CPM_CLKGR_SFC		(1 << 2)
#define CPM_CLKGR_EFUSE		(1 << 1)
#define CPM_CLKGR_NEMC		(1 << 0)

#define CPM_CLKGR_CPU		(1 << 17)
#define CPM_CLKGR_INTC		(1 << 16)
#define CPM_CLKGR_GPIO		(1 << 15)
#define CPM_CLKGR_SPDIF		(1 << 14)
#define CPM_CLKGR_DMIC		(1 << 13)
#define CPM_CLKGR_PCM		(1 << 12)
#define CPM_CLKGR_I2S3		(1 << 11)
#define CPM_CLKGR_I2S2		(1 << 10)
#define CPM_CLKGR_I2S1		(1 << 9)
#define CPM_CLKGR_I2S0		(1 << 8)
#define CPM_CLKGR_ROT		(1 << 7)
#define CPM_CLKGR_HASH		(1 << 6)
#define CPM_CLKGR_PWM5		(1 << 5)
#define CPM_CLKGR_UART5		(1 << 4)
#define CPM_CLKGR_UART4		(1 << 3)
#define CPM_CLKGR_UART3		(1 << 2)
#define CPM_CLKGR_I2C5		(1 << 1)
#define CPM_CLKGR_I2C4		(1 << 0)

#define cpm_inl(off)		readl(CPM_BASE + (off))
#define cpm_outl(val,off)	writel(val,CPM_BASE + (off))
#define cpm_clear_bit(val,off)	do{cpm_outl((cpm_inl(off) & ~(1<<(val))),off);}while(0)
#define cpm_set_bit(val,off)	do{cpm_outl((cpm_inl(off) |  (1<<val)),off);}while(0)
#define cpm_test_bit(val,off)	(cpm_inl(off) & (0x1<<val))

/* CPM scratch pad protected register(CPSPPR) */
#define CPSPPR_CPSPR_WRITABLE   (0x00005a5a)
#define RECOVERY_SIGNATURE      (0x1a1a)        /* means "RECY" */
#define RECOVERY_SIGNATURE_SEC  0x800           /* means "RECY" */
#define FASTBOOT_SIGNATURE      (0x0666)        /* means "FASTBOOT" */

#define cpm_get_scrpad()        readl(CPM_BASE + CPM_CPSPR)
#define cpm_set_scrpad(data)                    \
do {                                            \
	volatile int i = 0x3fff;                \
	writel(0x00005a5a,CPM_BASE + CPM_CPSPPR);		\
	while(i--);				\
	writel(data,CPM_BASE + CPM_CPSPR);			\
	writel(0x0000a5a5,CPM_BASE + CPM_CPSPPR);      	\
} while (0)

#endif /* __CPM_H__ */
