#include <common.h>
#include <asm/io.h>
#include <asm/errno.h>

#include <asm/gpio.h>

#include "secall.h"
#include "pdma.h"

//#include "test1.h"
#include "test_nku.h"
#include "otp.h"
#include <cloner/cloner.h>
static void gpio_output_value(int gpio, int value)
{
	gpio_direction_output(gpio, value);

	//*(volatile unsigned int*)0xb0010018 = 1 << 1;
	//*(volatile unsigned int*)0xb0010024 = 1 << 1;
	//*(volatile unsigned int*)0xb0010038 = 1 << 1;
	//if(val) {
		//*(volatile unsigned int*)0xb0010044 = 1<<1; /*SET 1*/
	//} else {
		//*(volatile unsigned int*)0xb0010048 = 1<<1; /*SET 1*/
	//}
}

int test_wtotp(int opera){
	unsigned int ret;
	volatile struct sc_args *args;
	int iLoop = 0;
	volatile unsigned int *dbg = (volatile unsigned int *)(MCU_TCSM_DBG);
	args = (volatile struct sc_args *)GET_SC_ARGS();

	args->arg[0] = opera;
	*(volatile unsigned int *)0xb3540000 |= 1<<15; /*pg en*/
	gpio_output_value(debug_args->efuse_gpio, 0);
	ret = secall(args,SC_FUNC_WTOTP,0,1);
	gpio_output_value(debug_args->efuse_gpio, 1);
	*(volatile unsigned int *)0xb3540000 &= ~(1<<15);

	if (*(volatile unsigned int *)(MCU_TCSM_RETVAL) != SC_ERR_SUCC){
		printf("secall SC_FUNC_WTOTP fail 0x%08x\n", *(volatile unsigned int *)(MCU_TCSM_RETVAL));
		return -ESEC;
	} else {
		printf("###########secall SC_FUNC_WTOTP success 0x%08x\n", *(volatile unsigned int *)(MCU_TCSM_RETVAL));
	}
	return 0;
}

int test_burnnku(void)
{
	cpu_burn_nku(nku1,sizeof(nku1));
}


int test_load_nku(void)
{
	cpu_load_nku(nku1, sizeof(nku1));
}


static unsigned int enckey[62];
int test_get_enckey(void)
{
	cpu_get_enckey(enckey);
	return 0;
}


int test_burnukey(void)
{
	unsigned int ret;
	volatile struct sc_args *args;
	volatile unsigned int *ukey = (volatile unsigned int *)MCU_TCSM_PUTUKEY;

	args = (volatile struct sc_args *)GET_SC_ARGS();

	unsigned int encukey1[4] = {
		0xee6694c3,
		0xef42d55b,
		0x60bdfed8,
		0x16536470
	};

	ukey[0] = encukey1[0];
	ukey[1] = encukey1[1];
	ukey[2] = encukey1[2];
	ukey[3] = encukey1[3];
	printf("ukey 0x%08x-0x%08x-0x%08x-0x%08x\n",ukey[0],ukey[1],ukey[2],ukey[3]);

	args->arg[0] = 0;
	args->arg[1] = MCU_TCSM_PADDR(MCU_TCSM_PUTUKEY);

	ret = secall(args,SC_FUNC_BURNUK,0,1);
	if (*(volatile unsigned int *)(MCU_TCSM_RETVAL) != SC_ERR_SUCC){
		printf("secall SC_FUNC_BURNUK fail 0x%08x\n", *(volatile unsigned int *)(MCU_TCSM_RETVAL));
	}
	test_wtotp(WT_OTP_UK);

	#if 0
	/*WRITE PROTECT bit 15.*/
	/*efuse config*/
	volatile unsigned int * reg_data0 = (volatile unsigned int *)0xb354000c;
	volatile unsigned int * reg_ctrl = (volatile unsigned int *)0xb3540000;
	volatile unsigned int * reg_stat = (volatile unsigned int *)0xb3540008;

	printf("xxxx otp efuse state:%x\n", *reg_stat);

	*reg_ctrl |= 1<<15; /*pg en*/
	gpio_output_value(debug_args->efuse_gpio, 0);

	*reg_data0 =  1<<15; /* ukey protected */
	*reg_ctrl &= ~(0x7f<<21 | 0x1f<<16); /*set address ,length*/
	*reg_ctrl |= 0x3e<<21;

	*reg_ctrl |= 1<<1; /*write en*/
	while(!(*reg_stat & (1<<1)));

	gpio_output_value(debug_args->efuse_gpio, 1);
	*reg_ctrl &= ~(1<<15);

	*reg_ctrl |= 1<<0; /*read en*/
	while(!(*reg_stat & (1<<0)));
	printf("xxxx otp efuse state:%x\n", *reg_stat);
#endif

	return 0;
 }



