#include <common.h>
#include <spl.h>
#include <asm/u-boot.h>
#include <fat.h>
#include <version.h>
#include <mmc.h>
#include <asm/arch/clk.h>
#include <asm/arch/mmc.h>
#include <asm/io.h>

//#define DEBUG_MSC
//#define DEBUG_DDR_CONTENT

#ifdef DEBUG_MSC
#define msc_debug	printf
#else
#define msc_debug(fmt, args...) do { }while(0)
#endif

/* global variables */
static uint32_t io_base = MSC0_BASE;
static int highcap = 0;

#define MSC_BUS_WIDTH_1            0
#define MSC_BUS_WIDTH_4            2
#define MSC_BUS_WIDTH_8            3

#if defined(CONFIG_SPL_JZ_MSC_BUS_8BIT)
static uint32_t bus_width = MSC_BUS_WIDTH_8;
#else
/* static uint32_t bus_width = MSC_BUS_WIDTH_4;*/
static uint32_t bus_width = MSC_BUS_WIDTH_1;
#endif

static uint32_t msc_readl(uint32_t off)
{
	return readl(io_base + off);
}
static uint16_t msc_readw(uint32_t off)
{
	return readw(io_base + off);
}
static uint16_t msc_readb(uint32_t off)
{
	return readb(io_base + off);
}

static void msc_writel(uint32_t off, uint32_t value)
{
	writel(value, io_base + off);
}
static void msc_writew(uint32_t off, uint32_t value)
{
	writew(value, io_base + off);
}
static void msc_writeb(uint32_t off, uint32_t value)
{
	writeb(value, io_base + off);
}

#ifdef DEBUG_MSC
static void dump_error_status(void)
{
	msc_debug("PSTATE_REG				= %x\n", msc_readl(MSC_PSTATE_REG));
	msc_debug("XFER_MODE_R				= %x\n", msc_readw(MSC_XFER_MODE_R));
	msc_debug("NORMAL_INT_STAT_EN_R	= %x\n", msc_readw(MSC_NORMAL_INT_STAT_EN_R));
	msc_debug("NORMAL_INT_STAT_R		= %x\n", msc_readw(MSC_NORMAL_INT_STAT_R));
	msc_debug("ERROR_INT_STAT_EN_R		= %x\n", msc_readw(MSC_ERROR_INT_STAT_EN_R));
	msc_debug("ERROR_INT_STAT_R		= %x\n", msc_readw(MSC_ERROR_INT_STAT_R));
	msc_debug("ADMA_ERR_STAT_R			= %x\n", msc_readw(MSC_ADMA_ERR_STAT_R));
	msc_debug("HOST_CTRL1_R			= %x\n", msc_readb(MSC_HOST_CTRL1_R));
	msc_debug("HOST_CTRL2_R			= %x\n", msc_readw(MSC_HOST_CTRL2_R));
}
#else
static void dump_error_status(void) {}
#endif


static void msc_reset(u8 mask)
{
	u32 timeout = 0xffff;

	msc_writeb(MSC_SW_RST_R, mask);
	while((msc_readb(MSC_SW_RST_R) & mask) && --timeout) {
		udelay(10);
	}

	if(!timeout)
		printf("mshc host reset=0x%x fail!\n", mask);
}

static void mmc_init_host(void)
{
	msc_reset(MSC_SW_RST_ALL_BIT);

	msc_writew(MSC_NORMAL_INT_STAT_EN_R, 0xffff);
	msc_writew(MSC_ERROR_INT_STAT_EN_R, 0xffff);

	msc_writeb(MSC_TOUT_CTRL_R, 0xe);
}

