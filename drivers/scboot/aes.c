#include <common.h>
#include <asm/io.h>
#include <asm/errno.h>
#include "secall.h"
#include "pdma.h"
#include "aes.h"


int aes(void *bininput, void *binoutput, unsigned int len, unsigned int key, unsigned int crypt)
{
	unsigned int ret;
	volatile struct sc_args *args;
	args = (volatile struct sc_args *)GET_SC_ARGS();
	volatile unsigned int *input = (volatile unsigned int *)(MCU_TCSM_INDATA);
	volatile unsigned int *output = (volatile unsigned int *)(MCU_TCSM_OUTDATA);

	memcpy(input, bininput, len);

	args->arg[0] = key | crypt;
	args->arg[2] = MCU_TCSM_PADDR(input); //input
	args->arg[3] = MCU_TCSM_PADDR(output); //output
	args->arg[4] = len; //len

	ret = secall(args, SC_FUNC_AESBYKEY, 0, 1);

	memcpy(binoutput, output, len);

	return 0;
}


int do_aes(void *binaddr, int len, int aeskey, int aescrypt)
{
	unsigned int iLoop = 0;
	unsigned int size = len;
	void *addr = binaddr;

	printf("tranfser size = %x\n", size);

	/*aes one time max 512 bytes (128 word) */
	for(iLoop = 0; iLoop * 512 < size; iLoop++) {
		if((size - 512 * iLoop) > 512)
			aes(addr + 512 * iLoop, addr + 512 * iLoop, 512, aeskey, aescrypt);
		else
			aes(addr + 512 * iLoop, addr + 512 * iLoop, (size - 512 * iLoop), aeskey, aescrypt);
	}

	return 0;
}

void do_aes_dma(void *in_paaddr, void *out_paaddr, int len, int aeskey, int aescrypt)
{

	volatile struct sc_args *args = (volatile struct sc_args *)(MCU_TCSM_SECALL_MSG);

	boot_up_mcu();

	args->arg[2] = in_paaddr;
	args->arg[3] = out_paaddr;
	args->arg[4] = len;

	args->arg[0] = 0;
	args->arg[0] |= aeskey | AES_CRYPT | AES_MODE;

	secall(args, SC_FUNC_AESBYKEY, 0, 1);

	if (aescrypt) {

		args->arg[0] = 0;
		args->arg[0] |= AES_BY_CKEY | AES_MODE;

		secall(args, SC_FUNC_AESBYKEY, 0, 1);
	}
}
