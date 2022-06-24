/*
 * SFC controller for SPI protocol, use FIFO and DMA;
 *
 * Copyright (c) 2015 Ingenic
 * Author: <xiaoyang.fu@ingenic.com>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
*/

#include <asm/arch/clk.h>
#include <asm/gpio.h>
#include <asm/arch/base.h>
#include <asm/io.h>
#include <linux/list.h>
#include <linux/types.h>
#include <linux/err.h>
#include <common.h>
#include <malloc.h>
#include <asm/arch/sfc.h>

//#define   DEBUG

static inline void sfc_writel(struct sfc *sfc, uint16_t offset, uint32_t value)
{
	writel(value, SFC_BASE + offset);
}

static inline uint32_t sfc_readl(struct sfc *sfc, uint16_t offset)
{
	return readl(SFC_BASE + offset);
}

#ifdef DEBUG
void dump_sfc_reg(struct sfc *sfc)
{
	int i = 0;
	printf("SFC_GLB			:%08x\n", sfc_readl(sfc, SFC_GLB ));
	printf("SFC_DEV_CONF	:%08x\n", sfc_readl(sfc, SFC_DEV_CONF ));
	printf("SFC_DEV_STA_EXP	:%08x\n", sfc_readl(sfc, SFC_DEV_STA_EXP));
	printf("SFC_DEV_STA_RT	:%08x\n", sfc_readl(sfc, SFC_DEV_STA_RT ));
	printf("SFC_DEV_STA_MSK	:%08x\n", sfc_readl(sfc, SFC_DEV_STA_MSK ));
	printf("SFC_TRAN_LEN		:%08x\n", sfc_readl(sfc, SFC_TRAN_LEN ));

	for(i = 0; i < 6; i++)
		printf("SFC_TRAN_CONF(%d)	:%08x\n", i,sfc_readl(sfc, SFC_TRAN_CONF(i)));

	for(i = 0; i < 6; i++)
		printf("SFC_DEV_ADDR(%d)	:%08x\n", i,sfc_readl(sfc, SFC_DEV_ADDR(i)));

	printf("SFC_MEM_ADDR :%08x\n", sfc_readl(sfc, SFC_MEM_ADDR ));
	printf("SFC_TRIG	 :%08x\n", sfc_readl(sfc, SFC_TRIG));
	printf("SFC_SR		 :%08x\n", sfc_readl(sfc, SFC_SR));
	printf("SFC_SCR		 :%08x\n", sfc_readl(sfc, SFC_SCR));
	printf("SFC_INTC	 :%08x\n", sfc_readl(sfc, SFC_INTC));
	printf("SFC_FSM		 :%08x\n", sfc_readl(sfc, SFC_FSM ));
	printf("SFC_CGE		 :%08x\n", sfc_readl(sfc, SFC_CGE ));
//	printf("SFC_RM_DR 	 :%08x\n", sfc_readl(spi, SFC_RM_DR));
}

static void dump_data(uint8_t *buf, uint32_t len)
{
	uint32_t i;
	for(i = 0; i < len; i++){
		if(!(i % 16)){
			printk("\n");
			printk("%08x:",i);
		}
		printk("%02x ",buf[i]);
	}
}
#endif

static void sfc_init(struct sfc *sfc)
{
	int n;
	for(n = 0; n < N_MAX; n++) {
		sfc_writel(sfc, SFC_TRAN_CONF(n), 0);
		sfc_writel(sfc, SFC_DEV_ADDR(n), 0);
		sfc_writel(sfc, SFC_DEV_ADDR_PLUS(n), 0);
	}

	sfc_writel(sfc, SFC_DEV_CONF, 0);
	sfc_writel(sfc, SFC_DEV_STA_EXP, 0);
	sfc_writel(sfc, SFC_DEV_STA_MSK, 0);
	sfc_writel(sfc, SFC_TRAN_LEN, 0);
	sfc_writel(sfc, SFC_MEM_ADDR, 0);
	sfc_writel(sfc, SFC_TRIG, 0);
	sfc_writel(sfc, SFC_SCR, 0);
	sfc_writel(sfc, SFC_INTC, 0);
	sfc_writel(sfc, SFC_CGE, 0);
	sfc_writel(sfc, SFC_RM_DR, 0);
}

static inline void sfc_flush_fifo(struct sfc *sfc)
{
	sfc_writel(sfc, SFC_TRIG, TRIG_FLUSH);
}

static inline void sfc_clear_all_intc(struct sfc *sfc)
{
	sfc_writel(sfc, SFC_SCR, 0x1f);
}

static inline void sfc_mask_all_intc(struct sfc *sfc)
{
	sfc_writel(sfc, SFC_INTC, 0x1f);
}