static void msc_clk_switch(int high_frq)
{
	uint32_t val;

#ifdef CONFIG_FPGA
#define CPM_MSC0_CLK_R			  (0xB0000068)
#define CPM_MSC1_CLK_R			  (0xB00000a4)
#define MSC_CLK_H_FREQ			  (0x1 << 20)
  #ifdef CONFIG_JZ_MMC_MSC0
	#define CPM_MSC_CLK_R   CPM_MSC0_CLK_R
  #endif
  #ifdef CONFIG_JZ_MMC_MSC1
	#define CPM_MSC_CLK_R   CPM_MSC1_CLK_R
  #endif
	/* set clk */
	val = readl(CPM_MSC_CLK_R);
	if(high_frq)
		val |= MSC_CLK_H_FREQ;
	else
		val &= ~MSC_CLK_H_FREQ;
	writel(val, CPM_MSC_CLK_R);
#else //CONFIG_FPGA
  #define MSC_INIT_CLK		200000
  #define MSC_WORKING_CLK	24000000
  #ifdef CONFIG_JZ_MMC_MSC0
	#define CPM_MSC	MSC0
  #endif
  #ifdef CONFIG_JZ_MMC_MSC1
	#define CPM_MSC	MSC1
  #endif
	/* TODO: set clk */
	msc_writew(MSC_CLK_CTRL_R, MSC_SD_CLK_EN_BIT | MSC_INTERNAL_CLK_EN_BIT);
	/* set clk */
	if (!high_frq) {
		clk_set_rate(CPM_MSC, MSC_INIT_CLK);
	}
	else
		clk_set_rate(CPM_MSC, MSC_WORKING_CLK);

	printf("%s : clk_id[%d], set clk[%d], clk_get_rate=%d\n", __func__,
			CPM_MSC, MSC_WORKING_CLK, clk_get_rate(CPM_MSC));
#endif //CONFIG_FPGA

}

#ifdef DEBUG_MSC
static u32 cmd_err_stat_check(void)
{
	if(msc_readw(MSC_ERROR_INT_STAT_R) \
			& (MSC_CMD_TOUT_ERR_STAT_BIT \
			| MSC_CMD_CRC_ERR_STAT_BIT \
			| MSC_CMD_END_BIT_ERR_STAT_BIT \
			| MSC_CMD_IDX_ERR_STAT_BIT)){
		msc_debug("[ERROR]:Command check Error!!!\n");
		dump_error_status();
		msc_writew(MSC_ERROR_INT_STAT_R, 0xffff);
		return -1;
	}
	return 0;
}

static u32 xfer_err_stat_check(void)
{
	if(msc_readw(MSC_ERROR_INT_STAT_R) \
			& (MSC_DATA_TOUT_ERR_STAT_BIT \
			| MSC_DATA_CRC_ERR_STAT_BIT)){
		msc_debug("[ERROR]:Data Xfer Error!!!\n");
		dump_error_status();
		msc_writew(MSC_ERROR_INT_STAT_R, 0xffff);
		return -1;
	}
	return 0;
}
#endif

static u32 wait_cmd_complete(void)
{
	u32 timeout = 100000;

	while(!(msc_readw(MSC_NORMAL_INT_STAT_R) \
				& MSC_CMD_COMPLETE_STAT_BIT) && --timeout){
		udelay(10);
	}

	if(!timeout) {
		printf("[ERROR]: MSC_ERROR_INT_STAT_R : %x, cmd timeout...\n", msc_readw(MSC_ERROR_INT_STAT_R));
		return -1;
	}

	msc_writew(MSC_NORMAL_INT_STAT_R, MSC_CMD_COMPLETE_STAT_BIT);

	return 0;
}

static u32 wait_xfer_complete(void)
{
	u32 timeout = 0xffff;

	while(!(msc_readw(MSC_NORMAL_INT_STAT_R) \
				& MSC_XFER_COMPLETE_STAT_BIT) && --timeout){
		udelay(10);
	}

	if(!timeout) {
		printf("[ERROR]: MSC_ERROR_INT_STAT_R : %x, xfer timeout...\n", msc_readw(MSC_ERROR_INT_STAT_R));
		return -1;
	}

	msc_writew(MSC_NORMAL_INT_STAT_R, MSC_XFER_COMPLETE_STAT_BIT);

	return 0;
}

