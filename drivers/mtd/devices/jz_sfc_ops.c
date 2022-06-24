/*
 * jz_sfc_ops.c is for sfc nor flash to set special function,
 * such as set_quad_mode, set_4byte_mode, and so on.
 *
 */

#include <linux/types.h>
#include <errno.h>
#include <asm/arch/spinor.h>
#include "./jz_sfc_common.h"


int32_t get_status(struct sfc_flash *flash, uint8_t command, uint32_t len)
{
	struct sfc_transfer transfer;
	int ret;
	unsigned char buf[32];
	int i = 0;
	int val = 0;

	memset(buf, 0, sizeof(buf));
	memset(&transfer, 0, sizeof(transfer));
	sfc_list_init(&transfer);

	transfer.sfc_mode = TM_STD_SPI;
	transfer.cmd_info.cmd = command;

	transfer.addr = 0;
	transfer.addr_len = 0;

	transfer.cmd_info.dataen = ENABLE;
	transfer.len = len;
	transfer.data = buf;
	transfer.cur_len = 0;
	transfer.direction = GLB_TRAN_DIR_READ;

	transfer.ops_mode = CPU_OPS;
	transfer.data_dummy_bits = 0;

	ret = sfc_sync(flash->sfc, &transfer);
	if(ret) {
		printf("sfc_sync error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		ret = -EIO;
		return ret;
	}

	for(i = 0; i < len; i++) {
		val |= buf[i] << (i * 8);
	}
	return val;

}

/* do nothing to set quad mode, use cmd directly */
static int32_t set_quad_mode_cmd(struct sfc_flash *flash)
{
	struct spinor_flashinfo *nor_info = flash->flash_info;
	nor_info->quad_succeed = 1;
	return 0;
}

/* write nor flash status register QE bit to set quad mode */
static int32_t set_quad_mode_reg(struct sfc_flash *flash)
{
	struct spinor_flashinfo *nor_info = flash->flash_info;
	struct spi_nor_info *spi_nor_info = nor_info->nor_flash_info;
	struct spi_nor_cmd_info *wr_en = &spi_nor_info->wr_en;
	struct spi_nor_st_info *quad_set = &spi_nor_info->quad_set;
	struct spi_nor_st_info *quad_get = &spi_nor_info->quad_get;
	struct spi_nor_st_info *busy = &spi_nor_info->busy;
	unsigned int data = (quad_set->val & quad_set->mask) << quad_set->bit_shift;
	struct sfc_transfer transfer[3];
	uint32_t sta_reg = 0;
	int ret;

	memset(transfer, 0, sizeof(transfer));
	sfc_list_init(transfer);

	/* write enable */
	transfer[0].sfc_mode = wr_en->transfer_mode;
	transfer[0].cmd_info.cmd = wr_en->cmd;

	transfer[0].addr_len = wr_en->addr_nbyte;

	transfer[0].cmd_info.dataen = DISABLE;
	transfer[0].data_dummy_bits = wr_en->dummy_byte;

	/* write ops */
	transfer[1].sfc_mode = TM_STD_SPI;
	transfer[1].cmd_info.cmd = quad_set->cmd;

	transfer[1].cmd_info.dataen = ENABLE;
	transfer[1].len = quad_set->len;
	transfer[1].data = (uint8_t *)&data;
	transfer[1].direction = GLB_TRAN_DIR_WRITE;

	transfer[1].data_dummy_bits = quad_set->dummy;
	transfer[1].ops_mode = CPU_OPS;
	sfc_list_add_tail(&transfer[1], transfer);

	ret = sfc_sync(flash->sfc, transfer);
	if(ret) {
		printf("sfc_sync error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		ret=-EIO;
	}

	do {
		sta_reg = get_status(flash, busy->cmd, busy->len);
		sta_reg = (sta_reg >> busy->bit_shift) & busy->mask;
	} while (sta_reg != busy->val);

	do {
		sta_reg = get_status(flash, quad_get->cmd, quad_get->len);
		sta_reg = (sta_reg >> quad_get->bit_shift) & quad_get->mask;
	} while (sta_reg != quad_get->val);

	nor_info->quad_succeed = 1;

	return ret;

}

static int write_enable(struct sfc_flash *flash)
{
	struct spinor_flashinfo *nor_info = flash->flash_info;
	struct spi_nor_info *spi_nor_info = nor_info->nor_flash_info;
	struct spi_nor_cmd_info *wr_en = &spi_nor_info->wr_en;
	struct sfc_transfer transfer;
	int ret;

	memset(&transfer, 0, sizeof(transfer));
	sfc_list_init(&transfer);

	transfer.sfc_mode = wr_en->transfer_mode;
	transfer.cmd_info.cmd = wr_en->cmd;

	transfer.addr_len = wr_en->addr_nbyte;

	transfer.cmd_info.dataen = DISABLE;

	transfer.data_dummy_bits = wr_en->dummy_byte;

	ret = sfc_sync(flash->sfc, &transfer);
	if(ret) {
		printf("sfc_sync error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		ret=-EIO;
	}
	return ret;
}

static int enter_4byte(struct sfc_flash *flash)
{
	struct spinor_flashinfo *nor_info = flash->flash_info;
	struct spi_nor_info *spi_nor_info = nor_info->nor_flash_info;
	struct spi_nor_cmd_info *en4byte = &spi_nor_info->en4byte;
	struct sfc_transfer transfer;
	int ret;

	memset(&transfer, 0, sizeof(transfer));
	sfc_list_init(&transfer);

	transfer.sfc_mode = en4byte->transfer_mode;
	transfer.cmd_info.cmd = en4byte->cmd;

	transfer.addr_len = en4byte->addr_nbyte;

	transfer.cmd_info.dataen = DISABLE;

	transfer.data_dummy_bits = en4byte->dummy_byte;

	ret = sfc_sync(flash->sfc, &transfer);
	if(ret) {
		printf("sfc_sync error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		ret=-EIO;
	}
	return ret;
}

/* send 4byte command to enter 4byte mode */
static int set_4byte_mode_normal(struct sfc_flash *flash)
{
	int ret;
	ret = enter_4byte(flash);
	if (ret) {
		printf( "enter 4byte mode failed\n");
	}
	return ret;
}

/**
 * 1.send write enable command
 * 2.send 4byte command to enter 4byte mode
 **/
static int set_4byte_mode_wren(struct sfc_flash *flash)
{
	int ret;
	ret = write_enable(flash);
	if (ret) {
		printf( "enter 4byte mode failed\n");
		return ret;
	}

	ret = enter_4byte(flash);
	if (ret) {
		printf( "enter 4byte mode failed\n");
	}
	return ret;
}


struct spi_nor_flash_ops nor_flash_ops;

static int noop(struct sfc_flash *flash)
{
	return 0;
}

int32_t sfc_nor_get_special_ops(struct sfc_flash *flash)
{
	struct spinor_flashinfo *nor_info = flash->flash_info;
	struct spi_nor_info *spi_nor_info = nor_info->nor_flash_info;

	switch (spi_nor_info->quad_ops_mode) {
	case 0:
		nor_flash_ops.set_quad_mode = set_quad_mode_cmd;
		break;
	case 1:
		nor_flash_ops.set_quad_mode = set_quad_mode_reg;
		break;
	default:
		nor_flash_ops.set_quad_mode = noop;
		break;
	}

	switch (spi_nor_info->addr_ops_mode) {
	case 0:
		nor_flash_ops.set_4byte_mode = set_4byte_mode_normal;
		break;
	case 1:
		nor_flash_ops.set_4byte_mode = set_4byte_mode_wren;
		break;
	default:
		nor_flash_ops.set_4byte_mode = noop;
		break;
	}

	nor_info->nor_flash_ops = &nor_flash_ops;

	return 0;
}

