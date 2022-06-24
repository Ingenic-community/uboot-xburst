#include <common.h>
#include <spl.h>
#include <asm/u-boot.h>
#include <fat.h>
#include <version.h>
#include <mmc.h>
#include <asm/arch/clk.h>
#include <asm/arch/mmc.h>
#include <asm/io.h>

/* #define DEBUG_MSC */
/* #define DEBUG_DDR_CONTENT */
#ifdef DEBUG_MSC
#define msc_debug	printf
#else
#define msc_debug(fmt, args...) do { }while(0)
#endif

/* global variables */
static uint32_t io_base = MSC0_BASE;
static int bus_width = 0;
static int highcap = 0;


static uint32_t msc_readl(uint32_t off)
{
	return readl(io_base + off);
}
static uint16_t msc_readw(uint32_t off)
{
	return readw(io_base + off);
}
static void msc_writel(uint32_t off, uint32_t value)
{
	writel(value, io_base + off);
}


static void mmc_init_one(void)
{
	uint32_t strpcl;
	/* msc reset */
	strpcl = MSC_STRPCL_RESET;
	msc_writel(MSC_STRPCL, strpcl);
	mdelay(1);
	strpcl &= ~(MSC_STRPCL_RESET);
	msc_writel(MSC_STRPCL, strpcl);

	msc_writel(MSC_IMASK, 0xffffffff);
	msc_writel(MSC_IREG, 0xffffffff);

	msc_writel(MSC_RESTO, 0x100);
	msc_writel(MSC_RDTO, 0x1ffffff);
}


static void wait_prog_done(void)
{
	int timeout = 0xffff;
	while (!(msc_readl(MSC_STAT) & MSC_STAT_PRG_DONE) && timeout--) {
		mdelay(1);
	}
	if(timeout == 0) {
		msc_debug("wait prog done timeout !\n");
		goto err;
	}
	msc_writel(MSC_IREG, MSC_IREG_PRG_DONE);
err:
	return;
}

static u8* mmc_cmd(u16 cmd, u32 arg, u32 cmdat, u16 rtype)
{
	static u8 resp[6];
	int i;
	int timeout = 0xffff;
	msc_writel(MSC_CMD, cmd);
	msc_writel(MSC_ARG, arg);
	msc_writel(MSC_CMDAT, cmdat);

	msc_writel(MSC_STRPCL, MSC_STRPCL_START_OP);

	while (!(msc_readl(MSC_STAT) & MSC_STAT_END_CMD_RES) && timeout--) {
		mdelay(1);
	}
	if(timeout == 0) {
		msc_debug("mmc cmd timeout !!\n");
		return 0;
	}

	msc_writel(MSC_IREG, MSC_IREG_END_CMD_RES);

	if (rtype == MSC_CMDAT_RESPONSE_NONE)
		return 0;

	for (i = 2; i >= 0; i--) {
		u16 res_fifo = msc_readw(MSC_RES);

		int offset = i << 1;

		resp[offset] = ((u8 *)&res_fifo)[0];
		resp[offset + 1] = ((u8 *)&res_fifo)[1];
	}

	return resp;
}


