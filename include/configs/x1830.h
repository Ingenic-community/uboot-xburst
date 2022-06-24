/*
 * Ingenic audio configuration
 *
 * Copyright (c) 2017  Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
 * Based on: include/configs/urboard.h
 *           Written by Paul Burton <paul.burton@imgtec.com>
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
#ifndef __CONFIG_X1830_H__
#define __CONFIG_X1830_H__
/**
 * Basic configuration(SOC, Cache, UART, DDR).
 */
#define CONFIG_MIPS32		/* MIPS32 CPU core */
#define CONFIG_CPU_XBURST
#define CONFIG_SYS_LITTLE_ENDIAN
#define CONFIG_X1830		        /* X1830 SoC */
#define CONFIG_SYS_EXTAL		24000000	/* EXTAL freq: 24 MHz */
#define CONFIG_SYS_HZ			1000		/* incrementer freq */

#define DDR3_600M
/*#define DDR3_500M*/
/*#define DDR3_400M*/
/*#define DDR3_300M*/

/*#define CPU_900M*/
/*#define CPU_1200M*/
#define CPU_1536M

/**
 * PLL
 **/
#if defined(CPU_1536M)
#define CONFIG_SYS_APLL_FREQ		1536000000
#define CONFIG_SYS_APLL_MNOD		((95 << 20) | (2 << 14) | (0 << 11) | (1<<5))
#elif defined(CPU_1200M)
#define CONFIG_SYS_APLL_FREQ		1200000000
#define CONFIG_SYS_APLL_MNOD		((149 << 20) | (2 << 14) | (1 << 11) | (1<<5))
#elif defined(CPU_900M)
#define CONFIG_SYS_APLL_FREQ		900000000
#define CONFIG_SYS_APLL_MNOD		((74 << 20) | (1 << 14) | (1 << 11) | (2<<5))
#endif

#if defined(DDR3_600M) || defined(DDR3_400M)
#define CONFIG_SYS_MPLL_FREQ		1200000000
#define CONFIG_SYS_MPLL_MNOD		((149 << 20) | (2 << 14) | (1 << 11) | (1<<5))
#elif defined(DDR3_500M)
#define CONFIG_SYS_MPLL_FREQ		1000000000
#define CONFIG_SYS_MPLL_MNOD		((124 << 20) | (2 << 14) | (1 << 11) | (1<<5))
#elif defined(DDR3_300M)
#define CONFIG_SYS_MPLL_FREQ		600000000
#define CONFIG_SYS_MPLL_MNOD		((99 << 20) | (1 << 14) | (2 << 11) | (2<<5))
#endif
#define CONFIG_SYS_VPLL_FREQ		1200000000	/*If VPLL not use mast be set 0*/
#define CONFIG_SYS_EPLL_FREQ        	1200000000

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
#define CONFIG_BAUDRATE_STR             "115200"

/**
 * CPU & DDR
 **/
#define CONFIG_SYS_CPU_FREQ		CONFIG_SYS_APLL_FREQ

#ifndef DDR3_400M
#define CONFIG_SYS_MEM_FREQ		((CONFIG_SYS_MPLL_FREQ) >> 1)
#else
#define CONFIG_SYS_MEM_FREQ		((CONFIG_SYS_MPLL_FREQ) / 3)
#endif
#ifndef DDR3_500M
#define CONFIG_SYS_CPCCR_SEL		((2 << 30) | (1 << 28) | (2 << 26) | (2 << 24) \
						| (11 << 16) | (5 << 12) | (5 << 8)	\
						| (1 << 4) | (0 << 0))
#else
#define CONFIG_SYS_CPCCR_SEL		((2 << 30) | (1 << 28) | (2 << 26) | (2 << 24) \
						| (7 << 16) | (3 << 12) | (3 << 8)	\
						| (1 << 4) | (0 << 0))
#endif

#define CONFIG_CPU_SEL_PLL		APLL
#define CONFIG_DDR_SEL_PLL		MPLL

#define CONFIG_MULT_DDR_PARAMS_CREATOR

#ifndef CONFIG_MULT_DDR_PARAMS_CREATOR

