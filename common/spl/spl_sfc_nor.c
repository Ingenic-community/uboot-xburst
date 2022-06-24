#include <common.h>
#include <config.h>
#include <spl.h>
#include <asm/io.h>
#include <errno.h>
#include <linux/err.h>
#include <malloc.h>
#include <asm/arch/clk.h>
#include <div64.h>

#include <asm/arch/sfc.h>
#include <asm/arch/spinor.h>
#include <generated/sfc_timing_val.h>
#include <asm/nvrw_interface.h>


#define GS_RETRY_TIMES	100

static struct spinor_flashinfo *nor_info = (void *)(CONFIG_SYS_TEXT_BASE + 0x500000);

static inline void sfc_writel(unsigned short offset, u32 value)
{
	writel(value, SFC_BASE + offset);
}

static inline unsigned int sfc_readl(unsigned short offset)
{
	return readl(SFC_BASE + offset);
}

static inline void sfc_flush_and_start(void)
{
	sfc_writel(SFC_TRIG, TRIG_FLUSH);
	sfc_writel(SFC_TRIG, TRIG_START);
}

static inline void sfc_clear_all_intc(void)
{
	sfc_writel(SFC_SCR, 0x1f);
}

static inline void sfc_mask_all_intc(void)
{
	sfc_writel(SFC_INTC, 0x1f);
}

static inline void sfc_set_length(int value)
{
	sfc_writel(SFC_TRAN_LEN, value);
}

static inline unsigned int sfc_read_rxfifo(void)
{
	return sfc_readl(SFC_RM_DR);
}

static inline void sfc_write_txfifo(const unsigned int value)
{
	sfc_writel(SFC_RM_DR, value);
}

static inline unsigned int get_sfc_ctl_sr(void)
{
	return sfc_readl(SFC_SR);
}

static void cpu_read_rxfifo(struct sfc_transfer *xfer)
{
	int i;
	unsigned long align_len = 0;
	unsigned int fifo_num, size;

	align_len = ALIGN(xfer->len, 4);
	size = (align_len - xfer->cur_len) >> 2;

	fifo_num = (size < THRESHOLD) ? size : THRESHOLD;

	for (i = 0; i < fifo_num; i++) {
		*(unsigned int *)xfer->data = sfc_read_rxfifo();
		xfer->data += 4;
		xfer->cur_len += 4;
	}
}

static void cpu_write_txfifo(struct sfc_transfer *xfer)
{
	/**
	 * Assuming that all data is less than one word,
	 * if len large than one word, unsupport!
	 **/

	sfc_write_txfifo(*(unsigned int *)xfer->data);
}

static void sfc_sr_handle(struct sfc_transfer *xfer)
{
	unsigned int reg_sr = 0;
	unsigned int tmp = 0;

	while (1) {
		reg_sr = get_sfc_ctl_sr();
		if(reg_sr & CLR_END){
			tmp = CLR_END;
			break;
		}

		if (reg_sr & CLR_RREQ) {
			sfc_writel(SFC_SCR, CLR_RREQ);
			cpu_read_rxfifo(xfer);
		}

		if (reg_sr & CLR_TREQ) {
			sfc_writel(SFC_SCR, CLR_TREQ);
			cpu_write_txfifo(xfer);
		}

		if (reg_sr & (CLR_UNDER | CLR_OVER)) {
			tmp = CLR_UNDER | CLR_OVER;
			printf("OVER or UNDER!\n");
			break;
		}
	}
	if (tmp)
		sfc_writel(SFC_SCR, tmp);
}

static void sfc_start_transfer(struct sfc_transfer *xfer)
{
	sfc_clear_all_intc();
	sfc_mask_all_intc();
	sfc_flush_and_start();

	sfc_sr_handle(xfer);

}
static void sfc_phase_transfer(struct sfc_transfer *transfer)
{
	unsigned int tmp = 0;

	tmp |= (transfer->addr_len << ADDR_WIDTH_OFFSET);	//addr_len
	tmp |= TRAN_CONF_CMDEN;	//cmd_enable
	tmp |= transfer->cmd_info.cmd;	//cmd
	tmp |= transfer->data_dummy_bits << DMYBITS_OFFSET;	//dummy
	if(transfer->cmd_info.dataen == 1) {	//date_enable
		tmp |= TRAN_CONF_DATEEN;
	} else {
		tmp &= ~TRAN_CONF_DATEEN;
	}
	tmp |= (transfer->sfc_mode << TRAN_CONF_TRAN_MODE_OFFSET);	//transfer_mode 0~8
	sfc_writel(SFC_TRAN_CONF0, tmp);
	sfc_writel(SFC_DEV_ADDR0, transfer->addr);	//addr
	sfc_writel(SFC_DEV_ADDR_PLUS0, transfer->addr_plus);	//addr_plus
}

