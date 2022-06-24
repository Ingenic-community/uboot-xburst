/*
 *Ingenic mensa boot android system command
 *
 * Copyright (c) 2013 Imagination Technologies
 * Author: Martin <czhu@ingenic.cn>
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

#include <stdarg.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <errno.h>
#include <div64.h>
#include <common.h>
#include <command.h>
#include <config.h>
#include <mmc.h>
#include <boot_img.h>
#include <asm/arch/sfc.h>
#include <asm/arch/spinor.h>

extern void flush_cache_all(void);


/*boot.img has been in memory already. just call init_boot_linux() and jump to kernel.*/
static void bootx_jump_kernel(unsigned long mem_address)
{
	static u32 *param_addr = NULL;
	typedef void (*image_entry_arg_t)(int, char **, void *)
		__attribute__ ((noreturn));

	image_entry_arg_t image_entry =
		(image_entry_arg_t) mem_address;

	printf("Prepare kernel parameters ...\n");
	param_addr = (u32 *)CONFIG_PARAM_BASE;
	param_addr[0] = 0;
	param_addr[1] = CONFIG_BOOTX_BOOTARGS;
	flush_cache_all();
	image_entry(2, (char **)param_addr, NULL);

	printf("We should not come here ... \n");
}

/* boot the android system form the memory directly.*/
static int mem_bootx(unsigned int mem_address)
{
	printf("Enter mem_boot routine ...\n");
	bootx_jump_kernel(mem_address);
	return 0;
}
#ifdef CONFIG_JZ_SFC
static void sfc_boot(unsigned int mem_address,unsigned int sfc_addr)
{
	struct image_header *header;
	unsigned int header_size;
	unsigned int entry_point, load_addr, size;

	printf("Enter SFC_boot routine ...%x\n", sfc_addr);
#ifdef CONFIG_OTA_VERSION20
	{
		struct norflash_partitions partition;
		unsigned int user_offset = 0, updatefs_size = 0;
		int i;
		if(sfc_addr == 0) {
			sfc_nor_read(CONFIG_SPI_NORFLASH_PART_OFFSET,
				     sizeof(struct norflash_partitions), &partition);
			for (i = 0 ; i < partition.num_partition_info; i ++) {
				if (!strncmp(partition.nor_partition[i].name,
					     CONFIG_PAT_USERFS_NAME, sizeof(CONFIG_PAT_USERFS_NAME))) {
					user_offset = partition.nor_partition[i].offset;
				}
				if (!strncmp(partition.nor_partition[i].name,
					     CONFIG_PAT_UPDATEFS_NAME, sizeof(CONFIG_PAT_UPDATEFS_NAME))) {
					updatefs_size = partition.nor_partition[i].size;
				}
			}
			sfc_addr = user_offset + updatefs_size + 1024*1024;
		}
	}
#endif
	header_size = sizeof(struct image_header);
	sfc_nor_read(sfc_addr, header_size, CONFIG_SYS_TEXT_BASE);
	header = (struct image_header *)(CONFIG_SYS_TEXT_BASE);

	entry_point = image_get_load(header);
	/* Load including the header */
	load_addr = entry_point - header_size;
	size = image_get_data_size(header) + header_size;

	sfc_nor_read(sfc_addr, size, load_addr);

	bootx_jump_kernel(mem_address);
}
#endif
static int do_bootx(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned long mem_address,sfc_addr, size;
	int i;
	/* Consume 'bootx' */
	argc--; argv++;
	if (argc < 2)
		return CMD_RET_USAGE;

	if (!strcmp("mem",argv[0])) {
		mem_address=simple_strtoul(argv[1], NULL, 16);
		printf("mem boot start\n");
		mem_bootx(mem_address);
		printf("mem boot error\n");
	} else if (!strcmp("sfc",argv[0])) {
		mem_address = simple_strtoul(argv[1], NULL, 16);
		if(argc == 3)
			sfc_addr = simple_strtoul(argv[2], NULL, 16);
		else
			sfc_addr = 0;

		printf("SFC boot start\n");
#ifdef CONFIG_JZ_SFC
		sfc_boot(mem_address, sfc_addr);
#endif
		printf("SFC boot error\n");
		return 0;
	} else {
		printf("%s boot unsupport\n", argv[0]);
                return CMD_RET_USAGE;
	}
	return 0;
}

#ifdef CONFIG_SYS_LONGHELP
static char bootx_help_text[] =
        "[[way],[mem_address],[offset]]\n"
        "- boot Android system....\n"
        "\tThe argument [way] means the way of booting boot.img.[way]='mem'/'sfc'.\n"
        "\tThe argument [mem_address] means the start position of xImage in memory.\n"
        "\tThe argument [offset] means the position of xImage in sfc-nor.\n"
        "";
#endif

U_BOOT_CMD(
	bootx, 5, 1, do_bootx,
	"boot xImage ",bootx_help_text
);
