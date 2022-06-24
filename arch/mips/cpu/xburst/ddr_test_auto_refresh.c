#include <common.h>
#define DDR_CS0
//#define DDR_CS1
void ddr_test_refresh(unsigned int start_addr, unsigned int end_addr)
{

#ifdef DDR_CS0
	unsigned int start_addr_cs0 = start_addr;
	unsigned int end_addr_cs0 = end_addr;
#endif
#ifdef DDR_CS1
	unsigned int start_addr_cs1 = 0xa8000000;
	unsigned int end_addr_cs1 =   0xaa000000;
#endif
	unsigned int count = 0, num = 0;
	unsigned int i = 0;
	//int flag = 1;
	int run = 0;
	int cxtan = 64;
#ifdef DDR_CS0
	count = 0;
	num = 0;
	for (i = start_addr_cs0; i < end_addr_cs0; i += cxtan){
		*(volatile unsigned int *)i = i;
		count++;
		if (count == 30000){
			//mdelay(5);
			count = 0;
			num++;
			if (num % 30 == 0)
				printf("this program cs0 is alive, pd num is %d\n", num);
		}
	}
#endif
#ifdef DDR_CS1
	count = 0;
	num = 0;
	for (i = start_addr_cs1; i <= end_addr_cs1; i += cxtan){
		*(volatile unsigned int *)i = i;
		count++;
		if (count == 15000){
			//mdelay(5);
			count = 0;
			num++;
			if (num % 30 == 0)
				printf("this program cs1 is alive, pd num is %d\n", num);
		}
	}
#endif
	//while(flag)
	{
		run++;
		printf("now run is %d\n", run);
#ifdef DDR_CS0
		count = 0;
		num = 0;
		for (i = start_addr_cs0; i < end_addr_cs0; i += cxtan){
			if (*(volatile unsigned int *)i != i){
				printf("ddr cs0 error address is %x, error data is %x, right data is %x\n",i, *(volatile unsigned int *)i, i);
			}
			count++;
			if (count == 30000){
				mdelay(5);
				count = 0;
				num++;
				if (num % 30 == 0)
					printf("this cs0 program is alive ps num is %d\n", num);
			}
		}
#endif
#ifdef DDR_CS1
		count = 0;
		num = 0;
		for (i = start_addr_cs1; i <= end_addr_cs1; i += cxtan){
			if (*(volatile unsigned int *)i != i){
				printf("ddr cs1 error address is %x, error data is %x, right data is %x\n", i, *(volatile unsigned int *)i, i);
			}
			count++;
			if (count == 15000){
				//mdelay(5);
				count = 0;
				num++;
				if (num % 30 == 0)
					printf("this cs1 program is alive ps num is %d\n", num);
			}
		}
#endif
	}
}