static void sfc_glb_info_config(struct sfc_transfer *transfer)
{

	unsigned int tmp;
	tmp = sfc_readl(SFC_GLB);
	if(transfer->direction == GLB_TRAN_DIR_READ) {	//read or write direction
		tmp &= ~GLB_TRAN_DIR;
	} else {
		tmp |= GLB_TRAN_DIR;
	}
	tmp &= ~(GLB_OP_MODE); //use CPU mode
	tmp &= ~GLB_PHASE_NUM_MSK;	//phase_num=1
	tmp |= 1 << GLB_PHASE_NUM_OFFSET;
	sfc_writel(SFC_GLB, tmp);

	sfc_set_length(transfer->len);
}

static void sfc_sync(struct sfc_transfer *xfer)
{
	sfc_phase_transfer(xfer);
	sfc_glb_info_config(xfer);
	sfc_start_transfer(xfer);
}

static int get_norflash_status(int command, int len)
{
	struct sfc_transfer transfer;
	int val = 0;

	memset(&transfer, 0, sizeof(transfer));

	transfer.sfc_mode = TM_STD_SPI;
	transfer.cmd_info.cmd = command;

	transfer.cmd_info.dataen = ENABLE;
	transfer.len = len;
	transfer.data = (const unsigned char *)&val;
	transfer.direction = GLB_TRAN_DIR_READ;

	transfer.ops_mode = CPU_OPS;

	sfc_sync(&transfer);

	return val;

}

static void write_enable(struct mini_spi_nor_info *spi_nor_info)
{
	struct spi_nor_cmd_info *wr_en = &spi_nor_info->wr_en;
	struct sfc_transfer transfer;

	memset(&transfer, 0, sizeof(transfer));

	transfer.sfc_mode = wr_en->transfer_mode;
	transfer.cmd_info.cmd = wr_en->cmd;

	transfer.addr_len = wr_en->addr_nbyte;

	transfer.cmd_info.dataen = DISABLE;

	transfer.data_dummy_bits = wr_en->dummy_byte;

	sfc_sync(&transfer);
}

static void enter_4byte(struct mini_spi_nor_info *spi_nor_info)
{
	struct spi_nor_cmd_info *en4byte = &spi_nor_info->en4byte;
	struct sfc_transfer transfer;

	memset(&transfer, 0, sizeof(transfer));

	transfer.sfc_mode = en4byte->transfer_mode;
	transfer.cmd_info.cmd = en4byte->cmd;

	transfer.addr_len = en4byte->addr_nbyte;

	transfer.cmd_info.dataen = DISABLE;

	transfer.data_dummy_bits = en4byte->dummy_byte;
	sfc_sync(&transfer);
}

static inline void set_quad_mode_cmd(struct mini_spi_nor_info *spi_nor_info)
{
	nor_info->cur_r_cmd = &spi_nor_info->read_quad;
}

/* write nor flash status register QE bit to set quad mode */
static void set_quad_mode_reg(struct mini_spi_nor_info *spi_nor_info)
{
	struct spi_nor_st_info *quad_set = &spi_nor_info->quad_set;
	struct spi_nor_st_info *quad_get = &spi_nor_info->quad_get;
	struct spi_nor_st_info *busy = &spi_nor_info->busy;
	unsigned int data = (quad_set->val & quad_set->mask) << quad_set->bit_shift;
	struct sfc_transfer transfer;
	unsigned int times = GS_RETRY_TIMES;
	unsigned int val;

	write_enable(spi_nor_info);

	memset(&transfer, 0, sizeof(transfer));
	/* write ops */
	transfer.sfc_mode = TM_STD_SPI;
	transfer.cmd_info.cmd = quad_set->cmd;

	transfer.cmd_info.dataen = ENABLE;
	transfer.len = quad_set->len;
	transfer.data = (unsigned char *)&data;
	transfer.direction = GLB_TRAN_DIR_WRITE;

	transfer.data_dummy_bits = quad_set->dummy;
	transfer.ops_mode = CPU_OPS;
	sfc_sync(&transfer);

	while (times--) {
		val = (get_norflash_status(quad_get->cmd, quad_get->len) >> quad_get->bit_shift) & quad_get->mask;
		if (val == quad_get->val) {
			nor_info->cur_r_cmd = &spi_nor_info->read_quad;
			break;
		}
	}

	while(!(((get_norflash_status(busy->cmd, busy->len) >> busy->bit_shift) & busy->mask) == busy->val));
}

