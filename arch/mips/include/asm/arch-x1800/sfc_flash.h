#ifndef __SFC_FLASH_H
#define __SFC_FLASH_H
#include <linux/mtd/mtd.h>
#include <asm/arch/sfc.h>

struct sfc_flash {
	struct sfc *sfc;
	struct mtd_info  *mtd;

	void *flash_info;
};

#endif