static void sfc_set_phase_num(struct sfc *sfc, uint32_t num)
{
	uint32_t tmp;

	tmp = sfc_readl(sfc, SFC_GLB);
	tmp &= ~GLB_PHASE_NUM_MSK;
	tmp |= num << GLB_PHASE_NUM_OFFSET;
	sfc_writel(sfc, SFC_GLB, tmp);
}

static void sfc_threshold(struct sfc *sfc, uint32_t value)
{
	uint32_t tmp;
	tmp = sfc_readl(sfc, SFC_GLB);
	tmp &= ~GLB_THRESHOLD_MSK;
	tmp |= value << GLB_THRESHOLD_OFFSET;
	sfc_writel(sfc, SFC_GLB, tmp);
}

static void sfc_smp_delay(struct sfc *sfc, uint32_t value)
{
	uint32_t tmp;
	tmp = sfc_readl(sfc, SFC_DEV_CONF);
	tmp &= ~DEV_CONF_SMP_DELAY_MSK;
	tmp |= value << DEV_CONF_SMP_DELAY_OFFSET;
	sfc_writel(sfc, SFC_DEV_CONF, tmp);
}

int32_t set_flash_timing(struct sfc *sfc, uint32_t t_hold, uint32_t t_setup, uint32_t t_shslrd, uint32_t t_shslwr)
{
	uint32_t c_hold;
	uint32_t c_setup;
	uint32_t t_in, c_in;
	uint32_t cycle;
	uint32_t rate, tmp;

	rate = sfc->src_clk / 1000000;
	cycle = 1000 / rate;

	c_hold = t_hold / cycle;
	c_setup = t_setup / cycle;
	t_in = max(t_shslrd, t_shslwr);
	c_in = t_in / cycle;
	if(c_in > 0xf)
		c_in = 0xf;
	tmp = sfc_readl(sfc, SFC_DEV_CONF);
	tmp &= ~(DEV_CONF_THOLD_MSK | DEV_CONF_TSETUP_MSK | DEV_CONF_TSH_MSK);

	tmp |= ((c_hold << DEV_CONF_THOLD_OFFSET) |
		(c_setup << DEV_CONF_TSETUP_OFFSET) |
		(c_in << DEV_CONF_TSH_OFFSET));

	sfc_writel(sfc, SFC_DEV_CONF, tmp);

	return 0;
}

static inline void sfc_set_length(struct sfc *sfc, uint32_t value)
{
	sfc_writel(sfc, SFC_TRAN_LEN, value);
}

static void sfc_transfer_mode(struct sfc *sfc, uint32_t value)
{
	uint32_t tmp;
	tmp = sfc_readl(sfc, SFC_GLB);
	if(value == 0) {
		tmp &= ~GLB_OP_MODE;
	} else {
		tmp |= GLB_OP_MODE;
	}
	sfc_writel(sfc, SFC_GLB, tmp);
}

static inline void sfc_read_data(struct sfc *sfc, uint32_t *value)
{
	*value = sfc_readl(sfc, SFC_RM_DR);
}

static void sfc_write_data(struct sfc *sfc, const uint32_t value)
{
	sfc_writel(sfc, SFC_RM_DR, value);
}

static void cpu_read_rxfifo(struct sfc *sfc)
{
	int32_t i;
	uint32_t align_len = 0;
	uint32_t fifo_num = 0;
	uint32_t last_word = 0;
	uint32_t unalign_data;
	uint8_t *c;

	align_len = ALIGN(sfc->transfer->len, 4);

	if(((align_len - sfc->transfer->cur_len) / 4) > THRESHOLD) {
		fifo_num = sfc->threshold;
		last_word = 0;
	} else {
		/* last aligned THRESHOLD data*/
		if(sfc->transfer->len % 4) {
			fifo_num = (align_len - sfc->transfer->cur_len) / 4 - 1;
			last_word = 1;
		} else {
			fifo_num = (align_len - sfc->transfer->cur_len) / 4;
			last_word = 0;
		}
	}

	if ((uint32_t)sfc->transfer->data & 0x3) {
		/* addr not align */
		for (i = 0; i < fifo_num; i++) {
			sfc_read_data(sfc, &unalign_data);
			c = sfc->transfer->data;
			c[0] = (unalign_data >> 0) & 0xff;
			c[1] = (unalign_data >> 8) & 0xff;
			c[2] = (unalign_data >> 16) & 0xff;
			c[3] = (unalign_data >> 24) & 0xff;
			sfc->transfer->data += 4;
			sfc->transfer->cur_len += 4;
		}
	} else {

		for(i = 0; i < fifo_num; i++) {
			sfc_read_data(sfc, (uint32_t *)sfc->transfer->data);
			sfc->transfer->data += 4;
			sfc->transfer->cur_len += 4;
		}
	}

	/* last word */
	if(last_word == 1) {

		sfc_read_data(sfc, &unalign_data);
		c = sfc->transfer->data;

		for(i = 0; i < sfc->transfer->len % 4; i++) {
			c[i] = (unalign_data >> (i * 8)) & 0xff;
		}

		sfc->transfer->data += sfc->transfer->len % 4;
		sfc->transfer->cur_len += 4;
	}
}

