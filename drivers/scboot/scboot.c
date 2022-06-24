#include <common.h>
#include <asm/io.h>
#include <asm/arch/cpm.h>
#include <asm/reboot.h>

#include "secall.h"
#include "pdma.h"
#include "otp.h"
#include "aes.h"

/*
 * security boot.
 * 1. prepare a bin encrypted.
 *		|---------------|-------------------|					|
 *		| SC KEY(2048)	|  CODE encrypted   |
 *		|---------------|-------------------|					|
 * 2.
 *
 * */

#undef TCSM_CODE_ADDR
#undef TCSM_SC_KEY_ADDR
#undef MCU_TCSM_RETVAL
#undef MCU_TCSM_SECALL_MSG

#ifdef CONFIG_X1000
#define TCSM_CODE_ADDR			(TCSM_BANK(1) + 0)
#define TCSM_SC_KEY_ADDR		(TCSM_BANK(1) + 2048)
#define MCU_TCSM_RETVAL			(TCSM_BANK(0) + 2048 + 1084) /* cal from sc_interface. */
#define MCU_TCSM_SECALL_MSG		(TCSM_BANK(0) + 2048 + 128) /* MCU_TCSM_SECALL_MSG */
#else
#define TCSM_CODE_ADDR			(TCSM_BANK(1) + 0)
#define TCSM_SC_KEY_ADDR		(TCSM_BANK(1) + 2048)
#define MCU_TCSM_RETVAL			(TCSM_BANK(0) + 2048 + 1076) /* cal from sc_interface. */
#define MCU_TCSM_SECALL_MSG		(TCSM_BANK(0) + 2048 + 128) /* MCU_TCSM_SECALL_MSG */
#endif

#define SC_MAX_SIZE_PERTIME		(2048)
#define SC_MAGIC_SIZE			(512)
#define SC_KEY_SIZE				(1536)

#define SECURE_SCBOOT_MAGIC		0x54424353

#define USERKEY_ENCRYPT  0x2
#define CHIPKEY_ENCRYPT  0x1

#define NKU_NKEY_WORD_OFF 2
#define NKU_KUKEY_WORD_OFF (2+32)

#define PARAM_N_WORD_OFF 64
#define PARAM_KU_WORD_OFF 128
#define PARAM_RSASHA_WORD_OFF 192

#define MCU_TCSM_SPLSHA1ENCBUF_SIZE 264

#define SCKEY_INFO 64

#define RSASHA_SIZE 32

extern void flush_cache_all(void);
extern void read_flash(unsigned int from, unsigned int len, unsigned char *buf);
extern inline phys_addr_t virt_to_phys(volatile void * address);

static void secure_check(void *addr, int *issig)
{
	int *ddrptr = (int *)(addr);
	int p = (unsigned long)SECURE_SCBOOT_MAGIC;

	if(*ddrptr++ == p) {
		*issig = 1;
	}
}

static int setup_sckeys(void *addr, unsigned int *len)
{
	volatile unsigned int *tcsmptr = (volatile unsigned int *)(TCSM_SC_KEY_ADDR);
	int iLoop = 0;
	int *ddrptr = (int *)(addr + SC_MAGIC_SIZE);

	/* 384 * 4 = 1536, sc_key */
	for (iLoop = 0; iLoop < SC_KEY_SIZE/4; iLoop++)
		tcsmptr[iLoop] = ddrptr[iLoop];

	*len = tcsmptr[0]; /* image length */

	/* len must 4 wrod align */
	if((*len) == 0 || (*len) % 16)
		return -1;

	return 0;
}

static int start_scboot(void *input, void *output, unsigned int binlen)
{
	volatile struct sc_args *args = (volatile struct sc_args *)(MCU_TCSM_SECALL_MSG);
	volatile unsigned int *tcsmptr = (volatile unsigned int *)(TCSM_CODE_ADDR);
	unsigned int ret;
	int iLoop = 0;
	int *srcptr = (int *)(input + SC_MAGIC_SIZE + SC_KEY_SIZE);
	int *dstptr = (int *)(output);

	{
		int newround = 1;
		int endround = 0;
		int pos = 0;

		boot_up_mcu();

		do {
			int lens = binlen > SC_MAX_SIZE_PERTIME ? SC_MAX_SIZE_PERTIME : binlen;

			if (binlen <= SC_MAX_SIZE_PERTIME)
				endround = 1;

			args->arg[0] = endround << 1 | newround;
			args->arg[1] = lens;

			for (iLoop = 0; iLoop < lens / 4; iLoop++)
				tcsmptr[iLoop] = srcptr[pos++];

			newround = 0;

			ret = secall(args, SC_FUNC_SCBOOT, 0, 1);

			for (iLoop = 0; iLoop < lens / 4; iLoop++)
				dstptr[pos - lens / 4 + iLoop] = tcsmptr[iLoop];

			binlen -= SC_MAX_SIZE_PERTIME;
		} while (!endround);

		ret = *(volatile unsigned int *)MCU_TCSM_RETVAL;
		ret &= 0xFFFF;

	}
	return ret;
}