static u32 wait_buf_rb(void)
{
	unsigned int timeout = 0xffff;

	while(!(msc_readw(MSC_NORMAL_INT_STAT_R) \
				& MSC_BUF_RD_READY_STAT_BIT) && --timeout){
		udelay(10);  //1 block read time
	}

	if(!timeout) {
		printf("[ERROR]: MSC_ERROR_INT_STAT_R: %x, buf read timeout err ...\n", msc_readw(MSC_ERROR_INT_STAT_R));
		dump_error_status();
		return -1;
	}

	msc_writew(MSC_NORMAL_INT_STAT_R, MSC_BUF_RD_READY_STAT_BIT);

	return 0;
}
static u16 package_cmd(u32 cmdidx, u32 cmdat, u32 rtype)
{
	u16 cmd = 0;

	cmd |= cmdidx << MSC_CMD_INDEX_LBIT;

	switch (rtype) {
		case MSC_CMDAT_RESPONSE_NONE:
			cmd &= ~MSC_CMD_IDX_CHK_ENABLE_BIT;
			cmd	&= ~MSC_CMD_CRC_CHK_ENABLE_BIT; //command index & crc check disable
			break;
		case MSC_CMDAT_RESPONSE_R1:
		case MSC_CMDAT_RESPONSE_R7:
			cmd |= MSC_CMD_IDX_CHK_ENABLE_BIT | MSC_CMD_CRC_CHK_ENABLE_BIT;
			cmd &= ~MSC_RESP_TYPE_SELECT_MASK;
			cmd |= MSC_RESP_TYPE_SELECT_RESP_LEN_48; //length 48
			break;
		case MSC_CMDAT_RESPONSE_R1b:
			cmd |= MSC_CMD_IDX_CHK_ENABLE_BIT | MSC_CMD_CRC_CHK_ENABLE_BIT;
			cmd &= ~MSC_RESP_TYPE_SELECT_MASK;
			cmd |= MSC_RESP_TYPE_SELECT_RESP_LEN_48B; //length 48B
			break;
		case MSC_CMDAT_RESPONSE_R2:
			cmd &= ~(MSC_CMD_IDX_CHK_ENABLE_BIT | MSC_CMD_CRC_CHK_ENABLE_BIT);
			cmd &= ~MSC_RESP_TYPE_SELECT_MASK;
			cmd |= MSC_RESP_TYPE_SELECT_RESP_LEN_136; //length 136
			break;
		case MSC_CMDAT_RESPONSE_R3:
		case MSC_CMDAT_RESPONSE_R6:
			cmd &= ~(MSC_CMD_IDX_CHK_ENABLE_BIT | MSC_CMD_CRC_CHK_ENABLE_BIT);
			cmd &= ~MSC_RESP_TYPE_SELECT_MASK;
			cmd |= MSC_RESP_TYPE_SELECT_RESP_LEN_48; //length 48
			break;
		default:
			break;
	}

	return cmd;
}

static  u8* msc_get_resp(void)
{
	static u8 resp[20] = {0};
	u32 RESP_ARRAY[4];
	u32 words, i, j;

	memset(RESP_ARRAY, 0, sizeof(RESP_ARRAY));
	RESP_ARRAY[3] = msc_readl(MSC_RESP01_R);
	RESP_ARRAY[2] = msc_readl(MSC_RESP23_R);
	RESP_ARRAY[1] = msc_readl(MSC_RESP45_R);
	RESP_ARRAY[0] = msc_readl(MSC_RESP67_R);

	words = 1; //low lost eight
	memset(resp, 0, sizeof(u8)*20);
	for(i = 4; i > 0; i--){
		for(j = 0; j < 4; j++){
			resp[words] = (RESP_ARRAY[i - 1] & (0xff << j * 8)) >> j * 8;
			words++;
		}
	}
#if 0
	printf("Response of CMD\n");
	for(i=0; i<=3; i++){
		printf("\tRESP%d%d=0x%08x\n",6-(i*2),7-(i*2), RESP_ARRAY[i]);
	}
#endif

	return resp;
}