static void cpu_write_txfifo(struct sfc *sfc)
{
	uint32_t align_len = 0;
	uint32_t fifo_num = 0;
	uint8_t i;

	align_len = ALIGN(sfc->transfer->len , 4);

	if (((align_len - sfc->transfer->cur_len) / 4) > THRESHOLD){
		fifo_num = THRESHOLD;
	} else {
		fifo_num = (align_len - sfc->transfer->cur_len) / 4;
	}

	for(i = 0; i < fifo_num; i++) {
		sfc_write_data(sfc, *(uint32_t *)sfc->transfer->data);
		sfc->transfer->data += 4;
		sfc->transfer->cur_len += 4;
	}

}

uint32_t sfc_get_sta_rt(struct sfc *sfc)
{
	return sfc_readl(sfc, SFC_DEV_STA_RT);
}

static inline void sfc_dev_sta_exp(struct sfc *sfc, uint32_t value)
{
	sfc_writel(sfc, SFC_DEV_STA_EXP, value);
}

static inline void sfc_dev_sta_msk(struct sfc *sfc, uint32_t value)
{
	sfc_writel(sfc, SFC_DEV_STA_MSK, value);
}

static inline void sfc_set_mem_addr(struct sfc *sfc, uint32_t addr)
{
	sfc_writel(sfc, SFC_MEM_ADDR, addr);
}

static int32_t sfc_sr_handle(struct sfc *sfc)
{
	uint32_t reg_sr = 0;
	uint32_t tmp = 0;

	while (1) {
		reg_sr = sfc_readl(sfc, SFC_SR);

		if (reg_sr & CLR_RREQ) {
			sfc_writel(sfc, SFC_SCR, CLR_RREQ);
			cpu_read_rxfifo(sfc);
		}

		if (reg_sr & CLR_TREQ) {
			sfc_writel(sfc, SFC_SCR, CLR_TREQ);
			cpu_write_txfifo(sfc);
		}

		if(reg_sr & CLR_END){
			tmp = CLR_END;
			break;
		}

		if(reg_sr & CLR_UNDER){
			tmp = CLR_UNDER;
			printf("underun");
			break;
		}

		if(reg_sr & CLR_OVER){
			tmp = CLR_OVER;
			printf("overrun");
			break;
		}
	}
	if (tmp)
		sfc_writel(sfc, SFC_SCR, tmp);

	return 0;
}

static int sfc_start_transfer(struct sfc *sfc)
{
	sfc_mask_all_intc(sfc);
	sfc_clear_all_intc(sfc);
	sfc_writel(sfc, SFC_TRIG, TRIG_START);	    //sfc start transfer

	return sfc_sr_handle(sfc);
}

static void sfc_set_tran_config(struct sfc *sfc, struct sfc_transfer *transfer, uint32_t channel)
{
	uint32_t tmp = 0;

	tmp = (transfer->sfc_mode << TRAN_CONF_TRAN_MODE_OFFSET)        \
		  | (transfer->addr_len << ADDR_WIDTH_OFFSET)         \
		  | (transfer->cmd_info.pollen << TRAN_CONF_POLL_OFFSET)    \
		  | (TRAN_CONF_CMDEN)                     \
		  | (0 << TRAN_CONF_FMAT_OFFSET)                  \
		  | (transfer->data_dummy_bits << DMYBITS_OFFSET)         \
		  | (transfer->cmd_info.dataen << TRAN_CONF_DATEEN_OFFSET)   \
		  | transfer->cmd_info.cmd;

	sfc_writel(sfc, SFC_TRAN_CONF(channel), tmp);
}

static void sfc_phase_transfer(struct sfc *sfc,struct sfc_transfer *
		transfer, uint32_t channel)
{
	sfc_writel(sfc, SFC_DEV_ADDR(channel), transfer->addr);    //set addr
	sfc_writel(sfc, SFC_DEV_ADDR_PLUS(channel), transfer->addr_plus);  //set plus addr
	sfc_set_tran_config(sfc, transfer, channel);
}

static void sfc_set_glb_config(struct sfc *sfc, struct sfc_transfer *transfer)
{
	uint32_t tmp = sfc_readl(sfc, SFC_GLB);

	if (transfer->direction == GLB_TRAN_DIR_READ)
		tmp &= ~GLB_TRAN_DIR;
	else
		tmp |= GLB_TRAN_DIR;

	if (transfer->ops_mode == DMA_OPS)
		tmp |= GLB_OP_MODE;
	else
		tmp &= ~GLB_OP_MODE;

	sfc_writel(sfc, SFC_GLB, tmp);
}

