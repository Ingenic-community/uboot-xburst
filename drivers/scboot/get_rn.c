#include <common.h>
#include <asm/errno.h>
#include <asm/io.h>
#include "secall.h"
#include "pdma.h"


int cpu_get_rn(void)
{
	unsigned int ret, i;
	volatile struct sc_args *args;
	args = (volatile struct sc_args *)GET_SC_ARGS();
	volatile unsigned int *output = (volatile unsigned int *)(MCU_TCSM_OUTDATA);

//	printf("xxxxxxxxxxx func : %s\n",__func__);

	for (i = 0; i < 31; i++)
		output[i] = 0;

	*(volatile unsigned int *)0xb0000024 |= 1<<2; //rtc use osc32k

	args->arg[0] = 31; /* random cnt */
	args->arg[1] = MCU_TCSM_PADDR(output);

	ret = secall(args, SC_FUNC_RNG, 0, 1);

	if (*(volatile unsigned int *)(MCU_TCSM_RETVAL) != SC_ERR_SUCC){
		printf("get random fail, ret val %x\n",*(volatile unsigned int *)MCU_TCSM_RETVAL);
		return -ESEC;
	}

//	for (i = 0; i < 31; i++)
//		printf("output rn: %x\n", output[i]);

	return 0;
}


