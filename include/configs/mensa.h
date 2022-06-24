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

#ifndef __CONFIG_MENSA_H__
#define __CONFIG_MENSA_H__

/**
 * Basic configuration(SOC, Cache, UART, DDR).
 */
#define CONFIG_MIPS32		/* MIPS32 CPU core */
#define CONFIG_CPU_XBURST
#define CONFIG_SYS_LITTLE_ENDIAN
#define CONFIG_JZ4775		/* Jz4775 SoC */

#define CONFIG_SYS_APLL_FREQ		1200000000
#define CONFIG_SYS_MPLL_FREQ		-1

#define CONFIG_SYS_EXTAL		24000000	/* EXTAL freq: 48 MHz */
#define CONFIG_SYS_HZ			1000 /* incrementer freq */

#define CONFIG_SYS_CPU_FREQ		CONFIG_SYS_APLL_FREQ
#define CONFIG_SYS_MEM_DIV		3
#define CONFIG_SYS_MEM_FREQ		(CONFIG_SYS_APLL_FREQ / CONFIG_SYS_MEM_DIV)

#define CONFIG_SYS_DCACHE_SIZE		16384
#define CONFIG_SYS_ICACHE_SIZE		16384
#define CONFIG_SYS_CACHELINE_SIZE	32

#define CONFIG_SYS_UART_INDEX		3
#define CONFIG_BAUDRATE			115200

#define CONFIG_DDR_PARAMS_CREATOR
#define CONFIG_DDR_HOST_CC
#define CONFIG_DDR_TYPE_DDR3
#define CONFIG_DDR_CS0			1	/* 1-connected, 0-disconnected */
#define CONFIG_DDR_CS1			0	/* 1-connected, 0-disconnected */
#define CONFIG_DDR_DW32			1	/* 1-32bit-width, 0-16bit-width */
#define CONFIG_DDRC_CTRL_PDT DDRC_CTRL_PDT_128
#define CONFIG_DDR3_H5TQ2G63GFR_RDC
#define CONFIG_DDR_PHY_IMPEDANCE 40000 /* 60000: not stable on some mensa board. 40000 is much more stable */
#define CONFIG_DDR_PHY_ODT_IMPEDANCE 40000
#define CONFIG_DDR_DRIVER_OUT_STRENGTH 0 /* make mesa much more stable */
/* #define CONFIG_DDR_PHY_MR0_PD 0 */
/* #define CONFIG_DDR_DLL_OFF */
/*
 * #define CONFIG_DDR_CHIP_ODT
 * #define CONFIG_DDR_PHY_ODT
 * #define CONFIG_DDR_PHY_DQ_ODT
 * #define CONFIG_DDR_PHY_DQS_ODT
 * #define CONFIG_DDR_PHY_IMPED_PULLUP		0xe
 * #define CONFIG_DDR_PHY_IMPED_PULLDOWN	0xe
 */

/**
 * Boot arguments definitions.
 */
#define BOOTARGS_COMMON "console=ttyS3,115200 mem=256M@0x0 mem=256M@0x30000000 root_wait "

#ifdef CONFIG_BOOT_ANDROID
  #define CONFIG_BOOTARGS BOOTARGS_COMMON " ip=off root=/dev/ram0 rw rdinit=/init"
#else
  #ifdef CONFIG_SPL_MMC_SUPPORT
/*    #define CONFIG_BOOTARGS BOOTARGS_COMMON " ip=192.168.4.250:192.168.4.1:192.168.4.1:255.255.255.0 nfsroot=192.168.4.13:/home/nfsroot/fpga/user/bliu/root_ok rw"*/
/*	#define CONFIG_BOOTARGS BOOTARGS_COMMON " ip=off root=/dev/ram0 rw rdinit=/linuxrc" */
      #ifdef CONFIG_JZ_MMC_SPLMSC
         #define CONFIG_BOOTARGS BOOTARGS_COMMON " ip=192.168.4.90:192.168.4.1:192.168.4.1:255.255.255.0 init=/linuxrc root=/dev/mmcblk1p1 rw"
      #else
         #define CONFIG_BOOTARGS BOOTARGS_COMMON " ip=192.168.4.90:192.168.4.1:192.168.4.1:255.255.255.0 init=/linuxrc root=/dev/mmcblk0p1 rw"
      #endif
  #elif defined(CONFIG_JZ_NAND_MGR)
      #define CONFIG_BOOTARGS BOOTARGS_COMMON " root=/dev/ndsystem rw"
  #else
