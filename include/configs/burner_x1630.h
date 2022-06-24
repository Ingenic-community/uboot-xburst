/*
 * Ingenic burner configuration
 *
 * Copyright (c) 2017  Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
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

#ifndef __CONFIG_BURNER_H__
#define __CONFIG_BURNER_H__

/**
 * Basic configuration(SOC, Cache, UART, DDR).
 */
#define CONFIG_MIPS32		/* MIPS32 CPU core */
#define CONFIG_CPU_XBURST
#define CONFIG_SYS_LITTLE_ENDIAN
#define CONFIG_X1630		/* X1630 SoC */
#define CONFIG_XBURST_TRAPS
#define CONFIG_SYS_HZ			1000		/* incrementer freq */

/**
 * PLL
 **/
#define CONFIG_SYS_APLL_FREQ            900000000       /*If APLL not use mast be set 0*/
#define CONFIG_SYS_APLL_MNOD            ((74 << 20) | (1 << 14) | (1 << 11) | (2<<5))
#define CONFIG_SYS_MPLL_FREQ            1000000000      /*If MPLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_MNOD            ((124 << 20) | (2 << 14) | (1 << 11) | (1<<5))

/**
 * CACHE
 **/
#define CONFIG_SYS_DCACHE_SIZE		32768
#define CONFIG_SYS_ICACHE_SIZE		32768
#define CONFIG_SYS_CACHELINE_SIZE	32

/**
 * DEBUG
 **/
#define CONFIG_SYS_UART_INDEX		1
#define CONFIG_BAUDRATE			115200

/**
 * CPU & DDR
 **/
#define CONFIG_SYS_CPU_FREQ		CONFIG_SYS_APLL_FREQ
#define CONFIG_SYS_MEM_FREQ		(500000000)
#define CONFIG_SYS_CPCCR_SEL		(2 << 30) | (1 << 28) | (2 << 26) | (2 << 24) \
						| ((8 - 1) << 16) | ((4 - 1) << 12) | ((4 - 1) << 8)	\
						| ((2 - 1) << 4) | ((1 - 1) << 0)
#define CONFIG_CPU_SEL_PLL		APLL
#define CONFIG_DDR_SEL_PLL		MPLL

#define CONFIG_DDR_INNOPHY
#define CONFIG_X1XXX_INNOPHY
#define CONFIG_DDR_TYPE_DDR2
#define CONFIG_DDR_PARAMS_CREATOR
#define CONFIG_DDR_HOST_CC
#define CONFIG_DDR_CS0			1	/* 1-connected, 0-disconnected */
#define CONFIG_DDR_CS1			0	/* 1-connected, 0-disconnected */
#define CONFIG_DDR_DW32			0	/* 1-32bit-width, 0-16bit-width */
#define CONFIG_DDRC_CTRL_PDT DDRC_CTRL_PDT_128
#define CONFIG_DDR2_M14D5121632A
#define DDR2_CHIP_DRIVER_OUT_STRENGTH 0
#define CONFIG_DDR_PHY_IMPEDANCE 40000
#define CONFIG_DDR_PHY_ODT_IMPEDANCE 50000

/**
 * Environment
 **/
#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_SIZE 512

/**
 * Boot command definitions.
 */
#define CONFIG_BOOTDELAY 0
#define CONFIG_BOOTCOMMAND "burn"

/**
 * Drivers configuration.
 */
/* I2C */
#define CONFIG_INGENIC_SOFT_I2C

/* SFC */
#define CONFIG_MTD_SFCNOR
#define CONFIG_MTD_SFCNAND
#define CONFIG_CMD_SFCNAND
#define CONFIG_CMD_SFC_NOR
#define CONFIG_JZ_SFC
#define CONFIG_JZ_SFC_NOR
#define CONFIG_SFC_NOR_RATE    		150000000
#define CONFIG_SFC_NAND_RATE	    150000000
#define CONFIG_SPL_VERSION_OFFSET	16
#define CONFIG_SPIFLASH_PART_OFFSET	(0x6800)


/*
 *  SPINAND MAC SN : the product of customer add partition of sequence code.
 */
#define CONFIG_JZ_SPINAND_MAC
#define CONFIG_MAC_SIZE	    (1 * 1024 * 1024)
#define CONFIG_JZ_SPINAND_SN
#define CONFIG_SN_SIZE	    (1 * 1024 * 1024)


/* MMC */
#define CONFIG_GENERIC_MMC		1
#define CONFIG_MMC			1
#define CONFIG_JZ_MMC 			1
#define CONFIG_JZ_MMC_MSC0
#define CONFIG_CMD_MMC			/* MMC/SD support*/