static int mmc_block_read(u32 start, u32 blkcnt, u32 *dst)
{
	u32 stat, cnt, nob;
	int timeout = 0x1ffff;
	int cmd_args = 0;
	int cmd_data = 0;
	int i;

	if(blkcnt <= 0)
		return -1;

	if(highcap == 0) {
		/* standard capacity : Bytes addressed */
		cmd_args = start * 512;
	} else {
		/* high capacity: sector addressed */
		cmd_args = start;
	}

	msc_debug("start:%x ", start);
	msc_debug("blkcnt:%x ", blkcnt);
	msc_debug("dst:%x\n", dst);
	msc_debug("bus_width:%x\n", bus_width);

	nob = blkcnt;

	mmc_cmd(MMC_CMD_SET_BLOCKLEN, 0x200, 0x1, MSC_CMDAT_RESPONSE_R1);

	msc_writel(MSC_BLKLEN, 0x200);
	msc_writel(MSC_NOB, blkcnt);

	cmd_data = 0x9 | bus_width << 9 | 2 << 14;
	mmc_cmd(MMC_CMD_READ_MULTIPLE_BLOCK, cmd_args, cmd_data, MSC_CMDAT_RESPONSE_R1);

	for(; nob > 0; nob--) {
		for(i = 0; i< 512/256; i++) {
			while(1){
				stat = msc_readl(MSC_STAT);

				if (stat & (MSC_STAT_TIME_OUT_READ | MSC_STAT_CRC_READ_ERROR)) {
					msc_debug("mmc read timeout or crc error!, msc state:%x\n", stat);
					goto err;
				}

				if(!(stat & MSC_STAT_DATA_FIFO_EMPTY))
					break;
			}
			while (!(msc_readl(MSC_IREG) & MSC_IREG_RXFIFO_RD_REQ));

			cnt = 64;		//64 words
			while (cnt--) {
				*dst = msc_readl(MSC_RXFIFO);
				dst++;
			}
		}
	}

	mmc_cmd(MMC_CMD_STOP_TRANSMISSION, 0, 0x41, MSC_CMDAT_RESPONSE_R1);

	while (!(msc_readl(MSC_STAT) & MSC_STAT_DATA_TRAN_DONE) && timeout--) {
		mdelay(1);
	}

	if(timeout == 0) {
		printf("data trans timeout !\n");
		goto err;
	}
	msc_writel(MSC_IREG, MSC_IREG_DATA_TRAN_DONE);

err:
	return blkcnt - nob;
}


int sd_found(void)
{
	u8 *resp;
	u32 cardaddr, timeout = 1000;
	int rca;

	msc_debug("sd_found\n");

	resp = mmc_cmd(55, 0, 0x1, MSC_CMDAT_RESPONSE_R1);
	resp = mmc_cmd(41, 0x40ff8000, 0x3, MSC_CMDAT_RESPONSE_R3);

	while (timeout-- && !(resp[4] & 0x80)) {
		mdelay(1);
		resp = mmc_cmd(55, 0, 0x1, MSC_CMDAT_RESPONSE_R1);
		resp = mmc_cmd(41, 0x40ff8000, 0x3, MSC_CMDAT_RESPONSE_R3);
	}

	if (!(resp[4] & 0x80)) {
		msc_debug("sd found error!\n");
		return -1;
	}

	if((resp[4] & 0x60 ) == 0x40)
		highcap = 1;
	else
		highcap =0;

	resp = mmc_cmd(2, 0, 0x2, MSC_CMDAT_RESPONSE_R2);
	resp = mmc_cmd(3, 0, 0x6, MSC_CMDAT_RESPONSE_R6);
	cardaddr = (resp[4] << 8) | resp[3];
	rca = cardaddr << 16;

	resp = mmc_cmd(9, rca, 0x2, MSC_CMDAT_RESPONSE_R2);

#ifndef CONFIG_FPGA
	msc_writel(MSC_CLKRT, MSC_CLKRT_CLK_RATE_DIV_1);
#else
	msc_writel(MSC_CLKRT, MSC_CLKRT_CLK_RATE_DIV_32);
#endif
	resp = mmc_cmd(7, rca, 0x1, MSC_CMDAT_RESPONSE_R1);
	resp = mmc_cmd(55, rca, 0x1, MSC_CMDAT_RESPONSE_R1);
#if defined(CONFIG_MSC_DATA_WIDTH_8BIT)
	bus_width = 3;
#elif defined(CONFIG_MSC_DATA_WIDTH_4BIT)
	bus_width = 2;
#else
	bus_width = 0;
#endif
	resp = mmc_cmd(6, bus_width, 0x1 | (bus_width << 9), MSC_CMDAT_RESPONSE_R1);

	return 0;
}

