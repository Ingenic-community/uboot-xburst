/*
 * Ingenic mensa configuration
 *
 * Copyright (c) 2013 Ingenic Semiconductor Co.,Ltd
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

#ifndef __CONFIG_CANNA_H__
#define __CONFIG_CANNA_H__

/**
 * Basic configuration(SOC, Cache, UART, DDR).
 */
#define CONFIG_MIPS32R2		/* MIPS32 CPU core */
#define CONFIG_CPU_XBURST
#define CONFIG_SYS_LITTLE_ENDIAN
#define CONFIG_X1000

#define CONFIG_SYS_APLL_FREQ		1008000000	/*If APLL not use mast be set 0*/
#define CONFIG_SYS_MPLL_FREQ		600000000	/*If MPLL not use mast be set 0*/
#define CONFIG_CPU_SEL_PLL		APLL
#define CONFIG_DDR_SEL_PLL		MPLL
#define CONFIG_SYS_CPU_FREQ		1008000000
#define CONFIG_SYS_MEM_FREQ		200000000

#define CONFIG_SYS_EXTAL		24000000	/* EXTAL freq: 24 MHz */
#define CONFIG_SYS_HZ			1000 /* incrementer freq */


#define CONFIG_SYS_DCACHE_SIZE		16384
#define CONFIG_SYS_ICACHE_SIZE		16384
#define CONFIG_SYS_CACHELINE_SIZE	32

#define CONFIG_SYS_UART_INDEX		2
/* #define CONFIG_SYS_UART2_PD */
#define CONFIG_SYS_UART2_PC
#define CONFIG_BAUDRATE			115200

/*#define CONFIG_DDR_TEST*/
#define CONFIG_DDR_PARAMS_CREATOR
#define CONFIG_DDR_HOST_CC
#define CONFIG_DDR_TYPE_LPDDR
#define CONFIG_DDR_CS0			1	/* 1-connected, 0-disconnected */
#define CONFIG_DDR_CS1			0	/* 1-connected, 0-disconnected */
#define CONFIG_DDR_DW32			0	/* 1-32bit-width, 0-16bit-width */
#define CONFIG_DDR_tREFI	    DDR__ns(7800)
/*
  Output Drive Strength: Controls the output drive strength. Valid values are:
  000 = Full strength driver
  001 = Half strength driver
  110 = Quarter strength driver
  111 = Octant strength driver
  100 = Three-quarters strength driver
*/
#define CONFIG_DDR_DRIVER_STRENGTH             4

#if defined CONFIG_128MB_LPDDR
#define CONFIG_MDDR_ECM220ACBCN_50
#elif defined CONFIG_64MB_LPDDR
#define CONFIG_MDDR_JSD12164PAI_KGD
#else
#define  CONFIG_DDR_64M     64
#define  CONFIG_DDR_32M     32
#define CONFIG_MDDR_EMD56164PC_50I
#endif

#define CONFIG_AUDIO_CAL_DIV
#define CONFIG_AUDIO_APLL CONFIG_SYS_APLL_FREQ
#define CONFIG_AUDIO_MPLL CONFIG_SYS_MPLL_FREQ


/* #define CONFIG_DDR_DLL_OFF */
/*
 * #define CONFIG_DDR_CHIP_ODT
 * #define CONFIG_DDR_PHY_ODT
 * #define CONFIG_DDR_PHY_DQ_ODT
 * #define CONFIG_DDR_PHY_DQS_ODT
 * #define CONFIG_DDR_PHY_IMPED_PULLUP		0xe
 * #define CONFIG_DDR_PHY_IMPED_PULLDOWN	0xe
 */

#if defined(CONFIG_SPL_SFC_NOR) || defined(CONFIG_SPL_SFC_NAND)
#define CONFIG_SPL_SFC_SUPPORT
#define CONFIG_FLASH_PARAMS_BASE 0x80200000
#define CONFIG_SPL_VERSION	1
#endif

/*get wifi_mac from SFC flash*/
#ifdef CONFIG_GET_WIFI_MAC
#define WIFI_MAC_READ_ADDR	0x40000
#define WIFI_MAC_READ_COUNT	12
#endif

#define CONFIG_CHECK_SOCID
/**
 * common boot arguments definitions.
 */
