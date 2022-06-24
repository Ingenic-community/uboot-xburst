#include <common.h>
#include <asm/io.h>
#include <asm/errno.h>

#include "secall.h"
#include "pdma.h"

struct rsa_param {
	unsigned int input_len;
	unsigned int key_len;
	unsigned int n_len;
	unsigned int out_len;

	unsigned int *input;
	unsigned int *key;		/*KU*/
	unsigned int *n;		/*N*/
	unsigned int *output;
};

int do_rsa(unsigned int *idata,unsigned int inputlen,unsigned int *odata,unsigned int *rsakey,unsigned int rsakeylen)
{
	unsigned int ret, iLoop;
	unsigned int *data = idata;
	volatile struct sc_args *args;
	args = (volatile struct sc_args *)GET_SC_ARGS();

	volatile unsigned int *input = (volatile unsigned int *)(MCU_TCSM_INDATA);
	volatile unsigned int *key = (volatile unsigned int *)(MCU_TCSM_INDATA + 0x80);
	volatile unsigned int *n = (volatile unsigned int *)(MCU_TCSM_INDATA + 0x100);
	volatile unsigned int *output = (volatile unsigned int *)(MCU_TCSM_INDATA + 0x180);
	volatile unsigned int *output_len = (volatile unsigned int *)(MCU_TCSM_INDATA + 0x200);

	for(iLoop = 0; iLoop < rsakeylen/4; iLoop++)
		n[iLoop] = rsakey[iLoop];
	for(iLoop = 0; iLoop < rsakeylen/4; iLoop++)
		key[iLoop] = rsakey[iLoop+rsakeylen/4];
	for(iLoop = 0; iLoop < rsakeylen/4; iLoop++)
		input[iLoop] = data[iLoop];

	args->arg[0] = inputlen/4;
	args->arg[1] = rsakeylen/4;
	args->arg[2] = rsakeylen/4;
	args->arg[3] = MCU_TCSM_PADDR(output_len);
	args->arg[4] = MCU_TCSM_PADDR(input);
	args->arg[5] = MCU_TCSM_PADDR(key);
	args->arg[6] = MCU_TCSM_PADDR(n);
	args->arg[7] = MCU_TCSM_PADDR(output);
	ret = secall(args,SC_FUNC_RSA,0,1);

	for(iLoop = 0; iLoop < output_len[0]; iLoop++)
		odata[iLoop] = output[iLoop];

	return 0;

}