static u32 msc_check_cmd_data_line(u32 cmdidx)
{
	unsigned int mask = 0;
	unsigned int timeout = 100;

	mask = MSC_CMD_INHIBIT_BIT | MSC_CMD_INHIBIT_DAT_BIT;

	/* We shouldn't wait for data inihibit for stop commands, even
	   though they might use busy signaling */
	if (cmdidx == MMC_CMD_STOP_TRANSMISSION)
		mask &= ~MSC_CMD_INHIBIT_DAT_BIT;

	while (msc_readl(MSC_PSTATE_REG) & mask) {
		if (timeout == 0) {
			printf("Controller never released inhibit bit(s).\n");
			return -1;
		}
		timeout--;
		udelay(1000);
	}

	return 0;
}

static u8* mmc_cmd(u32 cmdidx, u32 arg, u32 cmdat, u32 rtype)
{
	u16 cmd_args;
	u8 *resp = NULL;

	if(msc_check_cmd_data_line(cmdidx))
		goto exit;

	cmd_args = package_cmd(cmdidx, cmdat, rtype);
	cmd_args |= cmdat;

	msc_writel(MSC_ARGUMENT_R, arg);
	msc_writew(MSC_CMD_R, cmd_args);

	msc_debug("cmdidx=%d, cmd_args=0x%x, arg=0x%x, response=R%d\n", cmdidx, cmd_args, arg, rtype);

	if(wait_cmd_complete())
		goto exit;

#ifdef DEBUG_MSC
	if(cmd_err_stat_check())
		goto exit;
#endif
	resp = msc_get_resp();


exit:
	if(!(cmdat & MSC_DATA_PRESENT_SEL_BIT)) {
		msc_reset(MSC_SW_RST_CMD_BIT);
		msc_reset(MSC_SW_RST_DAT_BIT);
	}

	return resp;
}

static void msc_set_xfer_bus_width(unsigned int buswidth)
{
	u32 val;

	/* bus width */
	val = msc_readb(MSC_HOST_CTRL1_R);

	switch (buswidth) {
		case MSC_BUS_WIDTH_1:
			val &= ~MSC_DAT_XFER_WIDTH_BIT;  //1bit mode
			break;
		case MSC_BUS_WIDTH_4:
			val |= MSC_DAT_XFER_WIDTH_BIT;  //4bit mode
			break;
		case MSC_BUS_WIDTH_8:
			val |= MSC_EXT_DAT_XFER_BIT;  //8bit mode
			break;
		default:
			val &= ~MSC_DAT_XFER_WIDTH_BIT;  //1bit mode
			break;
	}

	msc_writeb(MSC_HOST_CTRL1_R, val);
}

