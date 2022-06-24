#include "jz_pdma.h"

int cmp_data(unsigned long *src,unsigned long *dst,unsigned long len)
{
	unsigned long *start = src;
	unsigned long *end_src = src + len;
	while(start < end_src)
	{
		if(*start++ != *dst++) {
			printf("cmp data error: src:%08x, dst:%08x\n", *(start-1), *(dst-1));
			return (start - src);
		}
	}
	return 0;
}


int tcsmbank0_data_check(void)
{

	int i;
	unsigned int *d = (unsigned char *)(TCSM_BANK1);
	for(i = 0; i<64; i++) {
		printf("checkout data d:%08x, [%d]:%08x\n",&d[i], i, d[i]);
	}
	printf("\n\n");
}
