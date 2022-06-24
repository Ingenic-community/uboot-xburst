#include <asm-generic/errno.h>
#include "../extend_policy_manager.h"
#include <efuse.h>

__attribute__((weak))
int efuse_read_id(void *buf, int length, int id)
{
	return 0;
}

enum EFUSE_OPS{
	WRITE_USER_ID,
	READ_CHIP_ID,
	READ_USER_ID,
	READ_RN
};

struct efuse_info {
	int gpio;
	int offset;
	int ops;  /*ops = 1 write*/
	int length;
	long long data;
};

struct efuse_priv {
	char *transfer_buf;
	int trans_len;
};


int expy_efuse_write(void *buf, int length, void *data)
{
	printf("%s\n", __func__);
	struct efuse_info *info = (struct efuse_info *)buf;
	struct efuse_priv *priv = (struct efuse_priv *)data;

	int r = 0, len = 0, id = 0, flag = 0;
	len = info->length;
	priv->transfer_buf = malloc(sizeof(char)*len);
	memset(priv->transfer_buf, 0, len);

	switch(info->ops) {
		case WRITE_USER_ID:
			break;
		case READ_CHIP_ID:
			id = EFUSE_R_CHIP_ID;
			len = 16;
			flag = 1;
			break;
		case READ_USER_ID:
			id = EFUSE_R_USER_ID;
			len = 30;
			flag = 1;
			break;
		case READ_RN:
			id = EFUSE_R_RN;
			len = 16;
			flag = 1;
			break;
		default:;
			break;
	}
	if ((r = efuse_init(info->gpio)) < 0) {
		printf("efuse init error\n");
		return r;
	}
	if (flag) {
		if ((r = efuse_read_id(priv->transfer_buf, len, id)) < 0) {
			printf("efuse read error\n");
			return r;
		}
	} else {
		printf("gpio = %d, data = %llx, length = %d, offset = %x, ops = %d\n",
				info->gpio,info->data,info->length, info->offset,info->ops);
		sprintf(priv->transfer_buf,"%llx",info->data);
		if (!!(r = efuse_write(priv->transfer_buf, len, info->offset))) {
			printf("expy_efuse write error\n");
			return r;
		}
	}
	priv->trans_len = r * 4;
	return 0;
}

int expy_efuse_read(void *buf, int length, void *data)
{
	printf("%s\n", __func__);
	struct efuse_priv *priv = (struct efuse_priv *)data;
	memcpy(buf, (char *)priv->transfer_buf, priv->trans_len);
	return length;
}

int expy_efuse_init(void)
{
	struct extend_policy *expy = malloc(sizeof(struct extend_policy));
	if (!expy)
		return -ENOMEM;
	expy->magic = MAGIC_EFUSE;
	expy->write = expy_efuse_write;
	expy->read = expy_efuse_read;
	expy->data = (struct efuse_priv *)malloc(sizeof(struct efuse_priv));
	printf("expy efuse register\n");
	return epmg_register(expy);
}

CLONER_SUB_MOUDLE_INIT(expy_efuse_init);