/*#define CONFIG_BOOTARGS BOOTARGS_COMMON " ubi.mtd=1 ubi.mtd=2 root=ubi1:ndsystem rootfstype=ubifs rw"*/
      #define CONFIG_BOOTARGS BOOTARGS_COMMON " ubi.mtd=2 root=ubi0:ndsystem rootfstype=ubifs rw"
  #endif
#endif

#define PARTITION_NUM 10

/**
 * Boot command definitions.
 */
#define CONFIG_BOOTDELAY  1
#ifdef CONFIG_BOOT_ANDROID
#ifdef CONFIG_SPL_MMC_SUPPORT
#define CONFIG_BOOTCOMMAND							\
	"batterydet; cls; boota mmc 0 0x80f00000 6144"
#define CONFIG_NORMAL_BOOT CONFIG_BOOTCOMMAND
#define CONFIG_RECOVERY_BOOT "boota mmc 0 0x80f00000 24576"
#else
/*#define CONFIG_BOOTCOMMAND "boota nand 0 0x80f00000 6144"*/
#define CONFIG_BOOTCOMMAND  "nand_zm read ndboot 0 0x400000 0x80f00000;boota mem 0x80f00000"
#define CONFIG_NORMAL_BOOT CONFIG_BOOTCOMMAND
#define CONFIG_RECOVERY_BOOT "boota nand 0 0x80f00000 24576"
#endif
#else  /* CONFIG_BOOT_ANDROID */
#ifdef CONFIG_SPL_MMC_SUPPORT
   #ifdef CONFIG_JZ_MMC_SPLMSC
      #define CONFIG_BOOTCOMMAND "mmc dev 1;mmc read 0x80f00000 0x1800 0x3000; bootm 0x80f00000"
   #else
      #define CONFIG_BOOTCOMMAND "mmc dev 0;mmc read 0x80f00000 0x1800 0x3000; bootm 0x80f00000"
   #endif
/* #define CONFIG_BOOTCOMMAND "tftpboot 0x80600000 user/bliu/mensa/uImage ;bootm 0x80600000" */
#else
#ifdef CONFIG_JZ_NAND_MGR
#define CONFIG_BOOTCOMMAND  "nand_zm read ndboot 0 0x400000 0x80600000;bootm"
/*order ops pt offset len dst */
/*#define CONFIG_BOOTCOMMAND        "nand_zm read ndboot;bootm"*/
#else
#define CONFIG_BOOTCOMMAND												\
	"mtdparts default; ubi part kernel; ubi read 0x80f00000 ndboot 0x400000; bootm 0x80f00000"
/*"mtdparts default; ubi part kernel; ubifsmount ubi:ndboot;" \
  "ubifsload 0x80f00000 uImage; bootm 0x80f00000"*/
#endif /* endif config_jz_nand_MGR*/
#endif
#endif /* CONFIG_BOOT_ANDROID */

/**
 * Drivers configuration.
 */
#define CONFIG_LCD
#ifdef CONFIG_LCD
#define LCD_BPP				5
#define CONFIG_GPIO_LCD_PWM	 	GPIO_PE(1)
#define CONFIG_LCD_LOGO
#define CONFIG_RLE_LCD_LOGO
#define CONFIG_LCD_INFO_BELOW_LOGOONFIG_BMP_LOGO
#define CONFIG_SYS_VIDEO_LOGO_MAX_SIZE (1 << 20)
#define CONFIG_VIDEO_BMP_GZIP
#define CONFIG_LOGO_EXTEND
/*#define CONFIG_PANEL_DIV_LOGO    1*/
#define CONFIG_SYS_WHITE_ON_BLACK
#define CONFIG_SYS_PCLK_FREQ		33260000
#define CONFIG_SYS_PWM_PERIOD		10000 /* Pwm period in ns */
#define CONFIG_SYS_PWM_CHN		1  /* Pwm channel ok*/
#define CONFIG_SYS_PWM_FULL		256
#define CONFIG_SYS_BACKLIGHT_LEVEL	80 /* Backlight brightness is (80 / 256) */
#define CONFIG_VIDEO_JZ4775
#define CONFIG_JZ_PWM
/*#define CONFIG_VIDEO_BYD_BM8766U*/
/*#define CONFIG_VIDEO_KFM701A_21_1A*/
#define CONFIG_VIDEO_BM347WV_F_8991FTGF
#ifdef CONFIG_RLE_LCD_LOGO
#define CONFIG_CMD_BATTERYDET  	/* detect battery and show charge logo */
#define CONFIG_CMD_LOGO_RLE	/*display the logo using rle command*/
#endif
#endif /* CONFIG_LCD */

