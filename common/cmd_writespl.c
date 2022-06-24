/*
 * CI20 SPL write command
 *
 * Copyright (c) 2013 Imagination Technologies
 * Author: Paul Burton <paul.burton@imgtec.com>
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

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <nand.h>
#include <asm/io.h>
#include <asm/arch/nand.h>

#define SPL_SIZE CONFIG_SPL_PAD_TO
#define SPL_BLOCKSIZE 256
#define SPL_ECCSIZE 112

extern void jz_nand_set_pn(nand_info_t *nand, int bytes, int size, int skip);

static void calculate_bch64(uint8_t *data, uint8_t *ecc)
{
	uint32_t reg;
	int i;

	/* clear & completion & error interrupts */
	reg = BCH_BHINT_ENCF | BCH_BHINT_DECF |
		  BCH_BHINT_ERR | BCH_BHINT_UNCOR;
	writel(reg, BCH_BASE + BCH_BHINT);

	/* setup BCH count register */
	reg = SPL_BLOCKSIZE << BCH_BHCNT_BLOCKSIZE_SHIFT;
	reg |= SPL_ECCSIZE << BCH_BHCNT_PARITYSIZE_SHIFT;
	writel(reg, BCH_BASE + BCH_BHCNT);

	/* setup BCH control register */
	reg = BCH_BHCR_BCHE | BCH_BHCR_INIT | BCH_BHCR_ENCE;
	reg |= 64 << BCH_BHCR_BSEL_SHIFT;
	writel(reg, BCH_BASE + BCH_BHCR);

	/* write data */
	for (i = 0; i < SPL_BLOCKSIZE; i++)
		writeb(data[i], BCH_BASE + BCH_BHDR);
	/* wait for completion */
	while (!(readl(BCH_BASE + BCH_BHINT) & BCH_BHINT_ENCF));

	/* clear interrupts */
	writel(readl(BCH_BASE + BCH_BHINT), BCH_BASE + BCH_BHINT);

	/* read back parity data */
	for (i = 0; i < SPL_ECCSIZE; i++)
		ecc[i] = readb(BCH_BASE + BCH_BHPAR0 + i);
	/* disable BCH */
	writel(BCH_BHCR_BCHE, BCH_BASE + BCH_BHCCR);
}

static int raw_nand_write(nand_info_t *nand, uint32_t off, uint8_t *data, size_t size)
{
	mtd_oob_ops_t ops = {
		.datbuf = data,
		.oobbuf = NULL,
		.len = size,
		.ooblen = 0,
		.mode = MTD_OPS_RAW
	};

	return mtd_write_oob(nand, off, &ops);
}

static int do_writespl(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	nand_info_t *nand = &nand_info[nand_curr_device];
	unsigned int pagesize = (*nand).writesize;
	unsigned int spl_pages = ((SPL_SIZE + pagesize -1)/pagesize);
	unsigned int spl_blocksperpage = (pagesize/SPL_BLOCKSIZE);

	int page, block, err, copies = 8, start_index = 0, ret = CMD_RET_FAILURE;
	uint32_t nand_off = 0, is_spl = 0;
	uint8_t *input, *in_ptr, *ecc_buf;
	copies = (int)simple_strtoul(argv[--argc], NULL, 10);
	if (argc > 2)
		start_index = (int)simple_strtoul(argv[--argc], NULL, 10);
	if (argc > 1)
		input = (uint8_t *)simple_strtoul(argv[--argc], NULL, 16);
	if (argc != 1)
		return CMD_RET_USAGE;

	if (start_index + 1 + copies > 8)
		copies = 8 - start_index;
	if (start_index == 0)
		is_spl = 1;
	printf("write spl(%x) to (%d-%d)nand zone\n", input, start_index, (start_index + copies));
	ecc_buf = malloc(pagesize);
	if (!ecc_buf) return -ENOMEM;

	nand_off = start_index * 128 * pagesize;
	for (in_ptr = input; copies--; in_ptr = input) {
		for (page = 0; page < spl_pages; page++) {
			printf("writing code page %d to 0x%x from 0x%p,pagesize=%x, ecc size %d\n",
					page, nand_off, in_ptr, pagesize, spl_blocksperpage *SPL_ECCSIZE);
			/* write code */
			if (is_spl)
				jz_nand_set_pn(nand, pagesize, 256, page ? 0 : 1);
			else
				jz_nand_set_pn(nand, pagesize, 0,  0);

			err = raw_nand_write(nand, nand_off, in_ptr, pagesize);
			if (err) {
				printf("error %d writing code\n", err);
				goto out;
			}
			nand_off += pagesize;

			/* calculate ECC */
			for (block = 0; block < spl_blocksperpage; block++) {
				calculate_bch64(in_ptr, &ecc_buf[block * SPL_ECCSIZE]);
				in_ptr += SPL_BLOCKSIZE;
			}
			memset(&ecc_buf[spl_blocksperpage * SPL_ECCSIZE], 0,
				   pagesize - (spl_blocksperpage * SPL_ECCSIZE));

			/* write ECC */
			jz_nand_set_pn(nand, pagesize, 0, 0);
			err = raw_nand_write(nand, nand_off, ecc_buf, pagesize);
			if (err) {
				printf("error %d writing ECC\n", err);
				goto out;
			}
			nand_off += pagesize;
		}

		/* advance to the next 128 page boundary */
		nand_off += (128 - (spl_pages * 2)) * pagesize;
	}
	ret = CMD_RET_SUCCESS;
out:
	jz_nand_set_pn(nand, 0, 0, 0);
	free(ecc_buf);
	return ret;
}

U_BOOT_CMD(
	writespl, 4, 1, do_writespl,
	"write a new SPL to NAND flash",
	"address [start_index] copies"
);