static u32 mmc_block_read(u32 start, u32 blkcnt, u32 *dst)
{
	u32 cnt, nob;
	u32 cmd_args = 0;
	u32 xfer_data = 0;

	if(blkcnt <= 0)
		return -1;

	if(highcap == 0) {
		/* standard capacity : Bytes addressed */
		cmd_args = start * 512;
	} else {
		/* high capacity: sector addressed */
		cmd_args = start;
	}

	msc_debug("%s-->start: %d \n", __func__, start);
	msc_debug("%s-->blkcnt: %d \n", __func__, blkcnt);
	msc_debug("%s-->dst: 0x%x\n", __func__, dst);
	msc_debug("%s-->bus_width: %d\n", __func__, bus_width);

	nob = blkcnt;
	msc_writew(MSC_BLOCKSIZE_R, 0x200);
	msc_writew(MSC_BLOCKCOUNT_R, nob);

	msc_set_xfer_bus_width(bus_width);
	msc_debug("HOST_CTRL1_R			= %x\n", msc_readb(MSC_HOST_CTRL1_R));

	xfer_data = msc_readw(MSC_XFER_MODE_R);
	xfer_data |= (MSC_BLOCK_COUNT_ENABLE_BIT | MSC_DATA_XFER_DIR_BIT  \
				| MSC_AUTO_CMD12_ENABLE);
	if (1 != blkcnt)
		xfer_data |= MSC_MULTI_BLK_SEL_BIT;

	msc_writew(MSC_XFER_MODE_R, xfer_data);

	mmc_cmd(MMC_CMD_SET_BLOCKLEN, 0x200, 0, MSC_CMDAT_RESPONSE_R1);
	if(1 == blkcnt)
		mmc_cmd(MMC_CMD_READ_SINGLE_BLOCK, cmd_args, MSC_DATA_PRESENT_SEL_BIT, MSC_CMDAT_RESPONSE_R1);
	else
		mmc_cmd(MMC_CMD_READ_MULTIPLE_BLOCK, cmd_args, MSC_DATA_PRESENT_SEL_BIT, MSC_CMDAT_RESPONSE_R1);

	for(; nob > 0; nob--) {
		cnt = 512 / 4;
		if(wait_buf_rb())
			goto err;

		while(cnt--) {
			*dst = msc_readl(MSC_BUF_DATA_R);
			dst++;
		}
	}

	if(wait_xfer_complete())
		goto err;
#ifdef DEBUG_MSC
	if(xfer_err_stat_check())
		goto err;
#endif

err:
	msc_reset(MSC_SW_RST_CMD_BIT);
	msc_reset(MSC_SW_RST_DAT_BIT);
	return blkcnt - nob;
}

static void msc_sync_abort(void)
{
	msc_writeb(MSC_BGAP_CTRL_R, MSC_STOP_BG_REQ_BIT);

	wait_xfer_complete();

	mmc_cmd(12, 0, MSC_CMD_TYPE_ABORT_CMD, MSC_CMDAT_RESPONSE_R1b);

	msc_reset(MSC_SW_RST_CMD_BIT);
	msc_reset(MSC_SW_RST_DAT_BIT);
}


static int sd_found(void)
{
	u8 *resp;
	u32 cardaddr, timeout = 0xffff;
	int rca;

	msc_debug("sd_found\n");
	resp = mmc_cmd(55, 0, 0, MSC_CMDAT_RESPONSE_R1);

	resp = mmc_cmd(41, 0x40ff8000, 0, MSC_CMDAT_RESPONSE_R3);

	while (timeout-- && !(resp[4] & 0x80)) {
		mdelay(1);
		resp = mmc_cmd(55, 0, 0, MSC_CMDAT_RESPONSE_R1);
		resp = mmc_cmd(41, 0x40ff8000, 0, MSC_CMDAT_RESPONSE_R3);
	}

	if (!(resp[4] & 0x80)) {
		printf("sd init fail!\n");
		return -1;
	}

	if((resp[4] & 0x60 ) == 0x40)
		highcap = 1;
	else
		highcap =0;

	resp = mmc_cmd(2, 0, 0, MSC_CMDAT_RESPONSE_R2);
	resp = mmc_cmd(3, 0, 0, MSC_CMDAT_RESPONSE_R6);
	cardaddr = (resp[4] << 8) | resp[3];
	rca = cardaddr << 16;
	resp = mmc_cmd(9, rca, 0, MSC_CMDAT_RESPONSE_R2);

	msc_clk_switch(1);
	resp = mmc_cmd(7, rca, 0, MSC_CMDAT_RESPONSE_R1);
	resp = mmc_cmd(55, rca, 0, MSC_CMDAT_RESPONSE_R1);
	resp = mmc_cmd(6, bus_width, 0, MSC_CMDAT_RESPONSE_R1);

	return 0;
}

