/*
 * X1501 Pico SoM configuration
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


/* Tab is 8 spaces! */

#ifndef __CONFIG_X1501_PICO_H__
#define __CONFIG_X1501_PICO_H__

/**
 * Basic configuration(SOC, Cache, UART, DDR).
 */
#define CONFIG_MIPS32R2
#define CONFIG_CPU_XBURST
#define CONFIG_SYS_LITTLE_ENDIAN
#define CONFIG_X1000


#define CONFIG_SYS_APLL_FREQ			1008000000
#define CONFIG_SYS_MPLL_FREQ			600000000
#define CONFIG_CPU_SEL_PLL			APLL
#define CONFIG_DDR_SEL_PLL			MPLL
#define CONFIG_SYS_CPU_FREQ			CONFIG_SYS_APLL_FREQ
#define CONFIG_SYS_MEM_FREQ			200000000

#define CONFIG_SYS_EXTAL			24000000	/* EXTAL freq: 24 MHz */
#define CONFIG_SYS_HZ				1000		/* incrementer freq */

#define CONFIG_SYS_DCACHE_SIZE			16384
#define CONFIG_SYS_ICACHE_SIZE			16384
#define CONFIG_SYS_CACHELINE_SIZE		32

#define CONFIG_SYS_UART_INDEX			2
#define CONFIG_SYS_UART2_PA
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

#define CONFIG_MDDR_PMD606416ATR_5IN


#ifdef CONFIG_SPL_SFC_NOR
	#define CONFIG_SPL_SFC_SUPPORT
	#define CONFIG_SPL_VERSION	1
#endif


/*
 * Reimu: Expected partition layout:
 * 0x000000 - 0x019fff (104 KiB)   uboot
 * 0x01a000 - 0x119fff (1 MiB)     kernel
 * 0x11a000 - 0x1fffff (920 KiB)   rootfs
 *
 * You must enable 'erase all' and flash all partitions at once in the cloner software.
*/


/**
 * Boot arguments definitions.
 */
#define CONFIG_DDR_8M			8

#define BOOTARGS_COMMON "console=ttyS2,115200n8 mem=8M@0x0 loglevel=7 "
#define	CONFIG_BOOTARGS BOOTARGS_COMMON "devtmpfs.mount=1 init=/linuxrc rootfstype=squashfs root=/dev/mtdblock2 ro"
#define CONFIG_BOOTCOMMAND "sfcnor read 0x01a000 0x100000 0x80000000 ;bootm 0x80000000"



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
#define CONFIG_CMD_MISC			/* Misc functions like sleep etc*/
#define CONFIG_CMD_RUN			/* run command in env variable	*/
#define CONFIG_CMD_SAVEENV		/* saveenv			*/

#ifndef CONFIG_SPL_BUILD
	#define CONFIG_USE_ARCH_MEMSET
	#define CONFIG_USE_ARCH_MEMCPY
#endif


/**
 * Serial download configuration
 */
#define CONFIG_LOADS_ECHO				1	/* echo on for serial download */

/**
 * Miscellaneous configurable options
 */
#undef CONFIG_GZIP
#define CONFIG_LZMA

#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_SYS_NO_FLASH
#define CONFIG_SYS_FLASH_BASE				0 /* init flash_base as 0 */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_MISC_INIT_R				1

#define CONFIG_BOOTP_MASK				(CONFIG_BOOTP_DEFAUL)

#define CONFIG_SYS_MAXARGS				16


#define CONFIG_SYS_PROMPT 				CONFIG_SYS_BOARD "-sfcnor# "


#define CONFIG_SYS_CBSIZE				1024
#define CONFIG_SYS_PBSIZE				(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)


#define CONFIG_SYS_MONITOR_LEN				(512 << 10)


#define CONFIG_SYS_MALLOC_LEN				(3 * 1024 * 1024)	/* 3MB for bzip2 */
#define CONFIG_SYS_BOOTPARAMS_LEN			(128 * 1024)

#define CONFIG_SYS_SDRAM_BASE				0x80000000 /* cached (KSEG0) address */
#define CONFIG_SYS_SDRAM_MAX_TOP			0x90000000 /* don't run into IO space */
#define CONFIG_SYS_INIT_SP_OFFSET			0x400000
#define CONFIG_SYS_LOAD_ADDR				0x88000000
#define CONFIG_SYS_MEMTEST_START			0x80000000
#define CONFIG_SYS_MEMTEST_END				0x88000000
#define CONFIG_SYS_TEXT_BASE				0x80100000
#define CONFIG_SYS_MONITOR_BASE				CONFIG_SYS_TEXT_BASE

/**
 * Environment
 */

#define CONFIG_ENV_SIZE					4096

/* Reimu: We expect the uboot-with-spl.bin to be smaller than 100KB, and the uboot partition is 104KB in total */
#define CONFIG_ENV_OFFSET				(100 * 1024)
#define CONFIG_CMD_SAVEENV


/**
 * SPL configuration
 */
#define CONFIG_SPL
#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_NO_CPU_SUPPORT_CODE
#define CONFIG_SPL_START_S_PATH				"$(CPUDIR)/$(SOC)"
#define CONFIG_SPL_LDSCRIPT				"$(CPUDIR)/$(SOC)/u-boot-spl.lds"
#define CONFIG_SYS_U_BOOT_MAX_SIZE_SECTORS		0x200 /* 256 KB */
#define CONFIG_SPL_SERIAL_SUPPORT
#define CONFIG_SPL_GPIO_SUPPORT
#define CONFIG_SPL_BOARD_INIT
#define CONFIG_SPL_LIBGENERIC_SUPPORT

#define CONFIG_UBOOT_OFFSET				(4<<12)
#define CONFIG_JZ_SFC_PA_6BIT
#define CONFIG_SPI_SPL_CHECK
#define CONFIG_SPL_TEXT_BASE				0xf4001000
#define CONFIG_SPL_MAX_SIZE				(12 * 1024)
#define CONFIG_SPL_PAD_TO				16384
#define CONFIG_CMD_SFC_NOR


#ifdef CONFIG_CMD_SFC_NOR
	#define CONFIG_JZ_SFC
	#define CONFIG_JZ_SFC_NOR
	#define CONFIG_SFC_QUAD
	#define CONFIG_SFC_NOR_RATE			150000000
#endif

#ifdef CONFIG_JZ_SFC_NOR
	#define CONFIG_SPIFLASH_PART_OFFSET		0x3c00
	#define CONFIG_SPI_NORFLASH_PART_OFFSET		0x3c74
	#define CONFIG_NOR_MAJOR_VERSION_NUMBER		1
	#define CONFIG_NOR_MINOR_VERSION_NUMBER		0
	#define CONFIG_NOR_REVERSION_NUMBER		0
	#define CONFIG_NOR_VERSION			(CONFIG_NOR_MAJOR_VERSION_NUMBER | (CONFIG_NOR_MINOR_VERSION_NUMBER << 8) | (CONFIG_NOR_REVERSION_NUMBER <<16))
#endif


#endif /* __CONFIG_X1501_PICO_H__ */
