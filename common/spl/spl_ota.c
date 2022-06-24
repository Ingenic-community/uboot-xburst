#include <common.h>
#include <config.h>
#include <spl.h>
#include <asm/io.h>
#include <errno.h>
#include <linux/err.h>
#include <malloc.h>
#include <div64.h>
#include <asm/arch/spinand.h>

#include "./spl_ota.h"

#define OTA_RAMDISK_OUT

#define UPDATE_FINISHING 0x1
#define UPDATE_KERNEL_FS 0x9
#define UPDATE_RECOVERY 0xd
#define UPDATE_WRITE_NEW_RECOVERY 0xf

extern void nv_map_area(unsigned int *base_addr, unsigned int nv_addr, unsigned int nv_size);
extern void spl_load_kernel(long offset);


static struct ota_ops *ota_ops = NULL;

void register_ota_ops(struct ota_ops *ops)
{
	ota_ops = ops;
}

static int ota_init(void)
{
	if (ota_ops->flash_init) {
		ota_ops->flash_init();
	}
}

char* spl_ota_load_image(void)
{
	char *cmdargs = NULL;
	unsigned int src_addr, updata_flag = 0;
	unsigned int nv_buf[2] = {0};
	int count = 8;
	unsigned int bootimg_addr = 0;
	struct jz_sfcnand_burner_param param;
	struct jz_sfcnand_partition partition;

	ota_init();

	nv_map_area(&src_addr, CONFIG_NVRW_ADDR, CONFIG_NVRW_SIZE);

	ota_ops->flash_read(src_addr, count, nv_buf);

	updata_flag = nv_buf[1];

#ifndef OTA_RAMDISK_OUT
	if((updata_flag == UPDATE_RECOVERY) || (updata_flag == UPDATE_KERNEL_FS) || (updata_flag == UPDATE_FINISHING)) {
		bootimg_addr = CONFIG_RECOVERY_ADDR;
		cmdargs = CONFIG_SYS_SPL_OTA_ARGS_ADDR;
	} else if (updata_flag == UPDATE_WRITE_NEW_RECOVERY) {
		bootimg_addr = CONFIG_SYSTEM_ADDR;
		cmdargs = CONFIG_SYS_SPL_OTA_ARGS_ADDR;
	} else {
		bootimg_addr = CONFIG_KERNEL_ADDR;
		cmdargs = CONFIG_SYS_SPL_ARGS_ADDR;
	}
#else
	if((updata_flag == UPDATE_RECOVERY) || (updata_flag == UPDATE_KERNEL_FS) || (updata_flag == UPDATE_FINISHING)) {
		bootimg_addr = CONFIG_RECOVERY_ADDR;
		cmdargs = CONFIG_SYS_SPL_OTA_ARGS_ADDR;
		ota_ops->flash_read(CONFIG_RAMDISK_ADDR, CONFIG_RAMDISK_SIZE, CONFIG_RAMDISK_LOAD_ADDR);
	} else if (updata_flag == UPDATE_WRITE_NEW_RECOVERY) {
		bootimg_addr = CONFIG_KERNEL_ADDR;
		cmdargs = CONFIG_SYS_SPL_OTA_ARGS_ADDR;
		ota_ops->flash_read(CONFIG_SYSTEM_ADDR, CONFIG_RAMDISK_SIZE, CONFIG_RAMDISK_LOAD_ADDR);
	} else {
		bootimg_addr = CONFIG_KERNEL_ADDR;
		cmdargs = CONFIG_SYS_SPL_ARGS_ADDR;
	}
#endif
	spl_load_kernel(bootimg_addr);

	return cmdargs;
}
