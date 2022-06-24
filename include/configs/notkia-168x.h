/*
 * Notkia 168x configuration
 *
 * Copyright (c) 2013 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
 * Based on: include/configs/urboard.h
 *           Written by Paul Burton <paul.burton@imgtec.com>
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

#ifndef __CONFIG_NOTKIA_168X_H__
#define __CONFIG_NOTKIA_168X_H__

/**
 * Basic configuration(SOC, Cache, UART, DDR).
 */
#define CONFIG_MIPS32R2		/* MIPS32 CPU core */
#define CONFIG_CPU_XBURST
#define CONFIG_SYS_LITTLE_ENDIAN
#define CONFIG_X1000

#define CONFIG_SYS_APLL_FREQ		1200000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_FREQ		600000000	/*If MPLL not use mast be set 0*/
#define CONFIG_CPU_SEL_PLL			APLL
#define CONFIG_DDR_SEL_PLL			MPLL
#define CONFIG_SYS_CPU_FREQ			CONFIG_SYS_APLL_FREQ
#define CONFIG_SYS_MEM_FREQ			200000000

#define CONFIG_SYS_EXTAL			24000000	/* EXTAL freq: 24 MHz */
#define CONFIG_SYS_HZ				1000		/* incrementer freq */

#define CONFIG_SYS_DCACHE_SIZE		16384
#define CONFIG_SYS_ICACHE_SIZE		16384
#define CONFIG_SYS_CACHELINE_SIZE	32

#define CONFIG_SYS_UART_INDEX		2
#define CONFIG_SYS_UART2_PD
#define CONFIG_BAUDRATE				115200

#define CONFIG_DDR_PARAMS_CREATOR
#define CONFIG_DDR_HOST_CC
#define CONFIG_DDR_TYPE_LPDDR
#define CONFIG_DDR_CS0				1	/* 1-connected, 0-disconnected */
#define CONFIG_DDR_CS1				0	/* 1-connected, 0-disconnected */
#define CONFIG_DDR_DW32				0	/* 1-32bit-width, 0-16bit-width */
/*
  Output Drive Strength: Controls the output drive strength. Valid values are:
  000 = Full strength driver
  001 = Half strength driver
  110 = Quarter strength driver
  111 = Octant strength driver
  100 = Three-quarters strength driver
*/
#define CONFIG_DDR_DRIVER_STRENGTH	4

#define CONFIG_MDDR_JSD12164PAI_KGD	/*DDR 64M param file*/

/*pmu slp pin*/

#ifdef CONFIG_SPL_SFC_NOR
	#define CONFIG_SPL_SFC_SUPPORT
	#define CONFIG_SPL_VERSION	1
#endif

/**
 * Boot arguments definitions.
 */
#define CONFIG_DDR_64M      64	    /*DDR size 64M*/


#define CONFIG_BOOTARGS		"console=ttyS2,115200n8 mem=64M@0x0 loglevel=7 ip=off rootfstype=jffs2 root=/dev/mtdblock2 rw no_hash_pointers no_console_suspend devtmpfs.mount=1"

#define CONFIG_BOOTCOMMAND	"sfcnor read 0x40000 0x800000 0x82000000; bootm 0x82000000"



#define PARTITION_NUM 10

/**
 * Boot command definitions.
 */
#define CONFIG_BOOTDELAY 1

/* CLK CGU */
#define  CGU_CLK_SRC {			\
		{OTG, EXCLK},			\
		{LCD, MPLL},			\
		{MSC, MPLL},			\
		{SFC, MPLL},			\
		{CIM, MPLL},			\
		{PCM, MPLL},			\
		{I2S, EXCLK},			\
		{SRC_EOF,SRC_EOF}		\
	}

/* GPIO */
#define CONFIG_JZ_GPIO

/**
 * Command configuration.
 */
#define CONFIG_CMD_BOOTD		/* bootd			*/
#define CONFIG_CMD_CONSOLE		/* coninfo			*/
#define CONFIG_CMD_ECHO			/* echo arguments		*/
#define CONFIG_CMD_MEMORY		/* md mm nm mw cp cmp crc base loop mtest */
#define CONFIG_CMD_MISC			/* Misc functions like sleep etc*/
#define CONFIG_CMD_RUN			/* run command in env variable	*/
#define CONFIG_CMD_SAVEENV		/* saveenv			*/
#define CONFIG_CMD_SETGETDCR	/* DCR support on 4xx		*/
#define CONFIG_CMD_SOURCE		/* "source" command support	*/
#define CONFIG_CMD_GETTIME
#define CONFIG_CMD_UNZIP        /* unzip from memory to memory  */

#ifndef CONFIG_SPL_BUILD
	#define CONFIG_USE_ARCH_MEMSET
	#define CONFIG_USE_ARCH_MEMCPY
#endif


