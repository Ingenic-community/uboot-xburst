#ifndef SFC_COMMON_H
#define SFC_COMMON_H
#include <common.h>
#include <asm/arch/sfc.h>
#include <asm/arch/sfc_flash.h>

void dump_sfc_reg(struct sfc *sfc);

void sfc_list_init(struct sfc_transfer *);
void sfc_list_add_tail(struct sfc_transfer *, struct sfc_transfer *);
int32_t sfc_sync(struct sfc *sfc, struct sfc_transfer *);
struct sfc *sfc_res_init(uint32_t);

uint32_t sfc_get_sta_rt(struct sfc *);
int32_t set_flash_timing(struct sfc *, uint32_t, uint32_t, uint32_t, uint32_t);

int32_t sfc_nor_get_special_ops(struct sfc_flash *);

#endif