#if defined CONFIG_128MB_LPDDR
#define BOOTARGS_BASE "console=ttyS2,115200n8 consoleblank=0 mem=128M lpj=5009408"
#elif defined CONFIG_64MB_LPDDR
#define BOOTARGS_BASE "console=ttyS2,115200n8 consoleblank=0 mem=64M lpj=5009408"
#else
#define BOOTARGS_BASE "console=ttyS2,115200n8 consoleblank=0 mem=32M lpj=5009408"
#endif

#if defined CONFIG_GET_WIFI_MAC
#define BOOTARGS_COMMON BOOTARGS_BASE "wifi_mac=xxxxxxxxxxxx "
#else
#define BOOTARGS_COMMON BOOTARGS_BASE
#endif

/* uboot load kernel settings. */
#if defined(CONFIG_SPL_SFC_NOR)
	#define CONFIG_BOOTARGS BOOTARGS_COMMON "ip=off init=/linuxrc rootfstype=cramfs root=/dev/mtdblock4 rw"
	#define CONFIG_BOOTCOMMAND "sfcnor read 0x40000 0x800000 0x80800000 ;bootm 0x80800000"
#elif defined(CONFIG_SPL_SFC_NAND)
	#define CONFIG_BOOTARGS BOOTARGS_COMMON "ip=off init=/linuxrc ubi.mtd=2 ubi.mtd=4 ubi.mtd=5 root=ubi1:updater rootfstype=ubifs rw"
	#define CONFIG_BOOTCOMMAND "sfcnand read 0x600000 0x300000 0x80600000 ;bootm 0x80600000"
#elif defined(CONFIG_SPL_JZMMC_SUPPORT)
	/* 1. X1000 ONLY support boot from msc0; 2. canna board ONLY support gpt partitions */
	#if defined(CONFIG_JZ_MMC_MSC0) || defined(CONFIG_GPT_CREATOR)
		#define CONFIG_BOOTARGS BOOTARGS_COMMON " root=/dev/mmcblk0p7 rootfstype=ext4 rw"
		#define CONFIG_BOOTCOMMAND "mmc dev 0;mmc read 0x80600000 0x1800 0x3000; bootm 0x80600000"
	#endif
#endif

/* spl load kernel settings. */
#ifdef CONFIG_SPL_OS_BOOT
	#define CONFIG_SPL_NV_BASE          CONFIG_SYS_TEXT_BASE
	#ifdef CONFIG_SPL_SFC_NAND
		#define CONFIG_SPL_OS_OFFSET        (0x600000) /* spi offset of zImage being loaded */
		#define CONFIG_SPL_OTA_OS_OFFSET    (0x4900000) /* spi offset of zImage being loaded */
		#define NV_AREA_BASE_ADDR           (0x100000)
		#define CONFIG_SPL_BOOTARGS         BOOTARGS_COMMON "ip=off init=/linuxrc ubi.mtd=2 ubi.mtd=4 ubi.mtd=5 root=ubi1:updater rootfstype=ubifs rw"
		#define CONFIG_SPL_OTA_BOOTARGS     BOOTARGS_COMMON "ip=off init=/linuxrc ubi.mtd=2 ubi.mtd=6 root=ubi1:updater rootfstype=ubifs rw"
	#elif defined CONFIG_SPL_SFC_NOR
		#define CONFIG_SPL_OTA_OS_OFFSET       (0xd00000) /* spi offset of zImage being loaded */
		#define NV_AREA_BASE_ADDR           (0x40000)
		#define CONFIG_SPL_BOOTARGS         BOOTARGS_COMMON "ip=off init=/linuxrc rootfstype=cramfs root=/dev/mtdblock4 rw"
		#define CONFIG_SPL_OTA_BOOTARGS     BOOTARGS_COMMON "ip=off init=/linuxrc rootfstype=cramfs root=/dev/mtdblock5 rw"
	#elif defined CONFIG_SPL_JZMMC_SUPPORT
		/* 1. X1000 ONLY support boot from msc0; 2. canna board ONLY support gpt partitions */
		#ifdef CONFIG_JZ_MMC_MSC0
			#ifdef CONFIG_GPT_CREATOR
				#define CONFIG_GPT_TABLE_PATH	"$(TOPDIR)/board/$(BOARDDIR)"
				#define CONFIG_SPL_OS_OFFSET        (0x600000) /* mmc offset of zImage being loaded */
				#define CONFIG_SPL_OTA_OS_OFFSET    (0x13500000) /* mmc offset of ota zImage being loaded */
				#define NV_AREA_BASE_ADDR           (0x100000) /* mmc offset of nv being loaded */
				#define CONFIG_SPL_BOOTARGS     BOOTARGS_COMMON " ip=off init=/linuxrc root=/dev/mmcblk0p5 rootfstype=ext4 rw"
				#define CONFIG_SPL_OTA_BOOTARGS BOOTARGS_COMMON " ip=off init=/linuxrc root=/dev/mmcblk0p7 rootfstype=ext4 rw"
			#endif /* CONFIG_GPT_CREATOR */
		#endif /* CONFIG_JZ_MMC_MSC0 */
	#endif /* CONFIG_SPL_JZMMC_SUPPORT */

	#define CONFIG_PAR_NV_NAME        "nv"
	#define CONFIG_SPL_OS_NAME        "kernel" /* spi offset of xImage being loaded */
	#define CONFIG_PAT_USERFS_NAME    "userfs"
	#define CONFIG_PAT_UPDATEFS_NAME  "updatefs"
	#define CONFIG_SYS_SPL_ARGS_ADDR    CONFIG_SPL_BOOTARGS
	#define CONFIG_BOOTX_BOOTARGS     BOOTARGS_COMMON "ip=off init=/linuxrc rootfstype=cramfs root=/dev/mtdblock6 rw"