int test_read_chip_id()
{

	volatile unsigned int * reg_ctrl = (volatile unsigned int *)0xb3540000;
	volatile unsigned int * reg_cfg = (volatile unsigned int *)0xb3540004;
	volatile unsigned int * reg_stat = (volatile unsigned int *)0xb3540008;

	volatile unsigned int * reg_data0 = (volatile unsigned int *)0xb354000c;
	volatile unsigned int * reg_data1 = (volatile unsigned int *)0xb3540010;
	volatile unsigned int * reg_data2 = (volatile unsigned int *)0xb3540014;
	volatile unsigned int * reg_data3 = (volatile unsigned int *)0xb3540018;

	*reg_ctrl = (0 << 21) | (15<<16) | (1<<0);
//	while(!(*reg_stat & (1 << 0)));


	printf("chip-id: %x-%x-%x-%x\n", *reg_data0, *reg_data1, *reg_data2, *reg_data3);


}

int test_aes_by_ckey()
{

	int data[4] = {
		0xee6694c3,
		0xef42d55b,
		0x60bdfed8,
		0x16536470,

	};
	int endata[4] = {

	};
	int tmpdata[4] = {

	};
	aes(data,endata,4*4,1,0);
	aes(endata,tmpdata,4*4,1,1);
	printf("--data--%x -%x -%x -%x---\n",data[0],data[1],data[2],data[3]);
	printf("--endata--%x -%x -%x -%x---\n",endata[0],endata[1],endata[2],endata[3]);
	printf("--tmpdata--%x -%x -%x -%x---\n",tmpdata[0],tmpdata[1],tmpdata[2],tmpdata[3]);
	//cmp_data(data,tmpdata,4);
}

int test_rsa()
{

	int input[31]= {
		0xe55f68e6,
		0x9fb656ce,
		0x31ccc3ed,
		0x1f649afd,
		0xb9388938,
		0xc6ebc6b1,
		0x951a5b57,
		0x568f5a98,
		0x79df07e8,
		0x8a03f1ff,
		0x09dfadc3,
		0xbfc0341f,
		0x7116f224,
		0x465ab592,
		0xdea47f66,
		0x9cf48e88,
		0xa422b843,
		0x2a1163a0,
		0x7f7ab108,
		0xf30d3330,
		0x92504f59,
		0x145f06a8,
		0xdb291354,
		0xf522eb85,
		0x70f877f1,
		0x4f14c40a,
		0x9ffe5a04,
		0xb46bc1cb,
		0x108e2c34,
		0x50d0676c,
		0x0092b2e9,
	};
	int cmpdata[4] = {
		0xf1085e5e,
		0x5eb7f071,
		0x4f176806,
		0xa6c5ec2e,
	};
	int output[4] = {

	};
	do_rsa(input,31*4,output,nku1,124);
	printf("--rsa--%x -%x -%x -%x---\n",output[0],output[1],output[2],output[3]);
	printf("--rsa cmp data--%x -%x -%x -%x---\n",cmpdata[0],cmpdata[1],cmpdata[2],cmpdata[3]);
	//cmp_data(output,data,4);
}


int cpu_burn_secboot_enable_1(void)
{

	/*efuse config*/
	volatile unsigned int * reg_ctrl = (volatile unsigned int *)0xb3540000;
	volatile unsigned int * reg_stat = (volatile unsigned int *)0xb3540008;
	volatile unsigned int * reg_data0 = (volatile unsigned int *)0xb354000c;

	printf("======================== sec boot enable ===================\n");

	printf("xxxx otp efuse state:%x\n", *reg_stat);
	printf("xxxx otp efuse date:%x\n", *reg_data0);

	*reg_ctrl |= 1<<15; /*pg en*/
	gpio_output_value(debug_args->efuse_gpio, 0);

	*reg_data0 = (1<<0 | 1<<4); /* security boot enable, security boot enable protected */
	*reg_ctrl &= ~(0x7f<<21 | 0x1f<<16); /*set address ,length*/
	*reg_ctrl |= 0x3e<<21;

	printf("xxxx otp efuse date:%x\n", *reg_data0);
	printf("xxxx otp ctrl: %x\n", *reg_ctrl);
	*reg_ctrl |= 1<<1; /*write en*/
	while(!(*reg_stat & (1<<1)));

	gpio_output_value(debug_args->efuse_gpio, 1);
	*reg_ctrl &= ~(1<<15);


	*reg_ctrl = (0x3e << 21) | (0x1<<16) | (1<<0);
	while(!(*reg_stat & (1 << 0)));


	printf("state updated: %x\n", *reg_stat);

}

