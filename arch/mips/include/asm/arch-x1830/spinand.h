#ifndef __SPINAND_H
#define __SPINAND_H
#include <asm/arch/sfc.h>
#include <asm/arch/spinand_cmd.h>
#include <linux/types.h>
#include <linker_lists.h>

struct jz_sfcnand_partition {
	char name[32];         /* identifier string */
	uint32_t size;          /* partition size */
	uint32_t offset;        /* offset within the master MTD space */
	uint32_t mask_flags;       /* master MTD flags to mask out for this partition */
	uint32_t manager_mode;     /* manager_mode mtd or ubi */
};

struct jz_sfcnand_burner_param {
	uint32_t magic_num;
	int32_t partition_num;
	struct jz_sfcnand_partition *partition;
};

struct jz_sfcnand_partition_param {
	uint8_t num_partition;
	struct jz_sfcnand_partition *partition;
};

#ifndef CONFIG_SPL_BUILD
/********************************************/

#define DEVICE_ID_STRUCT(id, name_string, parameter) {      \
                .id_device = id,          \
                .name = name_string,                      \
		.param = parameter,			    \
}
/********************************************/

struct jz_sfcnand_base_param {
	uint32_t pagesize;
	uint32_t blocksize;
	uint32_t oobsize;
	uint32_t flashsize;

	uint16_t tHOLD;
	uint16_t tSETUP;
	uint16_t tSHSL_R;
	uint16_t tSHSL_W;

	uint16_t tRD;
	uint16_t tPP;
	uint16_t tBE;

	uint8_t ecc_max;
	uint8_t need_quad;
};

struct device_id_struct {
	uint8_t id_device;
	char *name;
	struct jz_sfcnand_base_param *param;
};

/*this informtion is used by nand devices*/
struct flash_operation_message {
	struct sfc_flash *flash;
	uint32_t pageaddr;
	uint32_t columnaddr;
	u_char *buffer;
	size_t len;
};
struct jz_sfcnand_read {
	void (*pageread_to_cache)(struct sfc_transfer *, struct flash_operation_message *);
	int32_t (*get_feature)(struct flash_operation_message *);
	void (*single_read)(struct sfc_transfer *, struct flash_operation_message *);
	void (*quad_read)(struct sfc_transfer *, struct flash_operation_message *);
};

struct jz_sfcnand_write {
	void (*write_enable)(struct sfc_transfer *, struct flash_operation_message *);
	void (*single_load)(struct sfc_transfer *, struct flash_operation_message *);
	void (*quad_load)(struct sfc_transfer *, struct flash_operation_message *);
	void (*program_exec)(struct sfc_transfer *, struct flash_operation_message *);
	int32_t (*get_feature)(struct flash_operation_message *);
};

struct jz_sfcnand_erase {
	void (*write_enable)(struct sfc_transfer *, struct flash_operation_message *);
	void (*block_erase)(struct sfc_transfer *, struct flash_operation_message *);
	int32_t (*get_feature)(struct flash_operation_message *);
};

struct jz_sfcnand_ops {
	struct jz_sfcnand_read nand_read_ops;
	struct jz_sfcnand_write nand_write_ops;
	struct jz_sfcnand_erase nand_erase_ops;
};

struct jz_sfcnand_device {
	uint8_t id_manufactory;
	struct device_id_struct *id_device_list;
	uint8_t id_device_count;

	struct jz_sfcnand_ops ops;
	struct list_head list;
};

struct jz_sfcnand_flashinfo {
	uint8_t id_manufactory;
	uint8_t id_device;

	struct jz_sfcnand_base_param param;
	struct jz_sfcnand_partition_param partition;
	struct jz_sfcnand_ops ops;
};

#define X_ENV_LENGTH        1024
#define X_COMMAND_LENGTH    128


#define MTD_MODE        0x0
#define UBI_MANAGER     0x1

int jz_sfcnand_register(struct jz_sfcnand_device *flash);
typedef int32_t (*spinand_regcall_t)(void);

#define ingenic_entry_declare(_type, _name, _list)				\
	static _type ingenic_##_list##_2_##_name __aligned(4)			\
			__attribute__((used,					\
			section(".u_boot_list_2_"#_list"_2_"#_name)))

#define SPINAND_MOUDLE_INIT(fn)      \
	ingenic_entry_declare(spinand_regcall_t, _1##fn, flash) = fn

#endif

#ifdef CONFIG_SPL_BUILD
struct spl_nand_param {
		unsigned int pagesize:16;
		unsigned int id_manufactory:8;
		unsigned int device_id:8;

		unsigned int addrlen:2;
		unsigned int ecc_bit:3;
		unsigned int bit_counts:3;

		unsigned char eccstat_count;
		unsigned char eccerrstatus[2];
} __attribute__((aligned(4)));
#endif

#endif