#ifdef CONFIG_OTA_VERSION20
	#define CONFIG_NV_INFO_AS_IAD
#endif

#else /* CONFIG_SPL_OS_BOOT */

#endif /* CONFIG_SPL_OS_BOOT */

#define PARTITION_NUM 10

/**
 * Boot command definitions.
 */
#define CONFIG_BOOTDELAY 1

/* CLK CGU */
#define CGU_CLK_SRC {			\
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
#define CONFIG_CMD_BOOTD	/* bootd			*/
#define CONFIG_CMD_CONSOLE	/* coninfo			*/
#define CONFIG_CMD_ECHO		/* echo arguments		*/
#define CONFIG_CMD_EXT4 	/* ext4 support			*/
#define CONFIG_CMD_FAT		/* FAT support			*/
#define CONFIG_CMD_MEMORY	/* md mm nm mw cp cmp crc base loop mtest */
#define CONFIG_CMD_MISC		/* Misc functions like sleep etc*/
#define CONFIG_CMD_RUN		/* run command in env variable	*/
#define CONFIG_CMD_SAVEENV	/* saveenv			*/
#define CONFIG_CMD_SETGETDCR	/* DCR support on 4xx		*/
#define CONFIG_CMD_SOURCE	/* "source" command support	*/
#define CONFIG_CMD_GETTIME
#define CONFIG_CMD_UNZIP        /* unzip from memory to memory  */
#define CONFIG_CMD_DHCP
/* #define CONFIG_CMD_NET */     /* networking support*/
#define CONFIG_CMD_PING

#ifndef CONFIG_SPL_BUILD
#define CONFIG_USE_ARCH_MEMSET
#define CONFIG_USE_ARCH_MEMCPY
#endif

/* DEBUG ETHERNET */
#if 0
#define CONFIG_SERVERIP     192.168.4.13
#define CONFIG_IPADDR       192.168.4.90
#define CONFIG_GATEWAYIP    192.168.4.1
#define CONFIG_NETMASK      255.255.255.0
#define CONFIG_ETHADDR      00:11:22:33:44:55

#define GMAC_PHY_MII	1
#define GMAC_PHY_RMII	2
#define GMAC_PHY_GMII	3
#define GMAC_PHY_RGMII	4
#define CONFIG_NET_GMAC_PHY_MODE GMAC_PHY_RMII

#define PHY_TYPE_DM9161      1
#define PHY_TYPE_88E1111     2
#define CONFIG_NET_PHY_TYPE   PHY_TYPE_DM9161

#define CONFIG_NET_GMAC
#define CONFIG_GPIO_DM9161_RESET   GPIO_PB(25)
#define CONFIG_GPIO_DM9161_RESET_ENLEVEL   0
#endif