int mmc_found(void)
{
	u8 *resp;
	u32 timeout = 100;

	msc_debug("mmc_found\n");

	resp = mmc_cmd(1, 0x40ff8000, 0x3, MSC_CMDAT_RESPONSE_R3);

	while (timeout-- && !(resp[4] & 0x80)) {
		mdelay(1);
		resp = mmc_cmd(1, 0x40ff8000, 0x3, MSC_CMDAT_RESPONSE_R3);
	}

	if (!(resp[4] & 0x80)) {
		printf("mmc init faild\n");
		return -1;
	}
	if((resp[4] & 0x60 ) == 0x40)
		highcap = 1;
	else
		highcap =0;

	resp = mmc_cmd(2, 0, 0x2, MSC_CMDAT_RESPONSE_R2);
	resp = mmc_cmd(3, 0x10, 0x1, MSC_CMDAT_RESPONSE_R1);


#ifndef CONFIG_FPGA
	msc_writel(MSC_CLKRT, MSC_CLKRT_CLK_RATE_DIV_1);
#else
	msc_writel(MSC_CLKRT, MSC_CLKRT_CLK_RATE_DIV_32);
#endif
	resp = mmc_cmd(7, 0x10, 0x1, MSC_CMDAT_RESPONSE_R1);

#if defined(CONFIG_MSC_DATA_WIDTH_8BIT)
	resp = mmc_cmd(6, 0x3b70201, 0x41, MSC_CMDAT_RESPONSE_R1); /*set 8bit buswidth*/
	bus_width = 0x3;
#elif defined(CONFIG_MSC_DATA_WIDTH_4BIT)
	resp = mmc_cmd(6, 0x3b70101, 0x41, MSC_CMDAT_RESPONSE_R1); /* set 4bit buswidth*/
	bus_width = 0x2;
#else
	/* default set to 1bit */
	resp = mmc_cmd(6, 0x3b70001, 0x41, MSC_CMDAT_RESPONSE_R1); /* set 1bit buswidth*/
	bus_width = 0x0;
#endif

	wait_prog_done();

	return 0;
}

static int jzmmc_init()
{
	u8 *resp;
	u32 ret = 0;
	u32 clk_set = 0, clkrt = 0;

#ifndef CONFIG_FPGA
	clk_set_rate(MSC0, CONFIG_SYS_EXTAL);
	clk_set = clk_get_rate(MSC0);
#else
	clk_set = CONFIG_SYS_EXTAL;
#endif

	mmc_init_one();

	msc_writel(MSC_LPM, 1);
	while (200000 < clk_set) {
		clkrt++;
		clk_set >>= 1;
	}
	if (clkrt > 7) {
		clkrt = 7;
	}

	msc_debug("clk_set:%d, clkrt:%d, actual_clk:%d\n",clk_set, clkrt, clk_set/(clkrt+1));
	msc_writel(MSC_CLKRT, clkrt);

	msc_debug("init stat :%x\n", msc_readl(MSC_STAT));
	/* cmd12 reset when we reading or writing from the card, send this cmd */
	resp = mmc_cmd(12, 0, 0x41, MSC_CMDAT_RESPONSE_R1);
	msc_debug("init stat :%x\n", msc_readl(MSC_STAT));

	resp = mmc_cmd(0, 0, 0x80, MSC_CMDAT_RESPONSE_NONE);
	resp = mmc_cmd(8, 0x1aa, 0x1, MSC_CMDAT_RESPONSE_R1);

	resp = mmc_cmd(55, 0, 0x1, MSC_CMDAT_RESPONSE_R1);

	if (resp[0] & 0x20){
		if (resp[5] == 0x37){
			resp = mmc_cmd(41, 0x40ff8000, 0x3, MSC_CMDAT_RESPONSE_R3);
			if (resp[5] == 0x3f)
				ret = sd_found();
			else
				ret = mmc_found();
		} else {
			ret = mmc_found();
		}
	} else {
		ret = mmc_found();
	}

	return 0;
}


#ifdef DEBUG_DDR_CONTENT
static int dump_ddr_content(unsigned int *src, int len)
{
	int i;
	for(i=0; i<len/4; i++) {
		msc_debug("%x:%x\n", src, *(unsigned int *)src);
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
#else
	io_base = MSC1_BASE;
#endif

	jzmmc_init();
	mmc_load_image_raw(CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR);
}