/* MMC */
#define CONFIG_GENERIC_MMC		1
#define CONFIG_MMC			1
#define CONFIG_JZ_MMC 1
#ifdef CONFIG_JZ_MMC_SPLMSC
#define CONFIG_JZ_MMC_MSC1 1
#define CONFIG_JZ_MMC_MSC1_PE 1
#else
#define CONFIG_JZ_MMC_MSC0 1
#define CONFIG_JZ_MMC_MSC0_PA_4BIT 1
#define CONFIG_JZ_MMC_SPLMSC 0
#endif


/* I2C */
#define CONFIG_SOFT_I2C
#define CONFIG_SYS_I2C_SPEED		50     /* the function is not implemented */
#define CONFIG_SYS_I2C_SLAVE		0x00   /* the function is not implemented */
#define CONFIG_SOFT_I2C_GPIO_SCL	GPIO_PE(3)
#define CONFIG_SOFT_I2C_GPIO_SDA	GPIO_PE(0)

/* PMU */
#define CONFIG_REGULATOR
#define CONFIG_PMU_ACT8600

/* Ethernet: gmac and 9161 */
#define GMAC_PHY_MII	1
#define GMAC_PHY_RMII	2
#define GMAC_PHY_GMII	3
#define GMAC_PHY_RGMII	4
#define CONFIG_NET_JZ4775

#ifdef CONFIG_MENSA_V20
#define CONFIG_GPIO_DM9161_RESET	GPIO_PF(15)
#define CONFIG_GPIO_DM9161_RESET_ENLEVEL	0
#define CONFIG_NET_GMAC_PHY_MODE GMAC_PHY_RMII
#else
#define CONFIG_GPIO_DM9161_RESET	GPIO_PB(7)
#define CONFIG_GPIO_DM9161_RESET_ENLEVEL	0
#define CONFIG_NET_GMAC_PHY_MODE GMAC_PHY_MII
#endif


/* DEBUG ETHERNET */
#define CONFIG_SERVERIP		192.168.4.13
#define CONFIG_IPADDR		192.168.4.90
#define CONFIG_GATEWAYIP        192.168.4.1
#define CONFIG_NETMASK          255.255.255.0
#define CONFIG_ETHADDR          00:11:22:33:44:55

/* GPIO */
#define CONFIG_JZ_GPIO

/**
 * Command configuration.
 */
#define CONFIG_CMD_BOOTD	/* bootd			*/
#define CONFIG_CMD_CONSOLE	/* coninfo			*/
#define CONFIG_CMD_DHCP 	/* DHCP support			*/
#define CONFIG_CMD_ECHO		/* echo arguments		*/
#define CONFIG_CMD_EXT4 	/* ext4 support			*/
#define CONFIG_CMD_FAT		/* FAT support			*/
#define CONFIG_CMD_LOADB	/* loadb			*/
#define CONFIG_CMD_LOADS	/* loads			*/
#define CONFIG_CMD_MEMORY	/* md mm nm mw cp cmp crc base loop mtest */
#define CONFIG_CMD_MISC		/* Misc functions like sleep etc*/
#define CONFIG_CMD_MMC		/* MMC/SD support			*/
#define CONFIG_CMD_NET		/* networking support			*/
#define CONFIG_CMD_PING
#define CONFIG_CMD_RUN		/* run command in env variable	*/
#define CONFIG_CMD_SAVEENV	/* saveenv			*/
#define CONFIG_CMD_SETGETDCR	/* DCR support on 4xx		*/
#define CONFIG_CMD_SOURCE	/* "source" command support	*/
#define CONFIG_CMD_GETTIME
#define CONFIG_CMD_BMP          /* BMP support                  */
#define CONFIG_CMD_UNZIP        /* unzip from memory to memory  */