#define CONFIG_DDR_TYPE_DDR3
#define CONFIG_DDR_PARAMS_CREATOR
#define CONFIG_DDR_HOST_CC
#define CONFIG_DDR_CS0			1	/* 1-connected, 0-disconnected */
#define CONFIG_DDR_CS1			0	/* 1-connected, 0-disconnected */
#define CONFIG_DDR_DW32			0	/* 1-32bit-width, 0-16bit-width */
#define CONFIG_DDRC_CTRL_PDT DDRC_CTRL_PDT_128
#define CONFIG_X1XXX_INNOPHY
#define CONFIG_DDR3_M15T1G1664A_2C
#define CONFIG_DDR_CHIP_ODT_VAL 1
#define CONFIG_DDR_PHY_IMPEDANCE 40000
#define CONFIG_DDR_PHY_ODT_IMPEDANCE 50000
#define CONFIG_DDR_AUTO_REFRESH_TEST

#else
#define CONFIG_DDR_HOST_CC
#define CONFIG_DDR_TYPE_DDR2
#define CONFIG_DDR_TYPE_DDR3
#define CONFIG_DDR_CS0			1	/* 1-connected, 0-disconnected */
#define CONFIG_DDR_CS1			0	/* 1-connected, 0-disconnected */
#define CONFIG_DDR_DW32			0	/* 1-32bit-width, 0-16bit-width */
#define CONFIG_DDRC_CTRL_PDT DDRC_CTRL_PDT_128
#define CONFIG_X1XXX_INNOPHY
#define CONFIG_DDR_CHIP_ODT_VAL 1
#define CONFIG_DDR_PHY_IMPEDANCE 40000
#define CONFIG_DDR_PHY_ODT_IMPEDANCE 50000
#define CONFIG_DDR_AUTO_REFRESH_TEST

#define CONFIG_DDR3_DFLAGS 	"-DCONFIG_DDR3_M15T1G1664A_2C"
#define CONFIG_DDR2_DFLAGS	"-DCONFIG_DDR2_M14D1G1664A"
#define CONFIG_LPDDR2_DFLAGS	""
#define CONFIG_LPDDR3_DFLAGS	""
#define CONFIG_LPDDR_DFLAGS	""


#endif
/*#define CONFIG_DDR_TEST*/
/*#define CONFIG_DDR_TEST_CPU*/

/**
 * Command configuration.
 */
#define CONFIG_CMD_BOOTD	/* bootd			*/
#define CONFIG_CMD_SAVEENV	/* saveenv			*/
#define CONFIG_CMD_CONSOLE	/* coninfo			*/
#define CONFIG_CMD_MEMORY	/* md mm nm mw cp cmp crc base loop mtest */
#define CONFIG_CMD_RUN		/* run command in env variable	*/

/**
 * Boot arguments & command definitions.
 */
#define BOOTARGS_COMMON "console=ttyS1," CONFIG_BAUDRATE_STR "n8 mem=128M@0x0"
#define CONFIG_BOOTDELAY 1
#ifdef CONFIG_SPL_JZMMC_SUPPORT
  #define CONFIG_BOOTARGS BOOTARGS_COMMON " init=/linuxrc root=/dev/mmcblk0p3 rw rootdelay=1"
  #define CONFIG_BOOTCOMMAND "mmc read 0x80600000 0x1800 0x3000; bootm 0x80600000"
#elif defined(CONFIG_MTD_SFCNOR)
  #define CONFIG_BOOTCOMMAND "sfcnor read 0x40000 0x280000 0x80600000 ;bootm 0x80600000"
  #define CONFIG_BOOTARGS BOOTARGS_COMMON " init=/linuxrc rootfstype=jffs2 root=/dev/mtdblock2 rw mtdparts=jz_sfc:256k(boot),2560k(kernel),12800k(root)"
#elif defined(CONFIG_MTD_SFCNAND)
  #define CONFIG_BOOTARGS BOOTARGS_COMMON " ip=off init=/linuxrc ubi.mtd=2 root=ubi0:rootfs rootfstype=ubifs ubi.mtd=3 rw"
  #define CONFIG_BOOTCOMMAND "sfcnand read 0x100000 0x400000 0x80600000 ;bootm 0x80600000"
#endif /* CONFIG_SFC_NOR */

