#ifndef __JZ_NAND_H__
#define __JZ_NAND_H__

#include <ingenic_nand_mgr/nand_param.h>

struct jz_nand_priv {
	int pn_bytes;
	int pn_size;
	int pn_skip;
};

static inline void dump_nand_info(struct nand_chip *nand_chip)
{
	struct nand_ecclayout *nand_ecclayout = nand_chip->ecc.layout;
	int i;

	debug("nand_chip->ecc.size = %d\n", nand_chip->ecc.size);
	debug("nand_chip->ecc.strength = %d\n", nand_chip->ecc.strength);
	debug("nand_chip->ecc.bytes = %d\n", nand_chip->ecc.bytes);
	debug("nand_chip->ecc.steps = %d\n", nand_chip->ecc.steps);
	debug("nand_chip->IO_ADDR_R = %p\n", nand_chip->IO_ADDR_R);
	debug("nand_ecclayout->eccbytes = %d\n", nand_ecclayout->eccbytes);
	debug("nand_ecclayout->oobavail = %d\n", nand_ecclayout->oobavail);
	for (i = 0; i < nand_ecclayout->eccbytes; i++) {
		debug("eccpos[%d] = %d\n", i, nand_ecclayout->eccpos[i]);
	}
	debug("nand_ecclayout->oobfree[0].offset = %d\n", nand_ecclayout->oobfree[0].offset);
	debug("nand_ecclayout->oobfree[0].length = %d\n", nand_ecclayout->oobfree[0].length);
}

extern void jz_nand_enable_pn(int enable, bool force);
extern void jz_nand_select_chip(struct mtd_info *mtd, int select);
extern int jz_nand_device_ready(struct mtd_info *mtd);
extern void jz_nand_cmd_ctrl(struct mtd_info *mtd, int cmd, unsigned int ctrl);
extern int jz_nand_init(struct nand_chip *nand, nand_timing_param *param, int buswidth);
extern void jz_nand_hwctl(struct mtd_info *mtd, int mode);
extern int jz_nand_calculate_ecc(struct mtd_info *mtd, const u_char *dat, u_char *ecc_code);
extern int jz_nand_correct_data(struct mtd_info *mtd, u_char *dat, u_char *read_ecc, u_char *calc_ecc);
extern int jz_nand_ecc_init(struct mtd_info *mtd, struct nand_chip *nand);
extern int jz_get_nand_id(void);
#endif /*__JZ_NAND_H__*/
