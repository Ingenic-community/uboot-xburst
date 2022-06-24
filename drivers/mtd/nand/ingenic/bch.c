#include <common.h>
#include <malloc.h>
#include <errno.h>
#include <nand.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <asm/arch/nand.h>
#include <asm/io.h>

struct bch_params {
	int size;
	int bytes;
	int strength;
};

static void bch_init(struct bch_params *param, int encode)
{
	uint32_t reg;

	/* clear & completion & error interrupts */
	reg = readl(BCH_BASE + BCH_BHINT);
	writel(reg, BCH_BASE + BCH_BHINT);

	/* setup BCH count register */
	reg = param->size << BCH_BHCNT_BLOCKSIZE_SHIFT;
	reg |= param->bytes << BCH_BHCNT_PARITYSIZE_SHIFT;
	writel(reg, BCH_BASE + BCH_BHCNT);

	/* setup BCH control register */
	reg = BCH_BHCR_BCHE | BCH_BHCR_INIT;
	reg |= param->strength << BCH_BHCR_BSEL_SHIFT;
	if (encode)
		reg |= BCH_BHCR_ENCE;
	writel(reg, BCH_BASE + BCH_BHCR);
}
static void bch_disable(void)
{
	uint32_t reg;
	reg = readl(BCH_BASE + BCH_BHINT);
	writel(reg, BCH_BASE + BCH_BHINT);
	writel(BCH_BHCR_BCHE, BCH_BASE + BCH_BHCCR);
}

#ifndef CONFIG_SPL_BUILD
static void bch_caculate(struct bch_params *param, const u_char *dat, u_char * ecc_code)
{
	int i;
	uint32_t status;

	bch_init(param, 1);

	/* write data */
	for (i = 0; i < param->size; i++)
		writeb(dat[i], BCH_BASE + BCH_BHDR);

	/* wait for completion */
	do {
		status = readl(BCH_BASE + BCH_BHINT);
	} while (!(status & BCH_BHINT_ENCF));
	writel(status, BCH_BASE + BCH_BHINT);

	/* read back parity data */
	for (i = 0; i < param->bytes; i++)
		ecc_code[i] = readb(BCH_BASE + BCH_BHPAR0 + i);

	/* disable BCH */
	bch_disable();
}
#endif

static int bch_correct(struct bch_params *param, u_char *dat, u_char *read_ecc)
{
	uint32_t status;
	int i, ret = -EBADMSG;

	bch_init(param, 0);

	/* write data */
	for (i = 0; i < param->size; i++)
		writeb(dat[i], BCH_BASE + BCH_BHDR);

	/* write ECC */
	for (i = 0; i < param->bytes; i++)
		writeb(read_ecc[i], BCH_BASE + BCH_BHDR);

	/* wait for completion */
	do {
		status = readl(BCH_BASE + BCH_BHINT);
	} while (!(status & BCH_BHINT_DECF));
	writel(status, BCH_BASE + BCH_BHINT);

	/* check status */
	if (status & BCH_BHINT_UNCOR) {
		printf("uncorrectable ecc error\n");
		goto out;
	}

	ret = 0;
	/* correct any detected errors */
	if (status & BCH_BHINT_ERR) {
		int err_count = (status & BCH_BHINT_ERRC_MASK) >> BCH_BHINT_ERRC_SHIFT;
		ret = (status & BCH_BHINT_TERRC_MASK) >> BCH_BHINT_TERRC_SHIFT;
		for (i = 0; i < err_count; i++) {
			uint32_t err_reg = readl(BCH_BASE + BCH_BHERR0 + (i * 4));
			uint32_t mask = (err_reg & BCH_BHERRn_MASK_MASK) >> BCH_BHERRn_MASK_SHIFT;
			uint32_t index = (err_reg & BCH_BHERRn_INDEX_MASK) >> BCH_BHERRn_INDEX_SHIFT;
			dat[(index * 2) + 0] ^= mask;
			dat[(index * 2) + 1] ^= mask >> 8;
		}
	}
out:
	/* disable BCH */
	bch_disable();
	return ret;
}

void jz_nand_hwctl(struct mtd_info *mtd, int mode) {}

int jz_nand_calculate_ecc(struct mtd_info *mtd, const u_char *dat, u_char *ecc_code)
{
#ifndef CONFIG_SPL_BUILD
	struct nand_chip *this = mtd->priv;
	struct bch_params param;

	if (this->state == FL_READING)
		return 0;
	param.size = this->ecc.size;
	param.bytes =  this->ecc.bytes;
	param.strength = this->ecc.strength;
	bch_caculate(&param, dat, ecc_code);
#endif
	return 0;
}

int jz_nand_correct_data(struct mtd_info *mtd, u_char *dat,
				   u_char *read_ecc, u_char *calc_ecc)
{
	struct nand_chip *this = mtd->priv;
	struct bch_params param;

	param.size = this->ecc.size;
	param.bytes =  this->ecc.bytes;
	param.strength = this->ecc.strength;
	return bch_correct(&param, dat, read_ecc);
}

#ifdef CONFIG_SPL_BUILD
int jz_spl_bch64_correct(u_char *dat,u_char *read_ecc)
{
	struct bch_params param;
	param.size = 256;
	param.bytes = 112;	/* 14*64/8 */
	param.strength = 64;
	return bch_correct(&param, dat, read_ecc);
}
#endif