#ifdef CONFIG_SPL_OS_BOOT
  #if defined(CONFIG_SPL_SFC_NAND)
    #define CONFIG_SPL_BOOTARGS BOOTARGS_COMMON " ip=off init=/linuxrc ubi.mtd=2 root=ubi0:rootfs ubi.mtd=3 rootfstype=ubifs ro"
  #else
	#error "not support config"
  #endif
  #if defined(CONFIG_OTA_VERSION20)
    #undef CONFIG_SPL_BOOTARGS
    #define CONFIG_OTA_AUDIO_VERSION20
    #define CONFIG_PAT_NV_NAME		"nv"
    #define CONFIG_PAT_RECOVERY_NAME	"recovery"
    #if defined(CONFIG_SPL_SFC_NAND)
      #define CONFIG_SPL_BOOTARGS	BOOTARGS_COMMON " ip=off init=/linuxrc ubi.mtd=4 root=ubi0:system ubi.mtd=5 rootfstype=ubifs ro"
      #define CONFIG_SPL_OTA_BOOTARGS   BOOTARGS_COMMON " ip=off ip=off root=/dev/ram0 rw rdinit=/linuxrc"
    #else
      #error "not support config"
    #endif /*CONFIG_SPL_SFC_NAND*/
    #define CONFIG_SYS_SPL_OTA_ARGS_ADDR    CONFIG_SPL_OTA_BOOTARGS
  #endif
  #define CONFIG_SPL_OS_NAME			"kernel"
  #define CONFIG_SYS_SPL_ARGS_ADDR		CONFIG_SPL_BOOTARGS
#endif
/**
 * Drivers configuration.
 */
/* SFC */
#if defined(CONFIG_MTD_SFCNOR) || defined(CONFIG_MTD_SFCNAND)
  #define CONFIG_JZ_SFC
  #ifdef CONFIG_MTD_SFCNOR
    #define CONFIG_JZ_SFC_NOR
    #define CONFIG_SFC_NOR_RATE    	        150000000
    #define CONFIG_SFC_QUAD
    #define CONFIG_CMD_SFC_NOR
    #define CONFIG_SPIFLASH_PART_OFFSET         (CONFIG_SPL_MAX_SIZE)
    #define CONFIG_SPI_NORFLASH_PART_OFFSET     (CONFIG_SPIFLASH_PART_OFFSET + ((size_t)&((struct burner_params*)0)->norflash_partitions))
    #define CONFIG_NOR_MAJOR_VERSION_NUMBER     1
    #define CONFIG_NOR_MINOR_VERSION_NUMBER     0
    #define CONFIG_NOR_REVERSION_NUMBER         0
    #define CONFIG_NOR_VERSION     (CONFIG_NOR_MAJOR_VERSION_NUMBER | (CONFIG_NOR_MINOR_VERSION_NUMBER << 8) | (CONFIG_NOR_REVERSION_NUMBER <<16))
    #ifdef CONFIG_ENV_IS_IN_SFC
      #define CONFIG_ENV_IS_IN_SFC_NOR
    #endif
  #elif defined(CONFIG_MTD_SFCNAND)
    #define CONFIG_SYS_NAND_SELF_INIT
    #define CONFIG_SFC_NAND_RATE    	100000000
    #define CONFIG_SPIFLASH_PART_OFFSET     (CONFIG_SPL_MAX_SIZE)
    #define CONFIG_SPI_NAND_BPP		(2048 +64)	/*Bytes Per Page*/
    #define CONFIG_SPI_NAND_PPB		(64)		/*Page Per Block*/
    #define CONFIG_CMD_SFCNAND
    #define CONFIG_CMD_NAND
    #define CONFIG_SYS_MAX_NAND_DEVICE	1
    #define CONFIG_SYS_NAND_BASE    	0
    #define CONFIG_MTD_DEVICE
    #define CONFIG_CMD_UBI
    #define CONFIG_CMD_UBIFS
    #define CONFIG_CMD_MTDPARTS
    #define CONFIG_MTD_PARTITIONS
    #define MTDIDS_DEFAULT                  "nand0=nand"
    #define MTDPARTS_DEFAULT                "mtdparts=nand:1M(boot),8M(kernel),64M(rootfs),-(data)"
    #ifdef CONFIG_ENV_IS_IN_SFC
      #define CONFIG_ENV_IS_IN_SFC_NAND
    #endif
  #endif
#endif /*MTD_SFCNOR || MTD SFCNAND*/

