#include <stdio.h>
#include "nand_common.h"

#define ZETTA_MID		    0xBA
#define ZETTA_NAND_DEVICD_COUNT	    2

static unsigned char zetta_xaw[] = {0x2};

static struct device_struct device[] = {
	DEVICE_STRUCT(0x71, 2048, 2, 4, 2, 1, zetta_xaw),
	DEVICE_STRUCT(0x72, 2048, 2, 4, 2, 1, zetta_xaw),
};

static struct nand_desc zetta_nand = {

	.id_manufactory = ZETTA_MID,
	.device_counts = ZETTA_NAND_DEVICD_COUNT,
	.device = device,
};

int zetta_nand_register_func(void) {
	return nand_register(&zetta_nand);
}