/* MMC */
#define CONFIG_CMD_MMC
/*#define CONFIG_MMC_TRACE*/


#ifdef CONFIG_CMD_MMC
#define CONFIG_GENERIC_MMC		1
#define CONFIG_MMC			1
#define CONFIG_JZ_MMC			1

#ifdef CONFIG_JZ_MMC_MSC0
#define CONFIG_JZ_MMC_SPLMSC 0
#define CONFIG_JZ_MMC_MSC0_PA_4BIT 1
/* #define CONFIG_JZ_MMC_MSC0_PA_8BIT 1 */
/* #define CONFIG_MSC_DATA_WIDTH_8BIT */
#define CONFIG_MSC_DATA_WIDTH_4BIT
/* #define CONFIG_MSC_DATA_WIDTH_1BIT */
#endif

#ifdef CONFIG_JZ_MMC_MSC1
/*#define CONFIG_JZ_MMC_SPLMSC 1*/
#define CONFIG_JZ_MMC_MSC1_PC 1
/* #define CONFIG_MSC_DATA_WIDTH_8BIT */
#define CONFIG_MSC_DATA_WIDTH_4BIT
/* #define CONFIG_MSC_DATA_WIDTH_1BIT */
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
#define CONFIG_MISC_INIT_R 1

#define CONFIG_BOOTP_MASK	(CONFIG_BOOTP_DEFAUL)

#define CONFIG_SYS_MAXARGS 16
#define CONFIG_SYS_LONGHELP

#if defined(CONFIG_SPL_JZMMC_SUPPORT)
	#if	defined(CONFIG_JZ_MMC_MSC0)
	#define CONFIG_SYS_PROMPT CONFIG_SYS_BOARD "-msc0# "
	#else
	#define CONFIG_SYS_PROMPT CONFIG_SYS_BOARD "-msc1# "
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

#if defined(CONFIG_SUPPORT_EMMC_BOOT)
#define CONFIG_SYS_MONITOR_LEN		(384 * 1024)
#else
#define CONFIG_SYS_MONITOR_LEN		(512 << 10)
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
#define CONFIG_SYS_MMC_ENV_DEV		0
#define CONFIG_ENV_SIZE			(32 << 10)
#define CONFIG_ENV_OFFSET		(CONFIG_SYS_MONITOR_LEN + CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512)
#elif defined(CONFIG_ENV_IS_IN_SFC)
#define CONFIG_ENV_SIZE			(4 << 10)
#define CONFIG_ENV_OFFSET		0x2e400 /*write nor flash 185k address*/
#define CONFIG_CMD_SAVEENV
#endif

/**
 * SPL configuration
 */
#define CONFIG_SPL
#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_NO_CPU_SUPPORT_CODE
#define CONFIG_SPL_START_S_PATH		"$(CPUDIR)/$(SOC)"
#ifdef CONFIG_SPL_NOR_SUPPORT
#define CONFIG_SPL_LDSCRIPT             "$(CPUDIR)/$(SOC)/u-boot-nor-spl.lds"
#else
#define CONFIG_SPL_LDSCRIPT		"$(CPUDIR)/$(SOC)/u-boot-spl.lds"
#endif
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR	0x3A /* 12KB+17K offset */
#define CONFIG_SYS_U_BOOT_MAX_SIZE_SECTORS	0x200 /* 256 KB */
#ifndef CONFIG_SMALLER_SPL
#define CONFIG_SPL_SERIAL_SUPPORT
#endif
#define CONFIG_SPL_GPIO_SUPPORT
#define CONFIG_SPL_LIBGENERIC_SUPPORT
#if defined(CONFIG_SPL_NOR_SUPPORT)
#define CONFIG_SPL_TEXT_BASE		0xba000000
#define CONFIG_SYS_UBOOT_BASE		(CONFIG_SPL_TEXT_BASE + CONFIG_SPL_PAD_TO - 0x40)
					/* 0x40 = sizeof (image_header)*/