static int mmc_found(void)
{
	u8 *resp;
	u32 buswidth_arg, buswidth, timeout = 100;

	msc_debug("mmc_found\n");

	msc_sync_abort();

	resp = mmc_cmd(0, 0, 0, MSC_CMDAT_RESPONSE_NONE);
	resp = mmc_cmd(1, 0x40ff8000, 0, MSC_CMDAT_RESPONSE_R3);

	while (timeout-- && !(resp[4] & 0x80)) {
		mdelay(1);
		resp = mmc_cmd(1, 0x40ff8000, 0, MSC_CMDAT_RESPONSE_R3);
	}

	if (!(resp[4] & 0x80)) {
		printf("mmc init fail!\n");
		return -1;
	}
	if((resp[4] & 0x60 ) == 0x40)
		highcap = 1;
	else
		highcap =0;

	resp = mmc_cmd(2, 0, 0, MSC_CMDAT_RESPONSE_R2);
	resp = mmc_cmd(3, 0x10, 0, MSC_CMDAT_RESPONSE_R1);

	msc_clk_switch(1);
	resp = mmc_cmd(7, 0x10, 0, MSC_CMDAT_RESPONSE_R1);

	buswidth = (bus_width - 1) < 0 ? 0 : (bus_width - 1);
	buswidth_arg = 0x3 << 24 | 183 << 16 | buswidth << 8 | 0x1;
	resp = mmc_cmd(6, buswidth_arg, 0, MSC_CMDAT_RESPONSE_R1); /* set buswidth*/

	return 0;
}

static int jzmmc_init(void)
{
	u8 *resp;
	u32 ret = 0;


	mmc_init_host();

	msc_clk_switch(0);

	msc_sync_abort();

	resp = mmc_cmd(0, 0, 0, MSC_CMDAT_RESPONSE_NONE);
	resp = mmc_cmd(8, 0x1aa, 0, MSC_CMDAT_RESPONSE_R1);

	resp = mmc_cmd(55, 0, 0, MSC_CMDAT_RESPONSE_R1);
	if (resp[1] & 1 << 5) {
		resp = mmc_cmd(41, 0x40ff8000, 0, MSC_CMDAT_RESPONSE_R3);
		if(*(u32*)resp)
			ret = sd_found();
		else
			ret = mmc_found();
	} else {
		ret = mmc_found();
	}

	return ret;
}

#ifdef DEBUG_DDR_CONTENT
static int dump_ddr_content(unsigned int *src, int len)
{
	int i;
	volatile unsigned int *p = src;
	for(i=0; i<len/4; i++) {
		msc_debug("%x:%x\n", &p[i], p[i]);
		src++;
	}
}
#endif

static int mmc_load_image_raw(unsigned long sector)
{
	int err = 0;
	u32 image_size_sectors;
	struct image_header *header;

	header = (struct image_header *)(CONFIG_SYS_TEXT_BASE -
					 sizeof(struct image_header));

	/* read image header to find the image size & load address */
	err = mmc_block_read(sector, 1, header);
	if (err < 0)
		goto end;

#ifdef DEBUG_DDR_CONTENT
	dump_ddr_content(header, 0x200);
#endif
	spl_parse_image_header(header);

	/* convert size to sectors - round up */
	image_size_sectors = (spl_image.size + 0x200 - 1) / 0x200;

	/* Read the header too to avoid extra memcpy */
	err = mmc_block_read(sector, image_size_sectors,
			     (void *)spl_image.load_addr);

#ifdef DEBUG_DDR_CONTENT
	dump_ddr_content(spl_image.load_addr, 200);
#endif
	flush_cache_all();
end:
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
	if (err < 0)
		msc_debug("spl: mmc blk read err - %lu\n", err);
#endif
	return err;
}

void spl_mmc_load_image(void)
{
#ifdef CONFIG_JZ_MMC_MSC0
	io_base = MSC0_BASE;
#endif
#ifdef CONFIG_JZ_MMC_MSC1
	io_base = MSC1_BASE;
#endif

	jzmmc_init();
	mmc_load_image_raw(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR);
}