/* USB */
#ifdef CONFIG_BOOT_ANDROID
#define CONFIG_CMD_FASTBOOT
#define CONFIG_USB_GADGET
#define CONFIG_USB_GADGET_DUALSPEED
#define CONFIG_USB_JZ_DWC2_UDC_V1_1
#define CONFIG_FASTBOOT_GADGET
#define CONFIG_FASTBOOT_FUNCTION
#define CONFIG_G_FASTBOOT_VENDOR_NUM	(0x18d1)
#define CONFIG_G_FASTBOOT_PRODUCT_NUM	(0xdddd)
#define CONFIG_USB_GADGET_VBUS_DRAW 500
#endif  /*CONFIG_CMD_FASTBOOT*/

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
#define CONFIG_SYS_PROMPT CONFIG_SYS_BOARD "# "
#define CONFIG_SYS_CBSIZE 1024 /* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)

#define CONFIG_SYS_MONITOR_LEN		(2 * 1024 * 1024)
#define CONFIG_SYS_MALLOC_LEN		(16 * 1024 * 1024)
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
#ifdef CONFIG_ENV_IS_IN_MMC
#define CONFIG_SYS_MMC_ENV_DEV		0
#define CONFIG_ENV_SIZE			(32 << 10)
#define CONFIG_ENV_OFFSET		(CONFIG_SYS_MONITOR_LEN + CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512)
#endif /* endif CONFIG_ENV_IS_IN_MMC */

#ifdef CONFIG_JZ_NAND_MGR
/* environment  */
#define CONFIG_ENV_IS_IN_NAND_ZM
#define CMDLINE_PARTITION	"ndcmdline"
#define CONFIG_ENV_SIZE			(32 << 10)
#define CONFIG_ENV_OFFSET		0

#define CONFIG_SPL_NAND_BASE
#define CONFIG_SPL_NAND_DRIVERS
#define CONFIG_SPL_NAND_SIMPLE
#define CONFIG_SPL_NAND_LOAD

/* nand gpio init */
#define CONFIG_NAND_LOADER
#define CFG_NAND_BW8    1
#define CONFIG_NAND_CS  1
#define CONFIG_SPL_TEXT_BASE		0xf4000800
#define CONFIG_SPL_MAX_SIZE		((16 * 1024) - 0x800)
/* the NAND SPL is small enough to enable serial */
#define CONFIG_SPL_SERIAL_SUPPORT
#define CONFIG_SYS_NAND_U_BOOT_OFFS	(128* CONFIG_SYS_NAND_PAGE_SIZE)
#define CONFIG_SYS_NAND_U_BOOT_DST	CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_NAND_U_BOOT_START	CONFIG_SYS_NAND_U_BOOT_DST
#define CONFIG_SYS_NAND_U_BOOT_SIZE	(512 * 1024)
#endif /* endif CONFIG_JZ_NAND_MGR */

#ifdef CONFIG_MTD_NAND_JZ
#define CONFIG_MTD_NAND_JZ_AUTO_PARAMS
#define CONFIG_SPL_NAND_SUPPORT
#define CONFIG_SPL_NAND_BASE
#define CONFIG_SPL_NAND_DRIVERS
#define CONFIG_SPL_NAND_SIMPLE
#define CONFIG_NAND_LOADER
#define CFG_NAND_BW8    1
#define CONFIG_NAND_CS  1
#define CONFIG_MTD_NAND_JZ_NEMC         1
#define CONFIG_MTD_NAND_JZ_BCH          1
#define CONFIG_SYS_MAX_NAND_DEVICE      1
#define CONFIG_SYS_NAND_BASE            0xbb000000
#define CONFIG_SYS_NAND_ECCSIZE         1024
#define CONFIG_SYS_NAND_HW_ECC_OOBFIRST
#define CONFIG_SPL_TEXT_BASE		0xf4000800
#define CONFIG_SPL_MAX_SIZE		((16 * 1024) - 0x800)
#define CONFIG_SPL_SERIAL_SUPPORT
#define CONFIG_SYS_NAND_U_BOOT_DST      CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_NAND_U_BOOT_START    CONFIG_SYS_NAND_U_BOOT_DST
#define CONFIG_SYS_NAND_U_BOOT_SIZE     (512 * 1024)
#define CONFIG_ENV_IS_IN_NAND_JZMTD
#define CONFIG_ENV_SIZE			(32 << 10)
#define CONFIG_SPL_PAD_TO		16384
#define MTDIDS_DEFAULT                  "nand0=nand"
#define MTDPARTS_DEFAULT                "mtdparts=nand:8M(boot),30M(kernel),-(root)"
#define CONFIG_MTD_DEVICE
#define CONFIG_MTD_PARTITIONS
#define CONFIG_CMD_NAND
#define CONFIG_CMD_MTDPARTS
#define CONFIG_CMD_UBI
#define CONFIG_CMD_NAND_LOCK_UNLOCK
#define CONFIG_CMD_UBIFS
/*#define CONFIG_MTD_UBI_DEBUG_MSG
  #define CONFIG_MTD_UBI_DEBUG_MSG_IO
  #define CONFIG_MTD_UBI_DEBUG_MSG_BLD*/
