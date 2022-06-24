/*
 * Ingenic burner configuration
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

#ifndef __CONFIG_BURNER_H__
#define __CONFIG_BURNER_H__

/**
 * Basic configuration(SOC, Cache, UART, DDR).
 */
#define CONFIG_MIPS32		/* MIPS32 CPU core */
#define CONFIG_CPU_XBURST
#define CONFIG_SYS_LITTLE_ENDIAN
#define CONFIG_X1000		/* x1000 SoC */
#define CONFIG_XBURST_TRAPS
/*#define CONFIG_FPGA*/

#define CONFIG_SYS_MPLL_FREQ		0
#define CONFIG_SYS_APLL_FREQ		0
#define CONFIG_CPU_SEL_PLL		APLL
#define CONFIG_DDR_SEL_PLL		MPLL
#define CONFIG_SYS_MEM_FREQ		0
#define CONFIG_SYS_CPU_FREQ		0

#define CONFIG_SYS_EXTAL		24000000	/* EXTAL freq: 24 MHz */
#define CONFIG_SYS_HZ			1000 /* incrementer freq */

#define CONFIG_SYS_DCACHE_SIZE		16384
#define CONFIG_SYS_ICACHE_SIZE		16384
#define CONFIG_SYS_CACHELINE_SIZE	32

#define CONFIG_SYS_UART_INDEX          0
#define CONFIG_BAUDRATE                        115200

#define CONFIG_DDR_TYPE_VARIABLE
/*#define CONFIG_DWC_DEBUG*/
/*#define CONFIG_DWC_DEBUG*/
/* #define CONFIG_DDR_TEST_CPU */
/* #define CONFIG_DDR_TEST */
/* #define CONFIG_DDR_TEST_DATALINE */
/* #define CONFIG_DDR_TEST_ADDRLINE */

#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_SIZE 512
/**
 * Boot arguments definitions.
 */
/*#define BOOTARGS_COMMON "console=ttyS3,115200 mem=256M@0x0 mem=256M@0x30000000"*/
#define BOOTARGS_COMMON "console=ttyS2,115200n8 mem=32M@0x0 loglevel=7 "
#define	CONFIG_BOOTARGS BOOTARGS_COMMON "ip=off init=/linuxrc rootfstype=jffs2 root=/dev/mtdblock2 rw"

#define CONFIG_BOOTDELAY 0
#define CONFIG_BOOTCOMMAND "burn"

/**
 * Drivers configuration.
 */
#define CONFIG_JZ_GPIO
#define CONFIG_INGENIC_SOFT_I2C

/* SPI */
#define CONFIG_SFC_RATE			150000000
#define CONFIG_CMD_SFC_NOR
#ifdef CONFIG_CMD_SFC_NOR
#define CONFIG_JZ_SFC
#define CONFIG_JZ_SFC_NOR
#define CONFIG_SFC_NOR_RATE		CONFIG_SFC_RATE
#define CONFIG_SPL_VERSION_OFFSET	16
#endif


/* MMC */
#define CONFIG_GENERIC_MMC		1
#define CONFIG_MMC			1
#define CONFIG_JZ_MMC 1
#define CONFIG_JZ_MMC_MSC0 1

/* PMU */
#define CONFIG_REGULATOR
#define CONFIG_PMU_ACT8600

/* GPIO */
#define CONFIG_JZ_GPIO

/* CLK CGU */
#define  CGU_CLK_SRC {				\
		{OTG, EXCLK},			\
		{LCD, MPLL},			\
		{MSC, MPLL},			\
		{SFC, MPLL},			\
		{CIM, MPLL},			\
		{PCM, MPLL},			\
		{I2S, EXCLK},			\
		{SRC_EOF,SRC_EOF}		\
	}

/**
 * Command configuration.
 */
#define CONFIG_CMD_BOOTD	/* bootd			*/
#define CONFIG_CMD_CONSOLE	/* coninfo			*/
#define CONFIG_CMD_ECHO		/* echo arguments		*/
#define CONFIG_CMD_EXT4 	/* ext4 support			*/
#define CONFIG_CMD_FAT		/* FAT support			*/
#define CONFIG_CMD_LOADB	/* loadb			*/
#define CONFIG_CMD_LOADS	/* loads			*/
#define CONFIG_CMD_MEMORY	/* md mm nm mw cp cmp crc base loop mtest */
#define CONFIG_CMD_MISC		/* Misc functions like sleep etc*/
#define CONFIG_CMD_MMC		/* MMC/SD support			*/
#define CONFIG_CMD_RUN		/* run command in env variable	*/
#define CONFIG_CMD_SETGETDCR	/* DCR support on 4xx		*/
#define CONFIG_CMD_SOURCE	/* "source" command support	*/
#define CONFIG_CMD_BURN		/*ingenic usb burner support*/
#define CONFIG_CMD_EFUSE	/*efuse*/

#ifdef CONFIG_CMD_EFUSE
#define	CONFIG_X1000_EFUSE
#define	CONFIG_JZ_EFUSE
#define CONFIG_EFUSE_GPIO	GPIO_PB(27)
#define CONFIG_EFUSE_LEVEL	0
#endif

/**
 * Serial download configuration
 */
#define CONFIG_LOADS_ECHO	1	/* echo on for serial download */

