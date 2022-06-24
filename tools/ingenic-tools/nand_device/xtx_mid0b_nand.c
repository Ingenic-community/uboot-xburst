#include <stdio.h>
#include "nand_common.h"

#define XTX_0B_MID			    0x0B

#define XTX_0B_NAND_DEVICD_COUNT	    1

static unsigned char xtx_0b_xaw[] = {0x04,0x07};

static struct device_struct device[1] = {
	DEVICE_STRUCT(0xF2, 2048, 2, 4, 3, 1, xtx_0b_xaw),
};

static struct nand_desc xtx_0b_nand = {

	.id_manufactory = XTX_0B_MID,
	.device_counts  = XTX_0B_NAND_DEVICD_COUNT,
	.device = device,
};

int xtx_mid0b_nand_register_func(void) {
	return nand_register(&xtx_0b_nand);
}
