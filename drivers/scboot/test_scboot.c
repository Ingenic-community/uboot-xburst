#include <common.h>
#include <asm/io.h>

#include "secall.h"
#include "pdma.h"

//#include "sc_firmware/aes.h"
//#include "sc_firmware/otp.h"
//#include "sc_firmware/scram.h"
//#include "sc_firmware/hash.h"

//#include "splbin.h"
//#include "x1000-splbin.h"
//#include "x1000-splbin-msc.h"
#include "x1000-splbin-sfc.h"
/*
 * test security boot.
 * 1. prepare a splbin encrypted.
 *		|---------------|-------------------|					|
 *		| SC KEY(2048)	| SPL encrypted(12K)|
 *		|---------------|-------------------|					|
 * 2.
 *
 * */



#define TCSM_SPL_CODE_ADDR           (TCSM_BANK(1) + 0)
#define TCSM_SC_KEY_ADDR             (TCSM_BANK(1) + 2048)
#define MCU_TCSM_RETVAL         (TCSM_BANK(0) + 2048 + 1084) /* cal from sc_interface. */
#define MCU_TCSM_SECALL_MSG     (TCSM_BANK(0) + 2048 + 128)  /* MCU_TCSM_SECALL_MSG */
#define SC_MAX_SIZE_PERTIME     (2048)

struct spl_struct {
	int len;
	int enc_type;

	int n_bit;
	int ku_bit;
	int rsa_enc_type;
	/* pad to 2048bit.:256Bytes:64words*/
	int reserved_0[64 - 5];

	int key_n[32];
	int reserved_1[64 - 32];
	/*pad to 2048bit:256Bytes*/
	int key_ku[32];
	/*pad to 2048bit:256Bytes*/
	int reserved_2[64 - 32];

	int spl_md5_sig_kr_enc[32];
	/*pad to 2048bit:256Bytes*/

	/*SPL BIN ....*/
};

static int parse_sckey(unsigned char *p)
{
	struct spl_struct *spl = p;
	int i;
	printf("spl->len:		%d\n", spl->len);
	printf("spl->enc_type:	%d\n", spl->enc_type);
	printf("spl->n_bit:		%d\n", spl->n_bit);
	printf("spl->ku_bit:	%d\n", spl->ku_bit);
	for(i=0;i<32;i++) {
		printf("spl->key_n[%d]:0x%08x\n",i, spl->key_n[i]);
	}
	for(i=0;i<32;i++) {
		printf("spl->key_ku[%d]:0x%08x\n", i, spl->key_ku[i]);
	}

	for(i=0; i<32;i++) {
		printf("spl->spl_md5_sig_kr_enc[%d]:0x%08x\n",
				i, spl->spl_md5_sig_kr_enc[i]);
	}
}



static int setup_sckeys(void *spladdr)
{
	unsigned int ret;

	volatile unsigned int *spltcsmptr = (unsigned int *)TCSM_SC_KEY_ADDR;
	unsigned int *spl_sckey_addr = (unsigned int *)spladdr;
	int iLoop = 0;

	printf("setup sckeys from :%x, to:%x\n", spl_sckey_addr, spltcsmptr);

	unsigned int *d = (unsigned char *)(TCSM_BANK0);
	int i;
	//for(i = 0; i<256; i++) {
		//printf("d:%08x, [%d]:0x%08x\n",&d[i], i, d[i]);
	//}

	for(iLoop = 0; iLoop < 1536/4; iLoop++) {
		spltcsmptr[iLoop] = spl_sckey_addr[iLoop];
	}
	parse_sckey(spl_sckey_addr);
	//for(i=0; i<32; i++) {
		//printf("spl sc key[%d]:0x%08x\n", i, spltcsmptr[i]);
	//}

	//for(i = 0; i<256; i++) {
		//printf("d:%08x, [%d]:0x%08x\n",&d[i], i, d[i]);
	//}
	return 0;
}


static int start_scboot(void *spladdr, int spllen)
{
	volatile struct sc_args *args;
	args = (volatile struct sc_args*)MCU_TCSM_SECALL_MSG;

	int iLoop = 0;
	volatile unsigned int *spltcsmptr = TCSM_SPL_CODE_ADDR;
	unsigned int *splddrptr = spladdr + 1536;

	int splbinlen = spllen - 1536; /* 512 head + 1536 sckeysize*/
	int newround = 1;
	int endround = 0;
	int pos = 0;
	int ret = 0;

	printf("start scboot, spladdr :0x%x, splddrptr:0x%08x, spllen:0x%x, splbinlen:0x%x\n", spladdr,splddrptr, spllen, splbinlen);
	unsigned int *output = (unsigned int *)MCU_TCSM_OUTDATA;
	int i;
	while(splbinlen > 0) {
		int lens = splbinlen > SC_MAX_SIZE_PERTIME ? SC_MAX_SIZE_PERTIME: splbinlen;

		if(splbinlen <= SC_MAX_SIZE_PERTIME) {
			endround = 1;
		}

		args->arg[0] = endround << 1 | newround;
		args->arg[1] = lens;
		for (iLoop = 0; iLoop < lens / 4; iLoop++){
			spltcsmptr[iLoop] = splddrptr[pos++];
			//spltcsmptr[iLoop] = iLoop;
		}

		printf("splbinlen is :%d, new round:%d, end round:%d, len:%d\n", splbinlen, newround, endround, lens);
		newround = 0;
		ret = secall(args, SC_FUNC_SCBOOT, 0, 1);
		if(ret != 0x0e000000) {
			printf("sc boot test faled.ret:%08x, %08x\n", ret, *(volatile unsigned int *)MCU_TCSM_RETVAL);
		} else {
			printf("wait for secall exec sucess!:%08x, %08x\n", ret, *(volatile unsigned int*)MCU_TCSM_RETVAL);
		}
		splbinlen -= SC_MAX_SIZE_PERTIME;

		for(i=0;i<8;i++) {
			printf("output[%d] :0x%08x\n", i, output[i]);
		}
	}

	ret = *(volatile unsigned int *)MCU_TCSM_RETVAL;
	ret &= 0xffff;
	printf("scboot ret is :%08x, should be 0\n", ret);
	if(ret == 0x00000000) {
		printf("####################scboot test success!!!!##################\n");
	}

}

int test_scboot(void)
{
	volatile struct sc_args *args;
	args = (volatile struct sc_args*)MCU_TCSM_SECALL_MSG;

	unsigned int *xx_splbin = splbin;
	printf("xx_splbint  1111:%08x\n", xx_splbin);
	xx_splbin = (unsigned int)xx_splbin + 512;
	printf("xx_splbin 22222:%08x\n", xx_splbin);
	setup_sckeys(xx_splbin);
	printf("setup sckeys to tcsm ok!\n");

	if(start_scboot(xx_splbin, sizeof(splbin) - 512) != 0) {
		printf("scboot failed !!!!!\n");
		return -1;
	}

	return 0;
}