static void sfc_nor_read_params(unsigned int addr, unsigned char *buf, unsigned int len)
{
	struct sfc_transfer transfer;

	memset(&transfer, 0, sizeof(transfer));

	transfer.sfc_mode = TM_STD_SPI;
	transfer.cmd_info.cmd = SPINOR_OP_READ;

	transfer.addr_len = DEF_ADDR_LEN;
	transfer.addr = addr;

	transfer.cmd_info.dataen = ENABLE;
	transfer.len = len;
	transfer.data = buf;
	transfer.cur_len = 0;
	transfer.direction = GLB_TRAN_DIR_READ;

	transfer.data_dummy_bits = 0;
	transfer.ops_mode = CPU_OPS;

	sfc_sync(&transfer);

}

static inline void set_flash_timing(void)
{
	sfc_writel(SFC_DEV_CONF, DEF_TIM_VAL);
}

static void reset_nor(void)
{
	struct sfc_transfer transfer;

	memset(&transfer, 0, sizeof(transfer));

	transfer.cmd_info.dataen = DISABLE;
	transfer.sfc_mode = 0;

	transfer.cmd_info.cmd = SPINOR_OP_RSTEN;
	sfc_sync(&transfer);

	transfer.cmd_info.cmd = SPINOR_OP_RST;
	sfc_sync(&transfer);
	udelay(100);
}

void sfc_init(struct mini_spi_nor_info *spi_nor_info)
{

	clk_set_rate(SFC, CONFIG_SFC_NOR_RATE);

	reset_nor();

	set_flash_timing();
	sfc_nor_read_params(CONFIG_SPIFLASH_PART_OFFSET + sizeof(struct burner_params), (void *)spi_nor_info, sizeof(struct mini_spi_nor_info));
	printf("%s %x\n", spi_nor_info->name, spi_nor_info->id);

	nor_info->cur_r_cmd = &spi_nor_info->read_standard;

#ifdef CONFIG_SFC_QUAD
	if(spi_nor_info->quad_ops_mode)
		set_quad_mode_reg(spi_nor_info);
	else
		set_quad_mode_cmd(spi_nor_info);
#endif

	if (spi_nor_info->chip_size > 0x1000000) {
		if(spi_nor_info->addr_ops_mode)
			write_enable(spi_nor_info);
		enter_4byte(spi_nor_info);
	}
}

static unsigned int sfc_do_read(unsigned int addr, unsigned char *buf, unsigned int len)
{
	unsigned char command = nor_info->cur_r_cmd->cmd;
	int dummy_byte = nor_info->cur_r_cmd->dummy_byte;
	int addr_size = nor_info->cur_r_cmd->addr_nbyte;
	int transfer_mode = nor_info->cur_r_cmd->transfer_mode;
	struct sfc_transfer transfer;

	memset(&transfer, 0, sizeof(transfer));

	transfer.sfc_mode = transfer_mode;
	transfer.cmd_info.cmd = command;

	transfer.addr_len = addr_size;
	transfer.addr = addr;

	transfer.cmd_info.dataen = ENABLE;
	transfer.len = len;
	transfer.data = buf;
	transfer.cur_len = 0;
	transfer.direction = GLB_TRAN_DIR_READ;

	transfer.ops_mode = CPU_OPS;
	transfer.data_dummy_bits = dummy_byte;
	sfc_sync(&transfer);

	return len;
}

int sfc_read_data(unsigned int from, unsigned int len, unsigned int *buf)
{
	int tmp_len = 0, current_len = 0;

	while(len) {
		tmp_len = sfc_do_read(from + current_len, &buf[current_len], len);
		current_len += tmp_len;
		len -= tmp_len;
	}

	return current_len;

}