static int scboot_boot_cmp_hash(void *input, void *output)
{
	volatile struct sc_args *args = (volatile struct sc_args *)(MCU_TCSM_SECALL_MSG);
	volatile unsigned int *splptr = (volatile unsigned int *)(SC_KEY_ADDR);
	int iLoop, key;
	int *srcptr = (int *)((int)(input + SC_MAGIC_SIZE + SC_KEY_SIZE));
	int *dstptr = (int *)output;
	int pasrcptr = (int *)((int)srcptr & (~(1 << 31)));
	int padstptr = (int *)((int)dstptr & (~(1 << 31)));
	int code_encrypt = splptr[1];
	int binlen = (splptr[0] + 15) & 0xFFFFFFF0;

	polling_done(args);

	//AES
	if (code_encrypt) {
		if (code_encrypt & CHIPKEY_ENCRYPT)
			key = AES_BY_CKEY;
		else if(code_encrypt & USERKEY_ENCRYPT)
			key = AES_BY_UKEY;
		else
			return SC_ERR_INVALID_SPLCRYPT;

			do_aes_dma((int *)pasrcptr,(int *)padstptr, binlen, key, 0);
			flush_cache_all();
	} else {
		for(iLoop = 0; iLoop < binlen / 4; iLoop++)
			dstptr[iLoop] = srcptr[iLoop];
	}

	do_md5 ((char *)dstptr, binlen, (unsigned char*)TCSM_CODE_ADDR);

	volatile unsigned int *rsarst = (volatile unsigned int *)(MCU_TCSM_RSAENCKEY);
	volatile unsigned int *rsasrc = (volatile unsigned int *)(TCSM_CODE_ADDR);

	if (cmp_data(rsarst, rsasrc, 4)) { /*128 for md5, 160 for sha1-1*/
		printf("Digital signature does not match!!!!\n");
		return SC_ERR_ILLEGAL_SPLSHA1;
	}
	return 0;
}

static void pdma_wait(void)
{
	__asm__ volatile (
		"	.set	push		\n\t"
		"	.set	noreorder	\n\t"
		"	.set	mips32		\n\t"
		"	li	$26, 0		\n\t"
		"	mtc0	$26, $12	\n\t"
		"	nop			\n\t"
		"1:				\n\t"
		"	wait			\n\t"
		"	b	1b		\n\t"
		"	nop			\n\t"
		"	.set	reorder		\n\t"
		"	.set	pop		\n\t"
		);
}

int secure_scboot(void *input, void *output)
{
	unsigned int ret = 0;
	unsigned int len;
	unsigned int *pdma_ins = (unsigned int *)pdma_wait;
	volatile unsigned int *pdma_bank0_off = (unsigned int *)TCSM_BANK0;
	int issig = 0;
	int tmp, i;

	/* start aes, pdma clk */
	tmp = cpm_readl(CPM_CLKGR);
	tmp &= ~(CPM_CLKGR_AES | CPM_CLKGR_PDMA);
	cpm_writel(tmp, CPM_CLKGR);

	for (i = 0; i < 6; i++)
		pdma_bank0_off[i] = pdma_ins[i];

	secure_check(input, &issig);

	if((EFUSTATE_SECBOOT_EN == 0) && (issig == 0)) {
		printf("Normal boot...\n");
	}
	else if(EFUSTATE_SECBOOT_EN && (issig == 0)) {
		printf("ERROR: please sign your image !!\n");
		while(1);
	}
	else if(issig == 1) {
		printf("Security boot...\n");
#ifndef CONFIG_SPL_BUILD
		ret = scboot_boot_cmp_hash(input, output);
		if(ret) {
			printf("ERROR: please check your image !!\n");
			hang();
		}
#else
		ret = setup_sckeys(input, &len);
		if(ret) {
			printf("ERROR: please check header information!!\n");
			hang();
		}
		ret = start_scboot(input, output, len);
		if(ret) {
			printf("ERROR: please check your image !!\n");
			hang();
		}
#endif
		memset(TCSM_SC_KEY_ADDR, 0, SC_KEY_SIZE);
		memset(MCU_TCSM_SPLSHA1ENCBUF, 0, RSASHA_SIZE);
		memset(MCU_TCSM_NKU, 0, MCU_TCSM_SPLSHA1ENCBUF_SIZE);
	}

	return ret;
}