#define CONFIG_SYS_OS_BASE		0
#define CONFIG_SYS_FDT_BASE		0
#define CONFIG_SPL_PAD_TO		32768
#define CONFIG_SPL_MAX_SIZE		(32 * 1024)
#elif defined(CONFIG_SPL_JZMMC_SUPPORT)
#define CONFIG_SPL_PAD_TO		12288  /* spl size */
#define CONFIG_SPL_TEXT_BASE		0xf4001000
#define CONFIG_SPL_MAX_SIZE		(12 * 1024)
#elif defined(CONFIG_SPL_SFC_SUPPORT)
#define CONFIG_UBOOT_OFFSET             (4<<12)
#define CONFIG_JZ_SFC_PA_6BIT
#ifdef	CONFIG_SPL_SFC_NAND
#define CONFIG_SFC_RATE    150000000
#define CONFIG_SPIFLASH_PART_OFFSET     0x3c00
#define CONFIG_SPI_NAND_BPP			(2048 +64)		/*Bytes Per Page*/
#define CONFIG_SPI_NAND_PPB			(64)		/*Page Per Block*/
#define CONFIG_SPL_TEXT_BASE		0xf4001000
#define CONFIG_SPL_MAX_SIZE		(12 * 1024)
#define CONFIG_SPL_PAD_TO		16384
#define CONFIG_MTD_SFCNAND
#define CONFIG_JZ_SFC
#define CONFIG_CMD_SFCNAND
#define CONFIG_CMD_NAND
#define CONFIG_SPI_SPL_CHECK
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE    0xb3441000

/*SFCNAND env*/
/* spi nand environment */
#define CONFIG_SYS_REDUNDAND_ENVIRONMENT
#define CONFIG_ENV_SECT_SIZE 0x20000 /* 128K*/
#define SPI_NAND_BLK            0x20000 /*the spi nand block size */
#define CONFIG_ENV_SIZE         SPI_NAND_BLK /* uboot is 1M but the last block size is the env*/
#define CONFIG_ENV_OFFSET       0xc0000 /* offset is 768k */
#define CONFIG_ENV_OFFSET_REDUND (CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE)
#define CONFIG_ENV_IS_IN_SFC_NAND


#else
#define CONFIG_SPI_SPL_CHECK
#define CONFIG_SPL_TEXT_BASE		0xf4001000
#define CONFIG_SPL_MAX_SIZE		((16 * 1024) - 0x800)
#define CONFIG_SPL_PAD_TO		16384
#define CONFIG_CMD_SFC_NOR
#endif
#endif

#ifdef CONFIG_CMD_SPI
#define CONFIG_SSI_BASE SSI0_BASE
#define CONFIG_SPI_BUILD
#ifdef CONFIG_SOFT_SPI
#undef SPI_INIT
#define SPI_DELAY
#define	SPI_SDA(val)    gpio_direction_output(GPIO_PA(21), val)
#define	SPI_SCL(val)	gpio_direction_output(GPIO_PA(18), val)
#define	SPI_READ	gpio_get_value(GPIO_PA(20))
#else
#define CONFIG_JZ_SPI
#endif
/*#define CONFIG_JZ_SPI_FLASH*/
#define CONFIG_CMD_SF
#define CONFIG_SPI_FLASH_INGENIC
#define CONFIG_SPI_FLASH
#define CONFIG_UBOOT_OFFSET             (4<<12)
#endif

#ifdef CONFIG_CMD_SFC_NOR
#define CONFIG_JZ_SFC
#define CONFIG_JZ_SFC_NOR
/*#define CONFIG_SPI_DUAL*/
#define CONFIG_SPI_QUAD
#define CONFIG_SFC_RATE    150000000
#endif

#ifdef CONFIG_JZ_SFC_NOR
#define CONFIG_SPIFLASH_PART_OFFSET     0x3c00
#define CONFIG_SPI_NORFLASH_PART_OFFSET     0x3c74
#define CONFIG_NOR_MAJOR_VERSION_NUMBER     1
#define CONFIG_NOR_MINOR_VERSION_NUMBER     0
#define CONFIG_NOR_REVERSION_NUMBER     0
#define CONFIG_NOR_VERSION     (CONFIG_NOR_MAJOR_VERSION_NUMBER | (CONFIG_NOR_MINOR_VERSION_NUMBER << 8) | (CONFIG_NOR_REVERSION_NUMBER <<16))
#endif


/*
* MTD support
*/
#define CONFIG_SYS_NAND_SELF_INIT


#endif /* __CONFIG_CANNA_H__ */