#ifdef CONFIG_OTA_VERSION20
  #ifdef CONFIG_NV_INFO_AS_IAD
void *spl_get_nvinfo(struct mini_spi_nor_info *spi_nor_info, unsigned int nv_addr)
{
/*	struct mini_spi_nor_info *spi_nor_info = nor_info + sizeof(struct spinor_flashinfo);*/
	nvinfo_t *nvinfo = (nvinfo_t *)CONFIG_SPL_NV_BASE;
	int i = 0, j = 0;
	int erasesize = 0;

	if (!nv_addr)
		return NULL;

	erasesize = spi_nor_info->erase_size;

	for (i = 0; i < 2; i++) {
		sfc_read_data(nv_addr + i * erasesize, sizeof(nvinfo_t), nvinfo);
		if (*(int *)nvinfo->start_magic == 0x41544f && *(int *)nvinfo->end_magic == 0x41544f) {
			printf("nvinfo->update_flag=%d, nvinfo->update_process: %d.\n",
				  nvinfo->update_flag, nvinfo->update_process);
			break;
		}
	}

	if (i == 0 || i == 1) {
		debug("nv is in #%d.\n", i);
		return nvinfo;
	} else {
		return NULL;
	}
}
  #endif

static void nv_map_area(struct mini_spi_nor_info *spi_nor_info, unsigned int *base_addr, unsigned int nv_addr, unsigned int nv_size)
{
	unsigned int blocksize = spi_nor_info->erase_size;
	unsigned int nv_num = nv_size / blocksize;
	unsigned int buf[6][2];
	unsigned int tmp_buf[4];
	unsigned int nv_off = 0, nv_count = 0;
	unsigned int addr, i;

	if(nv_num > 6) {
	//	printf("%s,bigger\n",__func__);
		while(1);
	}

	for(i = 0; i < nv_num; i++) {
		addr = nv_addr + i * blocksize;
		sfc_read_data(addr, 4, buf[i]);
		if(buf[i][0] == 0x5a5a5a5a) {
			sfc_read_data(addr + 1 *1024,  16, tmp_buf);
			addr += blocksize - 8;
			sfc_read_data(addr, 8, buf[i]);
			if(buf[i][1] == 0xa5a5a5a5) {
				if(nv_count < buf[i][0]) {
					nv_count = buf[i][0];
					nv_off = i;
				}
			}
		}
	}

	*base_addr = nv_addr + nv_off * blocksize;
}
#endif

static void spl_load_kernel(long offset)
{
	struct image_header *header = (struct image_header *)(CONFIG_SYS_TEXT_BASE);

	sfc_read_data(offset, sizeof(struct image_header), (unsigned int*)header);
	spl_parse_image_header(header);
	sfc_read_data(offset, spl_image.size, (unsigned int*)(spl_image.load_addr));
}

