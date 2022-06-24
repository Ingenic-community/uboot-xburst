#ifndef __SPL_OTA__
#define __SPL_OTA__

struct ota_ops {
	void (*flash_init)();
	int (*flash_read)(unsigned int addr, unsigned int len, unsigned int buf);
	struct jz_nand_partition_param (*flash_get_partitions)(void);
};

char* spl_ota_load_image(void);
void register_ota_ops(struct ota_ops *ops);

#endif	/* __SPL_OTA__ */