/* MMC */
#define CONFIG_CMD_MMC
#ifdef CONFIG_CMD_MMC
	#define CONFIG_GENERIC_MMC	1
	#define CONFIG_MMC			1
	#define CONFIG_JZ_MMC		1

	#ifdef CONFIG_JZ_MMC_MSC0
		#define CONFIG_JZ_MMC_SPLMSC		0
		#define CONFIG_JZ_MMC_MSC0_PA_4BIT	1
		#define CONFIG_MSC_DATA_WIDTH_4BIT
	#endif
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
#define CONFIG_ENV_OVERWRITE
#define CONFIG_MISC_INIT_R		1

#define CONFIG_BOOTP_MASK		(CONFIG_BOOTP_DEFAUL)

#define CONFIG_SYS_MAXARGS		16
#define CONFIG_SYS_LONGHELP

#if defined(CONFIG_SPL_JZMMC_SUPPORT)
	#if	defined(CONFIG_JZ_MMC_MSC0)
		#define CONFIG_SYS_PROMPT CONFIG_SYS_BOARD "-msc0# "
	#endif
#elif defined(CONFIG_SPL_SFC_SUPPORT)
	#if defined(CONFIG_SPL_SFC_NOR)
		#define CONFIG_SYS_PROMPT CONFIG_SYS_BOARD "-sfcnor# "
	#endif
#endif

#define CONFIG_SYS_CBSIZE 1024 /* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)

#define CONFIG_SUPPORT_EMMC_BOOT
#if defined(CONFIG_SUPPORT_EMMC_BOOT)
	#define CONFIG_SYS_MONITOR_LEN	(384 * 1024)
#else
	#define CONFIG_SYS_MONITOR_LEN	(512 << 10)
#endif

#define CONFIG_SYS_MALLOC_LEN		(8 * 1024 * 1024)
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
 * Environment
 */
#if defined(CONFIG_ENV_IS_IN_MMC)
	#define CONFIG_SYS_MMC_ENV_DEV	0
	#define CONFIG_ENV_SIZE			(32 << 10)
	#define CONFIG_ENV_OFFSET		(CONFIG_SYS_MONITOR_LEN + CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512)
#elif defined(CONFIG_ENV_IS_IN_SFC)
	#define CONFIG_ENV_SIZE			(4 << 10)
	#define CONFIG_ENV_OFFSET		0x3f000 /*write nor flash 252k address*/
	#define CONFIG_CMD_SAVEENV
#endif

/**
 * SPL configuration
 */
#define CONFIG_SPL
#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_NO_CPU_SUPPORT_CODE
#define CONFIG_SPL_START_S_PATH					"$(CPUDIR)/$(SOC)"
#define CONFIG_SPL_LDSCRIPT						"$(CPUDIR)/$(SOC)/u-boot-spl.lds"
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR	0x3A /* 12KB+17K offset */
#define CONFIG_SYS_U_BOOT_MAX_SIZE_SECTORS		0x200 /* 256 KB */
#define CONFIG_SPL_SERIAL_SUPPORT
#define CONFIG_SPL_GPIO_SUPPORT
#define CONFIG_SPL_BOARD_INIT
#define CONFIG_SPL_LIBGENERIC_SUPPORT
#if defined(CONFIG_SPL_JZMMC_SUPPORT)
	#define CONFIG_SPL_PAD_TO			12288		/* spl size */
	#define CONFIG_SPL_TEXT_BASE		0xf4001000
	#define CONFIG_SPL_MAX_SIZE			(12 * 1024)
#elif defined(CONFIG_SPL_SFC_SUPPORT)
	#define CONFIG_UBOOT_OFFSET			(4<<12)
	#define CONFIG_JZ_SFC_PA_6BIT
	#define CONFIG_SPI_SPL_CHECK
	#define CONFIG_SPL_TEXT_BASE		0xf4001000
	#define CONFIG_SPL_MAX_SIZE			(12 * 1024)
	#define CONFIG_SPL_PAD_TO			16384
	#define CONFIG_CMD_SFC_NOR
#endif

#ifdef CONFIG_CMD_SFC_NOR
	#define CONFIG_JZ_SFC
	#define CONFIG_JZ_SFC_NOR
	#define CONFIG_SFC_QUAD
	#define CONFIG_SFC_NOR_RATE			150000000
#endif

#ifdef CONFIG_JZ_SFC_NOR
	#define CONFIG_SPIFLASH_PART_OFFSET			0x3c00
	#define CONFIG_SPI_NORFLASH_PART_OFFSET     0x3c74
	#define CONFIG_NOR_MAJOR_VERSION_NUMBER     1
	#define CONFIG_NOR_MINOR_VERSION_NUMBER     0
	#define CONFIG_NOR_REVERSION_NUMBER     	0
	#define CONFIG_NOR_VERSION     (CONFIG_NOR_MAJOR_VERSION_NUMBER | (CONFIG_NOR_MINOR_VERSION_NUMBER << 8) | (CONFIG_NOR_REVERSION_NUMBER <<16))
#endif

/**
 * MBR configuration
 */
#define CONFIG_GPT_TABLE_PATH	"$(TOPDIR)/board/$(BOARDDIR)"

#endif /* __CONFIG_NOTKIA_168X_H__ */
