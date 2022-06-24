#include <asm-generic/errno.h>
#include "../extend_policy_manager.h"

#define msleep(a)	udelay(a * 1000)
static transfer_buf[1024];
int expy_test_write(void *buf, int length, void *data)
{
	char buf_x[100];
	int len = length;
	printf("%s\n", __func__);
	memset(transfer_buf,0,1024);
	memcpy(transfer_buf, buf, 1024);
	return 0;
}

int expy_test_read(void *buf, int length, void *data)
{
	printf("%s\n", __func__);
	memcpy(buf, transfer_buf, length);
	return length;
}


int expy_test_init(void)
{
	struct extend_policy *expy = malloc(sizeof(struct extend_policy));
	if (!expy)
		return -ENOMEM;
	expy->magic = 0x30545845;
	expy->write = expy_test_write;
	expy->read = expy_test_read;
	printf("expy test register\n");
	return epmg_register(expy);
}

CLONER_SUB_MOUDLE_INIT(expy_test_init);