/**
 * Miscellaneous configurable options
 */
#define CONFIG_DOS_PARTITION

#define CONFIG_LZO
#define CONFIG_RBTREE

#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_SYS_NO_FLASH
#define CONFIG_SYS_FLASH_BASE	0 /* init flash_base as 0 */
#define CONFIG_MISC_INIT_R 1

#define CONFIG_BOOTP_MASK	(CONFIG_BOOTP_DEFAUL)

#define CONFIG_SYS_MAXARGS 16
#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_PROMPT CONFIG_SYS_BOARD "# "
#define CONFIG_SYS_CBSIZE 1024 /* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)

#define CONFIG_SYS_MONITOR_LEN		(1024 * 1024)
#ifndef CONFIG_FPGA
#define CONFIG_SYS_MALLOC_LEN		(16 * 1024 * 1024)
#else
#define CONFIG_SYS_MALLOC_LEN		(16 * 1024 * 1024)
#endif
#define CONFIG_SYS_BOOTPARAMS_LEN	(128 * 1024)

#define CONFIG_SYS_SDRAM_BASE		0x80000000 /* cached (KSEG0) address */
#define CONFIG_SYS_SDRAM_MAX_TOP	0x90000000 /* don't run into IO space */
#define CONFIG_SYS_INIT_SP_OFFSET	0x400000
#define CONFIG_SYS_LOAD_ADDR		0x88000000
#define CONFIG_SYS_MEMTEST_START	0x80000000
#define CONFIG_SYS_MEMTEST_END		0x88000000

#define CONFIG_SYS_TEXT_BASE		0x80100000
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE

/**
 * SPL configuration
 */
#define CONFIG_SPL
#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_PAD_TO		16384

#define CONFIG_SPL_NO_CPU_SUPPORT_CODE
#define CONFIG_SPL_START_S_PATH		"$(CPUDIR)/$(SOC)"
#define CONFIG_SPL_LDSCRIPT		"$(TOPDIR)/board/$(BOARDDIR)/u-boot-spl.lds"

#define CONFIG_SPL_GINFO_BASE		0xf4001000
#define CONFIG_SPL_GINFO_SIZE		0x800

#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR	0x20 /* 16KB offset */
#define CONFIG_SYS_U_BOOT_MAX_SIZE_SECTORS	0x400 /* 512 KB */
#define CONFIG_SPL_LIBGENERIC_SUPPORT
#define CONFIG_SPL_GPIO_SUPPORT

#define CONFIG_SPL_TEXT_BASE		0xf4001800
#define CONFIG_SPL_MAX_SIZE		((16 * 1024) - 0x1000)
#define CONFIG_SPL_SERIAL_SUPPORT
#define CONFIG_SPIFLASH_PART_OFFSET	0x3c00

#ifdef CONFIG_JZ_SFC_NOR
#define CONFIG_SPI_NORFLASH_PART_OFFSET	0x3c74
#define CONFIG_NOR_MAJOR_VERSION_NUMBER     1
#define CONFIG_NOR_MINOR_VERSION_NUMBER     0
#define CONFIG_NOR_REVERSION_NUMBER     0
#define CONFIG_NOR_VERSION     (CONFIG_NOR_MAJOR_VERSION_NUMBER | (CONFIG_NOR_MINOR_VERSION_NUMBER << 8) | (CONFIG_NOR_REVERSION_NUMBER << 16))
#endif
/*
 *MTD
 */
#ifdef CONFIG_MTD_SFCNAND
#define CONFIG_SFC_NAND_RATE	    CONFIG_SFC_RATE
#endif
#define CONFIG_CMD_NAND
#define CONFIG_SYS_MAX_NAND_DEVICE  1
#define CONFIG_SYS_NAND_BASE CONFIG_SSI_BASE
#define CONFIG_SYS_NAND_SELF_INIT
#define CONFIG_CMD_UBI
#define CONFIG_CMD_UBIFS
#define CONFIG_MTD_PARTITIONS
#define CONFIG_CMD_MTDPARTS

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
#ifdef CONFIG_CMD_BURN
#define CONFIG_USB_GADGET_DUALSPEED
#define CONFIG_BURNER
#define CONFIG_USB_GADGET
#define CONFIG_JZ_SCBOOT
#define CONFIG_USB_JZ_BURNER_GADGET
#define CONFIG_JZ_VERDOR_BURN_EXTPOL
#define CONFIG_JZ_VERDOR_BURN_EP_TEST
#define	CONFIG_JZ_VERDOR_BURN_FUNCTION
#define CONFIG_USB_JZ_DWC2_UDC_V1_1
#define CONFIG_USB_SELF_POLLING
#define CONFIG_USB_PRODUCT_ID  0x1000
#define CONFIG_USB_VENDOR_ID   0xa108
#define CONFIG_BURNER_CPU_INFO "BOOTx1000"
#define CONFIG_USB_GADGET_VBUS_DRAW 500
#define CONFIG_BURNER_PRIDUCT_INFO	"X1000 USB Boot Device"
#endif	/* !CONFIG_CMD_BURN */

#define CONFIG_CMD_DATE
#define CONFIG_RTC_JZ47XX
#endif /* __CONFIG_BURNER_H__ */