#ifdef CONFIG_MTD_NAND_JZ_AUTO_PARAMS
#define CONFIG_SYS_NAND_SELF_INIT
#define CONFIG_SYS_SPL_NAND_FLAG_ADDR   0xf4000800
#define CONFIG_BOARD_TCSM_BASE		0xb3422000
#else
#define CONFIG_SYS_NAND_ONFI_DETECTION
#define CONFIG_NAND_MICRON_MT29F32G08CBACA
#define CONFIG_SYS_NAND_5_ADDR_CYCLE    1
#define CONFIG_SYS_NAND_PAGE_SIZE	4096
#define CONFIG_SYS_NAND_BLOCK_SIZE      (1024 << 10)
#define CONFIG_SYS_NAND_PAGE_COUNT      (CONFIG_SYS_NAND_BLOCK_SIZE / CONFIG_SYS_NAND_PAGE_SIZE)
#define CONFIG_SYS_NAND_BAD_BLOCK_POS   0
#define CONFIG_SYS_NAND_U_BOOT_OFFS	(128* CONFIG_SYS_NAND_PAGE_SIZE * 8)
#define CONFIG_SYS_NAND_U_BOOT_SIZE	(512 * 1024)
#define CONFIG_SYS_NAND_ECCSTRENGTH	24
#define CONFIG_SYS_NAND_ECCBYTES	(CONFIG_SYS_NAND_ECCSTRENGTH * 14 / 8)
#define CONFIG_SYS_NAND_OOBSIZE		224
#define CONFIG_SYS_NAND_ECCPOS {				\
		56 , 57 , 58 , 59 , 60 , 61 , 62 , 63 , \
		64 , 65 , 66 , 67 , 68 , 69 , 70 , 71 , \
		72 , 73 , 74 , 75 , 76 , 77 , 78 , 79 , \
		80 , 81 , 82 , 83 , 84 , 85 , 86 , 87 , \
		88 , 89 , 90 , 91 , 92 , 93 , 94 , 95 , \
		96 , 97 , 98 , 99 , 100, 101, 102, 103, \
		104, 105, 106, 107, 108, 109, 110, 111, \
		112, 113, 114, 115, 116, 117, 118, 119, \
		120, 121, 122, 123, 124, 125, 126, 127, \
		128, 129, 130, 131, 132, 133, 134, 135, \
		136, 137, 138, 139, 140, 141, 142, 143, \
		144, 145, 146, 147, 148, 149, 150, 151, \
		152, 153, 154, 155, 156, 157, 158, 159, \
		160, 161, 162, 163, 164, 165, 166, 167, \
		168, 169, 170, 171, 172, 173, 174, 175, \
		176, 177, 178, 179, 180, 181, 182, 183, \
		184, 185, 186, 187, 188, 189, 190, 191, \
		192, 193, 194, 195, 196, 197, 198, 199, \
		200, 201, 202, 203, 204, 205, 206, 207, \
		208, 209, 210, 211, 212, 213, 214, 215, \
		216, 217, 218, 219, 220, 221, 222, 223}
#endif
#endif /*CONFIG_MTD_NAND_JZ*/

/**
 * SPL configuration
 */
#define CONFIG_SPL
#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_STACK		0x80003000 /* 12K stack_size */
#define CONFIG_SPL_BSS_START_ADDR	0x80003000
#define CONFIG_SPL_BSS_MAX_SIZE		0x1000 /* stack_size + bss_size <= CONFIG_SYS_DCACHE_SIZE */

#define CONFIG_SPL_NO_CPU_SUPPORT_CODE
#define CONFIG_SPL_START_S_PATH		"$(CPUDIR)/$(SOC)"
#define CONFIG_SPL_LDSCRIPT		"$(CPUDIR)/$(SOC)/u-boot-spl.lds"