/* GPIO */
#define CONFIG_JZ_GPIO

/**
 * Command configuration.
 */
#define CONFIG_CMD_CONSOLE      /* coninfo                      */
#define CONFIG_CMD_MEMORY	/* md mm nm mw cp cmp crc base loop mtest */
#define CONFIG_CMD_RUN		/* run command in env variable	*/
#define CONFIG_CMD_BURN		/* ingenic usb burner support  */


/**
 * Miscellaneous configurable options
 */
#define CONFIG_RBTREE
#define CONFIG_SYS_NO_FLASH

#define CONFIG_SYS_MAXARGS 16
#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_PROMPT CONFIG_SYS_BOARD "# "
#define CONFIG_SYS_CBSIZE 1024 /* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)

#define CONFIG_SYS_MONITOR_LEN		(1024 * 1024)
#define CONFIG_SYS_MALLOC_LEN		(16 * 1024 * 1024)
#define CONFIG_SYS_BOOTPARAMS_LEN	(128 * 1024)

#define CONFIG_SYS_SDRAM_BASE		0x80000000 /* cached (KSEG0) address */
#define CONFIG_SYS_SDRAM_MAX_TOP	0x90000000 /* don't run into IO space */
#define CONFIG_SYS_INIT_SP_OFFSET	0x400000
#define CONFIG_SYS_MEMTEST_START	0x80000000
#define CONFIG_SYS_MEMTEST_END		0x84000000

#define CONFIG_SYS_TEXT_BASE		0x80100000
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE

/**
 * SPL configuration
 */
#define CONFIG_SPL
#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_NO_CPU_SUPPORT_CODE
#define CONFIG_SPL_START_S_PATH		"$(CPUDIR)/$(SOC)"
#define CONFIG_SPL_PAD_TO		26624 /* equal to spl max size in M200 */
#define CONFIG_SPL_LDSCRIPT		"$(CPUDIR)/$(SOC)/u-boot-spl.lds"


#define CONFIG_SPL_GINFO_BASE		0x80001000
#define CONFIG_SPL_GINFO_SIZE		0x800
#define CONFIG_SPL_BOARD_INIT
#define CONFIG_SPL_LIBGENERIC_SUPPORT
#define CONFIG_SPL_GPIO_SUPPORT
#define CONFIG_SPL_TEXT_BASE		0x80001800
#define CONFIG_SPL_MAX_SIZE		(26 * 1024)
#define CONFIG_SPL_SERIAL_SUPPORT

/*
 *MTD
 */
#define CONFIG_CMD_NAND
#define CONFIG_SYS_MAX_NAND_DEVICE      1
#define CONFIG_SYS_NAND_BASE            0
#define CONFIG_SYS_NAND_SELF_INIT
#define CONFIG_CMD_UBI
#define CONFIG_CMD_UBIFS
#define CONFIG_LZO
#define CONFIG_MTD_PARTITIONS
#define CONFIG_CMD_MTDPARTS
#define CONFIG_SYS_LOAD_ADDR		0x88000000

#define CONFIG_CMD_JFFS2
#define CONFIG_JFFS2_NAND	1
#define CONFIG_JFFS2_DEV	"nand0"
#define CONFIG_JFFS2_PART_OFFSET	0x800000	/*jffs2 offset*/
#define CONFIG_JFFS2_PART_SIZE		0x7800000	/* jffs2  part size*/

#define MTDIDS_DEFAULT                  "nand0=nand"
#define CONFIG_MTD_DEVICE
#define CONFIG_CMD_MTDPARTS

/**
 * Burner
 */
#define CONFIG_USB_GADGET_DUALSPEED
#define CONFIG_BURNER
#define CONFIG_USB_GADGET
#define CONFIG_USB_JZ_BURNER_GADGET
#define CONFIG_JZ_VERDOR_BURN_EXTPOL
#define	CONFIG_JZ_VERDOR_BURN_FUNCTION
#define CONFIG_USB_JZ_DWC2_UDC_V1_1
#define CONFIG_USB_SELF_POLLING
#define CONFIG_USB_PRODUCT_ID           0xc309
#define CONFIG_USB_VENDOR_ID            0xa108
#define CONFIG_BURNER_CPU_INFO          "BOOTx1630"
#define CONFIG_USB_GADGET_VBUS_DRAW     500
#define CONFIG_BURNER_PRIDUCT_INFO      "X1630 USB Boot Device"

/*RTC*/
#define CONFIG_CMD_DATE
#define CONFIG_RTC_JZ47XX
#endif /* __CONFIG_BURNER_H__ */
