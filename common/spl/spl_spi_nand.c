#include <common.h>
#include <config.h>
#include <spl.h>
#include <asm/io.h>
#include <nand.h>
#include <asm/arch/clk.h>
#include <asm/arch/base.h>
//#include <asm/arch/sfc.h>
#include <asm/arch/spi.h>


#define SSI_BASE SSI0_BASE
#define SPL_TYPE_FLAG_LEN	6


static uint32_t jz_spi_readl(unsigned int offset)
{
	return readl(SSI_BASE + offset);
}

static void jz_spi_writel(unsigned int value, unsigned int offset)
{
	writel(value, SSI_BASE + offset);
}
static void jz_spi_flush(void )
{
	jz_spi_writel(jz_spi_readl(SSI_CR0) | SSI_CR0_TFLUSH | SSI_CR0_RFLUSH, SSI_CR0);
}

static unsigned int spi_rxfifo_empty(void )
{
	return (jz_spi_readl(SSI_SR) & SSI_SR_RFE);
}
static unsigned int spi_txfifo_full(void )
{
	return (jz_spi_readl(SSI_SR) & SSI_SR_TFF);
}

static int spi_claim_bus(struct spi_slave *slave)
{
	jz_spi_writel(jz_spi_readl(SSI_CR1) | SSI_CR1_UNFIN, SSI_CR1);
	return 0;
}

static void spi_release_bus(struct spi_slave *slave)
{
	jz_spi_writel(jz_spi_readl(SSI_CR1) & (~SSI_CR1_UNFIN), SSI_CR1);
	jz_spi_writel(jz_spi_readl(SSI_SR) & (~SSI_SR_UNDR) , SSI_SR);
}


static unsigned int spi_get_rxfifo_count(void )
{
	return ((jz_spi_readl(SSI_SR) & SSI_SR_RFIFONUM_MASK) >> SSI_SR_RFIFONUM_BIT);
}

static void jz_cs_reversal(void )
{
	spi_release_bus(NULL);

	udelay(1);

	spi_claim_bus(NULL);

	return ;
}

static void spi_send_cmd(unsigned char *cmd, unsigned int count)
{
	unsigned int sum = count;
	jz_spi_flush();
	while(!spi_rxfifo_empty());
	while(count) {
		jz_spi_writel(*cmd, SSI_DR);
		while (spi_txfifo_full());
		cmd++;
		count--;
	}
	while (spi_get_rxfifo_count() != sum);
}
static void spi_recv_cmd(unsigned char *read_buf, unsigned int count)
{
	unsigned int offset = 0;
	jz_spi_flush();
	while(!spi_rxfifo_empty());
	while (count) {
		jz_spi_writel(0, SSI_DR);
		while (spi_rxfifo_empty());
		writeb(jz_spi_readl(SSI_DR), read_buf + offset);
		offset++;
		count--;
	}
}


static void get_spinand_base_param(int *pagesize)
{
	unsigned char *spl_flag = (unsigned char *)0xF4001000;
	int type_len = SPL_TYPE_FLAG_LEN;

	*pagesize = spl_flag[type_len + 5] * 1024;//pagesize off 5,blocksize off 4
}
static int spi_read_page(unsigned int page,unsigned char *dst_addr,int pagesize)
{
	unsigned char cmd[5];
	unsigned int src_addr;
	int read_buf;
	u_char dummy;
	int column = 0;
	int t_read = 120;

	jz_cs_reversal();
	cmd[0] = 0x13;
	cmd[1] = (page >> 16) & 0xff;
	cmd[2] = (page >> 8) & 0xff;
	cmd[3] = page & 0xff;
	spi_send_cmd(cmd, 4);
	udelay(t_read);

	jz_cs_reversal();
	cmd[0] = 0x0f;
	cmd[1] = 0xc0;
	spi_send_cmd(cmd, 2);

	spi_recv_cmd(&read_buf, 1);
	while(read_buf & 0x1)
		spi_recv_cmd(&read_buf, 1);

	if((read_buf & 0x30) == 0x20) {
		printf("read error !!!\n");
		return -1;
	}
	jz_cs_reversal();
	cmd[0] = 0x03;
	cmd[1] = (column >> 8) & 0xff;
	cmd[2] = column & 0xff;
	cmd[3] = 0x0;
	spi_send_cmd(cmd, 4);

	//if(column)
	//	spi_recv_cmd(&dummy,2);

	spi_recv_cmd(dst_addr, pagesize);
}

void spi_nand_load(long offs,long size,void *dst)
{
	int pagesize,page;
	int pagecopy_cnt = 0;

	get_spinand_base_param(&pagesize);
	page = offs / pagesize;

	while (pagecopy_cnt * pagesize < size) {
		spi_read_page(page,(unsigned char *)dst,pagesize);

		dst += pagesize;
		page++;
		pagecopy_cnt++;
	}
	return ;
}

static void spi_init(void)
{
	//clk_set_rate(SSI, 24000000);

	jz_spi_writel(~SSI_CR0_SSIE & jz_spi_readl(SSI_CR0), SSI_CR0);
	jz_spi_writel(0, SSI_GR);
	jz_spi_writel(SSI_CR0_EACLRUN | SSI_CR0_RFLUSH | SSI_CR0_TFLUSH, SSI_CR0);
	jz_spi_writel(SSI_FRMHL_CE0_LOW_CE1_LOW | SSI_GPCMD | SSI_GPCHL_HIGH | SSI_CR1_TFVCK_3 | SSI_CR1_TCKFI_3 | SSI_CR1_FLEN_8BIT | SSI_CR1_PHA | SSI_CR1_POL, SSI_CR1);
	jz_spi_writel(SSI_CR0_SSIE | jz_spi_readl(SSI_CR0), SSI_CR0);

}
static void gpio_as_spi(void)
{
	writel(0x3c << 26,GPIO_PXINTC(0));
	writel(0x3c << 26,GPIO_PXMSKC(0));
	writel(0x3c << 26,GPIO_PXPAT1S(0));
	writel(0x3c << 26,GPIO_PXPAT0C(0));
}
void spl_spi_nand_load_image(void)
{
	struct image_header *header;
	char cmd[5] = {0};
	char idcode[5] = {0};

	header = (struct image_header *)(CONFIG_SYS_TEXT_BASE);

	spi_init();
	spi_claim_bus(NULL);
	spl_parse_image_header(header);
	spi_nand_load(CONFIG_UBOOT_OFFSET,CONFIG_SYS_MONITOR_LEN,(void *)CONFIG_SYS_TEXT_BASE);

}