#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR	0x20 /* 16KB offset */
#define CONFIG_SYS_U_BOOT_MAX_SIZE_SECTORS	0x400 /* 512 KB */

#define CONFIG_SPL_BOARD_INIT
#define CONFIG_SPL_LIBGENERIC_SUPPORT
#define CONFIG_SPL_GPIO_SUPPORT
#define CONFIG_SPL_I2C_SUPPORT
#define CONFIG_SPL_REGULATOR_SUPPORT
#define CONFIG_SPL_CORE_VOLTAGE		1300

#ifdef CONFIG_SPL_MMC_SUPPORT

#define CONFIG_SPL_TEXT_BASE		0xf4000a00
#define CONFIG_SPL_MAX_SIZE		((16 * 1024) - 0xa00)

#define CONFIG_SPL_SERIAL_SUPPORT


#endif /* !CONFIG_SPL_MMC_SUPPORT */

#ifdef CONFIG_SPL_SPI_SUPPORT
#define CONFIG_SPL_PAD_TO		16384
#endif
#ifdef CONFIG_SPL_MMC_SUPPORT
#define CONFIG_SPL_PAD_TO		15872  /* u-boot start addr - mbr size(512) */
#endif
#ifdef CONFIG_JZ_NAND_MGR
#define CONFIG_CMD_ZM_NAND  /* nand zone manager support */
#define CONFIG_SPL_PAD_TO		16384
#endif
/**
* GPT configuration
 */
#ifdef CONFIG_GPT_CREATOR
#define CONFIG_GPT_TABLE_PATH   "$(TOPDIR)/board/$(BOARDDIR)"
#else
/**
 * MBR configuration
 */
/*#ifdef CONFIG_MBR_CREATOR*/
#define CONFIG_MBR_P0_OFF	56mb
#define CONFIG_MBR_P0_END	556mb
#define CONFIG_MBR_P0_TYPE 	linux

#define CONFIG_MBR_P1_OFF	580mb
#define CONFIG_MBR_P1_END 	1604mb
#define CONFIG_MBR_P1_TYPE 	linux

#define CONFIG_MBR_P2_OFF	28mb
#define CONFIG_MBR_P2_END	54mb
#define CONFIG_MBR_P2_TYPE 	linux

#define CONFIG_MBR_P3_OFF	1609mb
#define CONFIG_MBR_P3_END	7800mb
#define CONFIG_MBR_P3_TYPE 	fat
#endif

/**
 * Keys.
 */
#define CONFIG_GPIO_USB_DETECT		GPIO_PA(16)
#define CONFIG_GPIO_USB_DETECT_ENLEVEL	1

#define CONFIG_GPIO_RECOVERY		GPIO_PD(19)	/* SW7 */
#define CONFIG_GPIO_RECOVERY_ENLEVEL	0

#define CONFIG_GPIO_FASTBOOT		GPIO_PG(15)	/* SW2 */
#define CONFIG_GPIO_FASTBOOT_ENLEVEL	0

#define CONFIG_GPIO_MENU		CONFIG_GPIO_FASTBOOT
#define CONFIG_GPIO_MENU_ENLEVEL	CONFIG_GPIO_FASTBOOT_ENLEVEL

#define CONFIG_GPIO_VOL_SUB		GPIO_PD(17)	/* SW9 */
#define CONFIG_GPIO_VOL_SUB_ENLEVEL	1

#define CONFIG_GPIO_VOL_ADD		GPIO_PD(18)	/* SW8 */
#define CONFIG_GPIO_VOL_ADD_ENLEVEL	0

#define CONFIG_GPIO_BACK		GPIO_PD(19)	/* SW7 */
#define CONFIG_GPIO_BACK_ENLEVEL	0

#define CONFIG_GPIO_PWR_WAKE		GPIO_PA(30)
#define CONFIG_GPIO_PWR_WAKE_ENLEVEL	0

/* TEST
#define CONFIG_GPIO_DC_DETECT           GPIO_PG(10)
#define CONFIG_GPIO_DC_DETECT_ENLEVEL   1

#define CONFIG_GPIO_CHARGE_DETECT               GPIO_PG(12)
#define CONFIG_GPIO_CHARGE_DETECT_ENLEVEL       0
*/
#endif /* __CONFIG_MENSA_H__ */