/* MMC */
#if defined(CONFIG_JZ_MMC_MSC0)
#define CONFIG_CMD_MMC			/* MMC/SD support*/
#define CONFIG_GENERIC_MMC		1
#define CONFIG_MMC			1
#define CONFIG_JZ_MMC			1
#define CONFIG_JZ_MMC_SPLMSC 0
#define CONFIG_JZ_MMC_MSC0_PB 		1
#define CONFIG_MSC_DATA_WIDTH_4BIT
#endif  /* JZ_MMC_MSC0 */

/* GPIO */
#define CONFIG_JZ_GPIO


/**
 * Miscellaneous configurable options
 */
#define CONFIG_LZO
#define CONFIG_RBTREE
#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_SYS_NO_FLASH
#define CONFIG_SYS_FLASH_BASE	0 /* init flash_base as 0 */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_MISC_INIT_R	1
#define CONFIG_BOOTP_MASK	(CONFIG_BOOTP_DEFAUL)
#define CONFIG_SYS_MAXARGS 	16
#define CONFIG_SYS_LONGHELP

#if defined(CONFIG_SPL_SFC_NOR) || defined(CONFIG_SPL_SFC_NAND)
  #define CONFIG_SPL_SFC_SUPPORT
#endif
#if defined(CONFIG_SPL_JZMMC_SUPPORT)
  #if defined(CONFIG_JZ_MMC_MSC0)
    #define CONFIG_SYS_PROMPT CONFIG_SYS_BOARD "-msc0# "
  #endif
#elif defined(CONFIG_SPL_NOR_SUPPORT)
  #define CONFIG_SYS_PROMPT CONFIG_SYS_BOARD "-nor# "
#elif defined(CONFIG_SPL_SFC_SUPPORT)
  #if defined(CONFIG_SPL_SFC_NOR)
    #define CONFIG_SYS_PROMPT CONFIG_SYS_BOARD "-sfcnor# "
  #else  /* CONFIG_SPL_SFC_NAND */
    #define CONFIG_SYS_PROMPT CONFIG_SYS_BOARD "-sfcnand# "
  #endif
#endif

#define CONFIG_SYS_CBSIZE 1024 /* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)

#define CONFIG_SYS_MONITOR_LEN		((256 * 1024) - CONFIG_SPL_PAD_TO)
#define CONFIG_SYS_MALLOC_LEN		(32 * 1024 * 1024)
#define CONFIG_SYS_BOOTPARAMS_LEN	(128 * 1024)

#define CONFIG_SYS_SDRAM_BASE		0x80000000 /* cached (KSEG0) address */
#define CONFIG_SYS_SDRAM_MAX_TOP	0x90000000 /* don't run into IO space */
#define CONFIG_SYS_INIT_SP_OFFSET	0x400000
#define CONFIG_SYS_LOAD_ADDR		0x88000000
#define CONFIG_SYS_MEMTEST_START	0x80000000
#define CONFIG_SYS_MEMTEST_END		0x84000000

#define CONFIG_SYS_TEXT_BASE		0x80100000
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE

/**
 * Environment
 */
#if defined(CONFIG_ENV_IS_IN_MMC)
#define CONFIG_CMD_SAVEENV
#define CONFIG_SYS_MMC_ENV_DEV		0
#define CONFIG_ENV_SIZE			(16 * 1024)
#undef CONFIG_SYS_MONITOR_LEN
#define CONFIG_SYS_MONITOR_LEN          ((512 * 1024) - (CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512) - CONFIG_ENV_SIZE)
#define CONFIG_ENV_OFFSET		(CONFIG_SYS_MONITOR_LEN + CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512)
#elif defined(CONFIG_ENV_IS_IN_SFC_NAND)
#define CONFIG_CMD_SAVEENV
#define CONFIG_SYS_REDUNDAND_ENVIRONMENT
#define CONFIG_ENV_SECT_SIZE 		(128 * 1024) /* the  nand block size is 128K */
#define CONFIG_ENV_SIZE         	CONFIG_ENV_SECT_SIZE
#undef CONFIG_SYS_MONITOR_LEN
#define CONFIG_SYS_MONITOR_LEN          ((512 * 1024) - CONFIG_UBOOT_OFFSET)
#define CONFIG_ENV_OFFSET_REDUND 	(CONFIG_SYS_MONITOR_LEN + CONFIG_UBOOT_OFFSET)
#define CONFIG_ENV_OFFSET       	(CONFIG_ENV_OFFSET_REDUND + CONFIG_ENV_SECT_SIZE)
#elif defined(CONFIG_ENV_IS_IN_SFC_NOR)
#define CONFIG_CMD_SAVEENV
#define CONFIG_ENV_SIZE			(16 * 1024)
#undef CONFIG_SYS_MONITOR_LEN
#define CONFIG_SYS_MONITOR_LEN		((256 * 1024) - CONFIG_UBOOT_OFFSET - CONFIG_ENV_SIZE)
#define CONFIG_ENV_OFFSET		(CONFIG_SYS_MONITOR_LEN + CONFIG_UBOOT_OFFSET)
#else
#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_SIZE			(512)
#endif