char* spl_sfc_nor_load_image(void)
{
	struct mini_spi_nor_info *spi_nor_info = nor_info + sizeof(struct spinor_flashinfo);
	char *cmdargs = NULL;
#ifdef CONFIG_SPL_OS_BOOT
	unsigned int bootimg_addr = 0;
	unsigned int bootimg_size = 0;
	struct norflash_partitions partition;
	int i;
#ifdef CONFIG_OTA_VERSION20
	nvinfo_t *nvinfo = NULL;
	unsigned int nv_rw_addr = 0;
	unsigned int nv_rw_size = 0;
	unsigned int src_addr, updata_flag;
	unsigned nv_buf[2];
	int count = 8;
#endif
#endif
	sfc_init(spi_nor_info);
#ifdef CONFIG_SPL_OS_BOOT
	sfc_read_data(CONFIG_SPIFLASH_PART_OFFSET + sizeof(struct spi_nor_info) + sizeof(int) * 2, sizeof(struct norflash_partitions), (unsigned int*)&partition);
	for (i = 0 ; i < partition.num_partition_info; i ++) {
		if (!strncmp(partition.nor_partition[i].name, CONFIG_SPL_OS_NAME, sizeof(CONFIG_SPL_OS_NAME))) {
			bootimg_addr = partition.nor_partition[i].offset;
			bootimg_size = partition.nor_partition[i].size;
		}
  #ifdef CONFIG_OTA_VERSION20
		if (!strncmp(partition.nor_partition[i].name, CONFIG_PAR_NV_NAME, sizeof(CONFIG_PAR_NV_NAME))) {
			nv_rw_addr = partition.nor_partition[i].offset;
			nv_rw_size = partition.nor_partition[i].size;
		}
  #endif /* CONFIG_OTA_VERSION20 */
	}

  #ifdef CONFIG_BOOT_VMLINUX
	spl_image.os = IH_OS_LINUX;
	spl_image.entry_point = CONFIG_LOAD_ADDR;
	sfc_read_data(bootimg_addr, bootimg_size, (unsigned int*)CONFIG_LOAD_ADDR);
	cmdargs = CONFIG_SYS_SPL_ARGS_ADDR;
	return cmdargs;
  #endif /* CONFIG_BOOT_VMLINUX */

  #ifdef CONFIG_BOOT_RTOS
	spl_image.entry_point = CONFIG_LOAD_ADDR;
	sfc_read_data(bootimg_addr, bootimg_size, (unsigned int*)CONFIG_LOAD_ADDR);
	return NULL;
  #endif /* CONFIG_BOOT_RTOS */

  #ifndef CONFIG_OTA_VERSION20 /* norflash spl boot kernel */
	spl_load_kernel(bootimg_addr);
	cmdargs = CONFIG_SYS_SPL_ARGS_ADDR;
  #else /* define CONFIG_OTA_VERSION20 */
    #ifndef CONFIG_NV_INFO_AS_IAD
	nv_map_area(spi_nor_info, (unsigned int *)&src_addr, nv_rw_addr, nv_rw_size);
	sfc_read_data(src_addr, count, nv_buf);
	updata_flag = nv_buf[1];
	if((updata_flag & 0x3) != 0x3) {
		spl_load_kernel(bootimg_addr);
		cmdargs = CONFIG_SYS_SPL_ARGS_ADDR;
	} else {
		struct image_header *header = (struct image_header *)(CONFIG_SYS_TEXT_BASE);
		memset(header, 0, sizeof(struct image_header));
		spl_parse_image_header(header);
		sfc_read_data(CONFIG_UBOOT_OFFSET, CONFIG_SYS_MONITOR_LEN,(unsigned int*)CONFIG_SYS_TEXT_BASE);
	}
    #else /* define CONFIG_NV_INFO_AS_IAD */
	nvinfo = spl_get_nvinfo(spi_nor_info, nv_rw_addr);
	if (!nvinfo) {
		/* if magic not valid, force to nonupdate status */
		printf("Invalid nvinfo.\n");
		nvinfo->update_flag = FLAG_NONUPDATE;
	}
	if (nvinfo->update_flag == FLAG_NONUPDATE) {
		spl_load_kernel(bootimg_addr);
		cmdargs = CONFIG_SYS_SPL_ARGS_ADDR;
	} else if (nvinfo->update_flag == FLAG_UPDATE) {
		switch (nvinfo->update_process) {
			case PROCESS_2:
				debug("update, process: %d, load kernel from 0x%x\n",
						nvinfo->update_process, CONFIG_SPL_OTA_OS_OFFSET);
				spl_load_kernel(CONFIG_SPL_OTA_OS_OFFSET);
				cmdargs = CONFIG_SPL_OTA_BOOTARGS;
				break;
			case PROCESS_1:
			case PROCESS_3:
			case PROCESS_DONE:
				debug("update, process: %d, load kernel from 0x%x\n",
						nvinfo->update_process, bootimg_addr);
				spl_load_kernel(bootimg_addr);
				cmdargs = CONFIG_SYS_SPL_ARGS_ADDR;
				break;
		}
	} else {
		debug("Invalid update flag: %d.\n", nvinfo->update_flag);
	}
    #endif	/* CONFIG_NV_INFO_AS_IAD */
  #endif	/* CONFIG_OTA_VERSION20 */
#else /* not define CONFIG_SPL_OS_BOOT */
	{
		struct image_header *header = (struct image_header *)(CONFIG_SYS_TEXT_BASE);
		memset(header, 0, sizeof(struct image_header));
		spl_parse_image_header(header);
		sfc_read_data(CONFIG_UBOOT_OFFSET, CONFIG_SYS_MONITOR_LEN,(unsigned int*)CONFIG_SYS_TEXT_BASE);
	}
#endif	/* CONFIG_SPL_OS_BOOT */

	return cmdargs;
}
