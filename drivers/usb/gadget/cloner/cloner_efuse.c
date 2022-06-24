
#include <efuse.h>
#ifdef CONFIG_CMD_EFUSE
__attribute__((weak))
int efuse_read_id(void *buf, int length, int id)
{
	return 0;
}

int efuse_program(struct cloner *cloner)
{
	static int enabled = 0;
	u32 partition, length;
	void *addr;
	int r = 0;
	int id = 0, flag = 0;
	int i = 0;
	if(!enabled) {
		r = efuse_init(debug_args->efuse_gpio);
		if(r < 0) {
			printf("efuse init error\n");
			return r;
		}
		enabled = 1;
	}

	switch(cloner->cmd_type) {
	case VR_GET_CHIP_ID:
		id = EFUSE_R_CHIP_ID;
		flag = 1;
		break;
	case VR_GET_USER_ID:
		id = EFUSE_R_USER_ID;
		flag = 1;
		break;

	default:	/* write request */
		partition = cloner->cmd->write.partition;
		length = cloner->cmd->write.length;
		addr = (void *)cloner->write_req->buf;

		if (!!(r = efuse_write(addr, length, partition))) {
			printf("efuse write error\n");
			return r;
		}
		break;
	}

	if(flag) {
		addr = (void *)cloner->ep0req->buf;
		length = cloner->ep0req->length;
		if ((r = efuse_read_id(addr, length, id)) < 0) {
			printf("efuse read chip id error\n");
			return r;
		}
	}

	return r;
}
#endif