/**
 * SPL configuration
 */
#define CONFIG_SPL
#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_NO_CPU_SUPPORT_CODE
#define CONFIG_SPL_START_S_PATH		"$(CPUDIR)/$(SOC)"
#ifdef CONFIG_SPL_NOR_SUPPORT
  #define CONFIG_SPL_LDSCRIPT		"$(CPUDIR)/$(SOC)/u-boot-nor-spl.lds"
#else
  #define CONFIG_SPL_LDSCRIPT		"$(CPUDIR)/$(SOC)/u-boot-spl.lds"
#endif
#define CONFIG_SPL_SERIAL_SUPPORT
#define CONFIG_SPL_GPIO_SUPPORT
#define CONFIG_SPL_BOARD_INIT
#define CONFIG_SPL_LIBGENERIC_SUPPORT
#ifdef CONFIG_SPL_NOR_SUPPORT
  #define CONFIG_SPL_TEXT_BASE		0xba000000
  #define CONFIG_SYS_OS_BASE		0
  #define CONFIG_SYS_SPL_ARGS_ADDR	0
  #define CONFIG_SYS_FDT_BASE		0
  #define CONFIG_SPL_PAD_TO		32768
  #define CONFIG_SPL_MAX_SIZE		CONFIG_SPL_PAD_TO
  #define CONFIG_SYS_UBOOT_BASE		(CONFIG_SPL_TEXT_BASE + CONFIG_SPL_PAD_TO - 0x40)  /* 0x40 = sizeof (image_header) */
#elif defined(CONFIG_SPL_JZMMC_SUPPORT)
  #define CONFIG_SPL_TEXT_BASE		0x80001000
  #define CONFIG_SPL_MAX_SIZE		26624
  #define CONFIG_SPL_PAD_TO	        CONFIG_SPL_MAX_SIZE
  #define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR	 ((CONFIG_SPL_PAD_TO + (17 * 1024)) >> 9) /* 28KB + 17K offset */
#elif defined(CONFIG_SPL_SFC_SUPPORT)
  #define CONFIG_SPI_SPL_CHECK
  #define CONFIG_SPL_TEXT_BASE		0x80001000
  #define CONFIG_SPL_PAD_TO		27648
  #define CONFIG_SPL_MAX_SIZE		(26 * 1024)
  #define CONFIG_UBOOT_OFFSET           CONFIG_SPL_PAD_TO
  #define CONFIG_SPL_VERSION            1
#endif	/*CONFIG_SPL_NOR_SUPPORT*/

/**
 * MBR & GPT configuration
 */
#ifdef CONFIG_MBR_CREATOR
#define CONFIG_MBR_P0_OFF	64mb
#define CONFIG_MBR_P0_END	556mb
#define CONFIG_MBR_P0_TYPE 	linux

#define CONFIG_MBR_P1_OFF	580mb
#define CONFIG_MBR_P1_END 	1604mb
#define CONFIG_MBR_P1_TYPE 	linux

#define CONFIG_MBR_P2_OFF	28mb
#define CONFIG_MBR_P2_END	58mb
#define CONFIG_MBR_P2_TYPE 	linux

#define CONFIG_MBR_P3_OFF	1609mb
#define CONFIG_MBR_P3_END	7800mb
#define CONFIG_MBR_P3_TYPE 	fat
#else
#define CONFIG_GPT_TABLE_PATH	"$(TOPDIR)/board/$(BOARDDIR)"
#endif

#endif /*__CONFIG_X1830_H__*/
