#include <errno.h>
#include <malloc.h>
#include <linux/mtd/partitions.h>
#include <asm/arch/spinand.h>
#include "../jz_sfc_common.h"
#include "nand_common.h"

#define XTX_MID0B_DEVICES_NUM         1
#define TSETUP		5
#define THOLD		5
#define	TSHSL_R		20
#define	TSHSL_W		20

#define TRD		260
#define TPP		350
#define TBE		3

static struct jz_sfcnand_base_param xtx_mid0b_param[XTX_MID0B_DEVICES_NUM] = {

	[0] = {
		/*XT26G02B */
		.pagesize = 2 * 1024,
		.blocksize = 2 * 1024 * 64,
		.oobsize = 64,
		.flashsize = 2 * 1024 * 64 * 2048,

		.tSETUP  = TSETUP,
		.tHOLD   = THOLD,
		.tSHSL_R = TSHSL_R,
		.tSHSL_W = TSHSL_W,

		.tRD = TRD,
		.tPP = TPP,
		.tBE = TBE,

		.ecc_max = 0x4,
		.need_quad = 1,
	}

};

static struct device_id_struct device_id[XTX_MID0B_DEVICES_NUM] = {
	DEVICE_ID_STRUCT(0xF2, "XT26G02B ", &xtx_mid0b_param[0]),
};

static int32_t xtx_mid0b_get_read_feature(struct flash_operation_message *op_info) {

	struct sfc_flash *flash = op_info->flash;
	struct jz_sfcnand_flashinfo *nand_info = flash->flash_info;
	struct sfc_transfer transfer;
	uint8_t device_id = nand_info->id_device;
	uint8_t ecc_status = 0;
	int32_t ret = 0;

retry:
	ecc_status = 0;
	memset(&transfer, 0, sizeof(transfer));
	sfc_list_init(&transfer);

	transfer.cmd_info.cmd = SPINAND_CMD_GET_FEATURE;
	transfer.sfc_mode = TM_STD_SPI;

	transfer.addr = SPINAND_ADDR_STATUS;
	transfer.addr_len = 1;

	transfer.cmd_info.dataen = ENABLE;
	transfer.data = &ecc_status;
	transfer.len = 1;
	transfer.direction = GLB_TRAN_DIR_READ;

	transfer.data_dummy_bits = 0;
	transfer.ops_mode = CPU_OPS;

	if(sfc_sync(flash->sfc, &transfer)) {
	        printf("sfc_sync error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		return -EIO;
	}

	if(ecc_status & SPINAND_IS_BUSY)
		goto retry;

	switch(device_id) {
		case 0xF2:
			switch((ecc_status >> 4) & 0x3) {
			    case 0x02:
				    ret = -EBADMSG;
				    break;
			    case 0x03:
				    ret = 0x8;
				    break;
			    default:
				    ret = 0;
			}
			break;
		default:
			printf("device_id err, it maybe don`t support this device, check your device id: device_id = 0x%02x\n", device_id);
			ret = -EIO;

	}
	return ret;
}

static int xtx_mid0b_nand_init(void) {
	struct jz_sfcnand_device *xtx_nand;
	xtx_nand = kzalloc(sizeof(*xtx_nand), GFP_KERNEL);
	if(!xtx_nand) {
		pr_err("alloc xtx_nand struct fail\n");
		return -ENOMEM;
	}

	xtx_nand->id_manufactory = 0x0B;
	xtx_nand->id_device_list = device_id;
	xtx_nand->id_device_count = XTX_MID0B_DEVICES_NUM;

	xtx_nand->ops.nand_read_ops.get_feature = xtx_mid0b_get_read_feature;
	return jz_sfcnand_register(xtx_nand);
}
SPINAND_MOUDLE_INIT(xtx_mid0b_nand_init);
