#include <errno.h>
#include <malloc.h>
#include <linux/mtd/partitions.h>
#include <asm/arch/spinand.h>
#include "../jz_sfc_common.h"
#include "nand_common.h"

#define ATO_DEVICES_NUM         1
#define TSETUP		5
#define THOLD		5
#define	TSHSL_R		30
#define	TSHSL_W		30

#define TRD		25
#define TPP		500
#define TBE		3

static struct jz_sfcnand_base_param ato25d1ga_param = {

	.pagesize = 2 * 1024,
	.oobsize = 64,
	.blocksize = 2 * 1024 * 64,
	.flashsize = 2 * 1024 * 64 * 1024,

	.tSETUP = TSETUP,
	.tHOLD  = THOLD,
	.tSHSL_R = TSHSL_R,
	.tSHSL_W = TSHSL_W,

	.tRD = TRD,
	.tPP = TPP,
	.tBE = TBE,

	.ecc_max = 0,//0x3,

	.need_quad = 1,

};

static struct device_id_struct device_id[ATO_DEVICES_NUM] = {
	DEVICE_ID_STRUCT(0x12, "ATO25D1GA", &ato25d1ga_param),
};

static int32_t ato_get_read_feature(struct flash_operation_message *op_info) {

	struct sfc_flash *flash = op_info->flash;
	struct jz_sfcnand_flashinfo *nand_info = flash->flash_info;
	struct sfc_transfer transfer;
	uint8_t device_id = nand_info->id_device;
	uint32_t ecc_status;

retry:
	ecc_status = 0;
	memset(&transfer, 0, sizeof(transfer));
	sfc_list_init(&transfer);

	transfer.cmd_info.cmd = SPINAND_CMD_GET_FEATURE;
	transfer.sfc_mode = TM_STD_SPI;

	transfer.addr = SPINAND_ADDR_STATUS;
	transfer.addr_len = 1;

	transfer.cmd_info.dataen = ENABLE;
	transfer.data = (uint8_t *)&ecc_status;
	transfer.len = 1;
	transfer.direction = GLB_TRAN_DIR_READ;

	transfer.data_dummy_bits = 0;
	transfer.ops_mode = CPU_OPS;

	/*wait poll time*/
	if(sfc_sync(flash->sfc, &transfer)) {
	        printf("sfc_sync error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		return -EIO;
	}

	if(ecc_status & SPINAND_IS_BUSY)
		goto retry;

	switch(device_id) {
		case 0x12:
			return 0;
		default:
			pr_err("device_id err,it maybe don`t support this device, please check your device id: device_id = 0x%02x\n", device_id);
			return -EIO;   //notice!!!
	}
}

static int ato_nand_init(void) {
	struct jz_sfcnand_device *ato_nand;
	ato_nand = kzalloc(sizeof(*ato_nand), GFP_KERNEL);
	if(!ato_nand) {
		pr_err("alloc ato_nand struct fail\n");
		return -ENOMEM;
	}

	ato_nand->id_manufactory = 0x9B;
	ato_nand->id_device_list = device_id;
	ato_nand->id_device_count = ATO_DEVICES_NUM;

	ato_nand->ops.nand_read_ops.get_feature = ato_get_read_feature;
	return jz_sfcnand_register(ato_nand);
}
SPINAND_MOUDLE_INIT(ato_nand_init);