extern void flush_cache_all(void);
static void sfc_glb_info_config(struct sfc *sfc, struct sfc_transfer *transfer)
{
	sfc_set_length(sfc, transfer->len);
	if((transfer->ops_mode == DMA_OPS)){
		flush_cache_all();
		sfc_set_mem_addr(sfc, virt_to_phys(transfer->data));
	}else{
		sfc_set_mem_addr(sfc, 0);
	}
	sfc_set_glb_config(sfc, transfer);
}

#ifdef	DEBUG
static void  dump_transfer(struct sfc_transfer *xfer,int num)
{
	printf("\n");
	printf("cmd[%d].cmd = 0x%02x\n",num,xfer->cmd_info->cmd);
	printf("cmd[%d].addr_len = %d\n",num,xfer->addr_len);
	printf("cmd[%d].dummy_byte = %d\n",num,xfer->data_dummy_bits);
	printf("cmd[%d].dataen = %d\n",num,xfer->cmd_info->dataen);
	printf("cmd[%d].sta_exp = %d\n",num,xfer->cmd_info->sta_exp);
	printf("cmd[%d].sta_msk = %d\n",num,xfer->cmd_info->sta_msk);


	printf("transfer[%d].addr = 0x%08x\n",num,xfer->addr);
	printf("transfer[%d].len = %d\n",num,xfer->len);
	printf("transfer[%d].data = 0x%p\n",num,xfer->data);
	printf("transfer[%d].direction = %d\n",num,xfer->direction);
	printf("transfer[%d].sfc_mode = %d\n",num,xfer->sfc_mode);
	printf("transfer[%d].ops_mode = %d\n",num,xfer->ops_mode);
}
#endif

int32_t sfc_sync(struct sfc *sfc, struct sfc_transfer *head)
{
	uint8_t phase_num = 0;
	struct sfc_transfer *xfer = head;

	sfc_flush_fifo(sfc);
	sfc_set_length(sfc, 0);
	do {

		sfc_phase_transfer(sfc, xfer, phase_num);    //set phase
		if(xfer->cmd_info.pollen) {			     //set polling
			sfc_dev_sta_exp(sfc, xfer->cmd_info.sta_exp);
			sfc_dev_sta_msk(sfc, xfer->cmd_info.sta_msk);
		}

		if(xfer->cmd_info.dataen && xfer->len) {     //set mem
			sfc_glb_info_config(sfc, xfer);
			if(xfer->ops_mode == CPU_OPS)
				sfc->transfer = xfer;
			if(xfer->ops_mode == DMA_OPS)
				xfer->cur_len = xfer->len;
		}

		phase_num++;
	} while (&xfer->list != head->list.prev &&
		(xfer = list_entry(xfer->list.next, typeof(*xfer), list)));

	sfc_set_phase_num(sfc, phase_num);
	return sfc_start_transfer(sfc);
}

void sfc_transfer_del(struct sfc_transfer *entry)
{
	list_del(&entry->list);
}

void sfc_list_add_tail(struct sfc_transfer *new, struct sfc_transfer *head)
{
	list_add_tail(&new->list, &head->list);
}

void sfc_list_init(struct sfc_transfer *head)
{
	INIT_LIST_HEAD(&head->list);
}

static inline void sfc_dev_conf_init(struct sfc *sfc) {

	/*set ce, wp, hold pin for high level*/
	sfc_writel(sfc, SFC_DEV_CONF,
		DEV_CONF_HOLDDL | DEV_CONF_WPDL | DEV_CONF_CEDL);
}

void sfc_ctl_init(struct sfc *sfc)
{
	sfc_writel(sfc, SFC_TRIG, TRIG_STOP);	    /*sfc stop*/
	sfc_init(sfc);

	/*set dev config*/
	sfc_dev_conf_init(sfc);

	sfc_mask_all_intc(sfc);
	sfc_clear_all_intc(sfc);

	sfc_threshold(sfc, sfc->threshold);
	/*config the sfc pin init state*/

	sfc_transfer_mode(sfc, SLAVE_MODE);
	if(sfc->src_clk >= 100000000){
		sfc_smp_delay(sfc, DEV_CONF_HALF_CYCLE_DELAY);
	}
}

struct sfc *sfc_res_init(uint32_t sfc_rate)
{
	struct sfc *sfc = NULL;
	sfc = malloc(sizeof(struct sfc));
	if (!sfc) {
		printf("ERROR: %s %d kzalloc() error !\n",__func__,__LINE__);
		return ERR_PTR(-ENOMEM);
	}
	memset(sfc, 0, sizeof(struct sfc));

	sfc->src_clk = sfc_rate;
	clk_set_rate(SFC, sfc->src_clk);

	sfc->threshold = THRESHOLD;

	sfc_ctl_init(sfc);

	return sfc;

}