int secure_boot_rsa_nku(unsigned int src_addr)
{
	volatile struct sc_args *args = (volatile struct sc_args *)(MCU_TCSM_SECALL_MSG);
	volatile unsigned int *tcsmptr = (volatile unsigned int *)(TCSM_SC_KEY_ADDR);
	volatile unsigned int *nkupaddr = (volatile unsigned int *)MCU_TCSM_NKU;
	volatile unsigned int *sharsabuf = (volatile unsigned int *)MCU_TCSM_SPLSHA1ENCBUF;
	unsigned int ret, iLoop, key;
	int *read_buf = NULL;

	read_buf = (int *)malloc(SC_KEY_SIZE);
	memset(read_buf, 0, SC_KEY_SIZE);
	read_flash(src_addr + SC_MAGIC_SIZE, SC_KEY_SIZE, read_buf);
	flush_cache_all();

	int nlen = (read_buf[2] / 8 + 15) & 0xFFFFFFF0;
	int kulen = (read_buf[3] / 8 + 15) & 0xFFFFFFF0;
	int key_encrypt = read_buf[4];
	int len = read_buf[0]; /* image length */
	nkupaddr[0] = read_buf[2];
	nkupaddr[1] = read_buf[3];
	tcsmptr[0] = read_buf[0];
	tcsmptr[1] = read_buf[1];

	int npaaddr = virt_to_phys(read_buf + PARAM_N_WORD_OFF);
	int kupaaddr = virt_to_phys(read_buf + PARAM_KU_WORD_OFF);

	/* len must 4 wrod align */
	if((len) == 0 || (len) % 16)
		return -1;

	for (iLoop = 0; iLoop < RSASHA_SIZE; iLoop++)
		sharsabuf[iLoop] = read_buf[iLoop + PARAM_RSASHA_WORD_OFF];

	if (key_encrypt) {
		if (key_encrypt & CHIPKEY_ENCRYPT)
			key = AES_BY_CKEY;
		else if(key_encrypt & USERKEY_ENCRYPT)
			key = AES_BY_UKEY;
		else
			return SC_ERR_INVALID_SPLCRYPT;

		do_aes_dma((int *)npaaddr,
			(int *)(MCU_TCSM_DDR(&nkupaddr[NKU_NKEY_WORD_OFF])), nlen, key, 0);

		do_aes_dma((int *)kupaaddr,
			(int *)(MCU_TCSM_DDR(&nkupaddr[NKU_KUKEY_WORD_OFF])), kulen, key, 0);
	}

	flush_cache_all();

	args->arg[0] = MCU_TCSM_PADDR(nkupaddr);
	secall(args, SC_FUNC_CHECKNKU, 0, 1);

	free(read_buf);
	read_buf = NULL;

	ret = *(volatile unsigned int *)MCU_TCSM_RETVAL;
	ret &= 0xFFFF;
	if (ret) {
		return ret;
	}

	args->arg[0] = nkupaddr[0] / 32;
	args->arg[1] = nkupaddr[1] / 32;
	args->arg[2] = nkupaddr[0] / 32;
	args->arg[3] = &(args->arg[3]);
	args->arg[4] = MCU_TCSM_PADDR(sharsabuf);
	args->arg[5] = MCU_TCSM_PADDR(&nkupaddr[NKU_KUKEY_WORD_OFF]);
	args->arg[6] = MCU_TCSM_PADDR(&nkupaddr[NKU_NKEY_WORD_OFF]);
	args->arg[7] = MCU_TCSM_PADDR(MCU_TCSM_RSAENCKEY);

	//RSA
	secall(args, SC_FUNC_RSA, 0, 0);


	return 0;
}
