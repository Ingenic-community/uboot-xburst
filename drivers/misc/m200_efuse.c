#include <common.h>
#include <malloc.h>
#include <linux/types.h>
#include <linux/string.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/errno.h>
#include <asm/arch/base.h>
#include <asm/arch/clk.h>
#include <efuse.h>
//#include <asm/arch/sc_rom.h>

//#define EFU_NO_REG_OPS

#ifdef EFU_NO_REG_OPS
#undef writel
#undef readl
#define writel(b, addr) {;}
#define readl(addr)	0
#endif

static int efuse_debug = 1;
static int efuse_gpio = -1;

#define EFUCTRL		0x00
#define EFUCFG		0x04
#define EFUSTATE	0x08
#define EFUDATA(x)	(0x0C + (x)*0x4)
#define EFUDATA_REG_NUM 8

#define EFU_ROM_BASE	0x200
#define EFU_ROM_END	0x3FF
#define EFU_CHIP_ID_BASE	0x200
#define EFU_CHIP_ID_END		0x20F
#define EFU_CHIP_NUM_BASE	0x210
#define EFU_CHIP_NUM_END	0x21F
#define EFU_COMS_ID_BASE	0x220
#define EFU_COMS_ID_END		0x22F
#define EFU_TRIM_DATA0_BASE	0x230
#define EFU_TRIM_DATA0_END	0x233
#define EFU_TRIM_DATA1_BASE	0x234
#define EFU_TRIM_DATA1_END	0x237
#define EFU_TRIM_DATA2_BASE	0x238
#define EFU_TRIM_DATA2_END	0x23B
#define EFU_TRIM_DATA3_BASE	0x23B
#define EFU_TRIM_DATA3_END	0x23D
#define EFU_PROT_BIT_BASE	0x23E
#define EFU_PROT_BIT_END	0x23F
#define EFU_ROOT_KEY_BASE	0x240
#define EFU_ROOT_KEY_END	0x24F
#define EFU_CHIP_KEY_BASE	0x250
#define EFU_CHIP_KEY_END	0x25F
#define EFU_USER_KEY_BASE	0x260
#define EFU_USER_KEY_EN		0x26F
#define EFU_MD5_BASE		0x270
#define EFU_MD5_END		0x27F
#define EFU_FIX_BT_BASE		0x280
#define EFU_FIX_BT_END		0x3FF

#define WR_ADJ_10TIME	65
#define WR_WR_STROBE_1TIME	10000
#define WR_WR_STROBE_1TIME_MAX	11000

static inline unsigned int max_integral_multiple(unsigned int val, unsigned int base)
{
	unsigned int max_int;
	max_int = val / base;
	max_int += val % base ? 1 : 0;
	return max_int;
}
static uint32_t efuse_readl(uint32_t reg_off)
{
	return readl(EFUSE_BASE + reg_off);
}

static void efuse_writel(uint32_t val, uint32_t reg_off)
{
	writel(val, EFUSE_BASE + reg_off);
}

int adjust_efuse(int is_wirte)
{
	uint32_t efucfg_reg = 0;
	int adj = 0, strobe = 0;
	int h2clk = clk_get_rate(H2CLK);
	int ret = 0;
	if (is_wirte) {
		adj = ((((WR_ADJ_10TIME*(h2clk/1000000))+10-1)/10)+1000-1)/(1000);
		adj = (adj != 0) ? (adj - 1) : adj;
		strobe = ((WR_WR_STROBE_1TIME/1000)*(h2clk/1000000));
		strobe = strobe - 1666 - adj;
		if (strobe < -2047) {
			strobe = ((WR_WR_STROBE_1TIME_MAX/1000)*(h2clk/1000000));
			strobe = strobe - 1666 - adj;
			if (strobe < -2047) {
				error("h2clk is too slow");
				ret = -EFAULT;
				goto out;
			}
		}
		if (adj > 0xf || strobe > 2047) {
			error("h2clk is too fast");
			ret = -EFAULT;
			goto out;
		}

		if (strobe < 0)
			strobe = (0x800|(strobe&0x7ff));

		efucfg_reg = (adj<<12)|(strobe<<0);
		writel(efucfg_reg,(EFUSE_BASE + EFUCFG));
	} else {
		efucfg_reg = (0xf<<20)|(0xf<<16);
		writel(efucfg_reg,(EFUSE_BASE+EFUCFG));
		efucfg_reg = readl((EFUSE_BASE+EFUCFG));
		if (((efucfg_reg>>16)&0xf) != 0x7) {
			efucfg_reg = (0xf<<20)|(0x7<<16);
			writel(efucfg_reg,(EFUSE_BASE + EFUCFG));
		}
	}
out:
	printf("h2clk is %d, efucfg_reg 0x%x\n",h2clk,readl((EFUSE_BASE+EFUCFG)));
	return ret;
}

void boost_vddq(int gpio)
{
	int val;
	printf("boost vddq\n");
	gpio_direction_output(gpio , CONFIG_EFUSE_LEVEL);
	do {
		val = gpio_get_value(gpio);
		printf("gpio %d level %d\n",gpio,val);
	} while (val);
}

void reduce_vddq(int gpio)
{
	int val;
	printf("reduce vddq\n");
	gpio_direction_output(gpio, !CONFIG_EFUSE_LEVEL);
	do {
		val = gpio_get_value(gpio);
		printf("gpio %d level %d\n",gpio,val);
	} while (!val);
}

int efuse_read_sc_key(unsigned int offset)
{
	int start = offset + EFU_ROM_BASE;
	int ret = -EPERM;

	/*step 1 : Set config register*/
	ret = adjust_efuse(0);
	if (ret) goto out;

	/*Step 2 : Invoke SC-ROM controller to read corresponding map address*/
	switch (start) {
	case EFU_ROOT_KEY_BASE:
		break;
	case EFU_CHIP_KEY_BASE:
		break;
	case EFU_USER_KEY_BASE:
		break;
	case EFU_MD5_BASE:
		break;
	default :
		ret = -EPERM;
	}
out:
	return ret;
}

static int efuse_read_data(void *buf, int length, uint32_t start_addr)
{
	int i = 0;
	char *tmp_buf = NULL;
	int32_t *ptmp_buf = NULL;
	char * pbuf =buf;
	uint32_t data_length = length - 1;
	uint32_t word_num,val;
	unsigned int addr = start_addr - EFU_ROM_BASE;

	debug_cond(efuse_debug, "efuse read length %d from offset 0x%x\n",length, start_addr);

	if (efuse_gpio >= 0)
		reduce_vddq(efuse_gpio);

	word_num = max_integral_multiple(length, 4);

	tmp_buf = malloc(word_num * sizeof(int32_t));
	if (!tmp_buf)
		return -ENOMEM;
	memset(tmp_buf , 0 , word_num * sizeof(int32_t));
	ptmp_buf = (int32_t *)tmp_buf;

	/* clear read done staus */
	efuse_writel(0, EFUSTATE);

	val = addr << 21 | data_length << 16;
	efuse_writel(val, EFUCTRL);
	/* enable read */
	val = efuse_readl(EFUCTRL);
	val |= 1;
	efuse_writel(val, EFUCTRL);
		printf("ctl == %x\n",efuse_readl(EFUCTRL));
	/* wait read done status */
	while(!(efuse_readl(EFUSTATE) & 1));
		printf("state == %x\n",efuse_readl(EFUSTATE));
	for(i = 0; i < word_num; i ++) {
		val = efuse_readl(EFUDATA(i));
		debug_cond(efuse_debug,"EFUDATA[0x%x]:0x%08x\n",(EFUSE_BASE+EFUDATA(i)),
			   val);
		*(ptmp_buf + i) = val;
	}

	/* clear read done staus */
	efuse_writel(0, EFUSTATE);

	for (i = 0 ; i < length; i++) {
		pbuf[i] = tmp_buf[i];
		/* printf("0x%02x, 0x%02x\n", pbuf[i],tmp_buf[i]); */
	}
	if (efuse_debug) {
		int i = 0;
		printf("====read data infomation====\n");
		for (i = 0; i < word_num; i++) {
			printf("0x%03x:0x%08x\n", i, *((int32_t *)pbuf + i));
		}
		printf("============================\n");
	}

	free(tmp_buf);
	return word_num;
}

#define EFUSE_CHECK
#define EFUSE_W_TIMEOUT	(100*800)

/**
* @brief write data to efuse at offset, with data length.
*
* @param buf			stores hex value.
* @param start_addr
* @param length
*
* @return
*/
static int efuse_write_data(void *buf, int length, uint32_t start_addr)
{
	int i = 0, ret = 0;
	unsigned int * pbuf = buf;
	uint32_t data_length = length - 1;
	unsigned long long tmp_data;
	uint32_t word_num, val;
	unsigned int addr = start_addr - EFU_ROM_BASE;
	int timeout = EFUSE_W_TIMEOUT;		//vddq high is less than 1 sec

	printf("write data to start_addr: %x, length: %d\n", start_addr, length);

	if  (efuse_gpio < 0) {
		error("efuse gpio is not init");
		return -ENODEV;
	}

	word_num = max_integral_multiple(length, 4);

	for (i = 0; i < word_num; i++) {
		printf("0x%08x\n", pbuf[i]);
	}


	if(word_num > 8) {
		printf("strongly recommend operate each segment separately\n");
	} else {
		for(i = 0; i < word_num; i++) {
			val = pbuf[i];
			debug_cond(efuse_debug,"====write data to register====\n");
			debug_cond(efuse_debug,"%d(0x%x):0x%x\n",i,(EFUSE_BASE+EFUDATA(i)),val);
			printf("%d(0x%x):0x%08x\n",i,(EFUSE_BASE+EFUDATA(i)),val);
			efuse_writel(val, EFUDATA(i));
		}
	}



	/*
	 * set write Programming address and data length
	 */
	val = 0;
	val = addr  << 21 | data_length << 16 | 1 << 15;
	efuse_writel(val, EFUCTRL);
	/* Connect VDDQ pin from 2.5V */
	boost_vddq(efuse_gpio);
	/*
	 * Programming EFUSE enable
	 */
	val = efuse_readl(EFUCTRL);
	val |= 1 << 15;
	efuse_writel(val, EFUCTRL);
	/* enable write */
	val = efuse_readl(EFUCTRL);
	val |= 2;
	efuse_writel(val, EFUCTRL);
		printf("ctl == %x\n",efuse_readl(EFUCTRL));
	/* wait write done status */
	while(!(efuse_readl(EFUSTATE) & 0x2) &&  --timeout);

		printf("state == %x\n",efuse_readl(EFUSTATE));
	/* Disconnect VDDQ pin from 2.5V. */
	reduce_vddq(efuse_gpio);
	efuse_writel(0, EFUCTRL);
	if(!timeout) {
		error("write efuse failed");
		ret = -EFAULT;
		goto out;
	}

out:
	return ret;
}


int efuse_write_sc_key(unsigned int offset)
{
	int start = offset + EFU_ROM_BASE;
	int ret = -EPERM;
	unsigned tmp_reg = 0;

	/*step 1 : Set config register*/
	ret = adjust_efuse(1);
	if (ret) goto out;

	/*step 2: Write control register PG_EN bit to 1*/
	tmp_reg = (1<<15);
	writel(tmp_reg,(EFUSE_BASE+EFUCTRL));

	/*step 3: Connect VDDQ pin to 2.5V*/
	boost_vddq(efuse_gpio);
	switch (start) {
	case EFU_ROOT_KEY_BASE:
		break;
	case EFU_CHIP_KEY_BASE:
		break;
	case EFU_USER_KEY_BASE:
		break;
	case EFU_MD5_BASE:
		break;
	default :
		ret =  -EPERM;
	}

	/*Step 7: Disconnect VDDQ pin from 2.5V*/
	reduce_vddq(efuse_gpio);

	/*step 8:Write control register PG_EN bit to 0.*/
	writel(0,(EFUSE_BASE+EFUCTRL));
out:
	return ret;
}

int efuse_write(void *buf, int length, off_t offset)
{
	int start = (offset+EFU_ROM_BASE);
	int end = (offset + length + EFU_ROM_BASE - 1);
	int ret = -EPERM;

	char *pbuf = NULL;
	unsigned int start_read = start;
	unsigned int data_length;
	unsigned long long *data_buf = NULL;
	int word_num;
	unsigned int hex;
	int i;


	data_length = max_integral_multiple(length, 2);
	word_num = max_integral_multiple(data_length, 4);

	unsigned int *read_data = NULL;
	read_data = malloc(word_num * 4);
	if(read_data == NULL) {
		printf("error allocate read data!\n");
		return ret;
	}

	memset(read_data, 0, word_num * 4);
	ret = efuse_read_data(read_data, data_length, start_read);
	if (ret < 0)
		return ret;
	else
		ret = 0;

	printf("read_data = %x\n",*read_data);
	if (*read_data & 0xffffffff) {
		error("The position has been written data, write efuse failed!\n");
		return -EINVAL;
	}


	data_buf = malloc(word_num * 4);
	if(data_buf == NULL) {
		printf("error allocate data_buf!\n");
		return -EINVAL;
	}
	memset(data_buf, 0, word_num * 4);

	printf("length = %d\n", length);
	printf("offset = %x\n", (unsigned int)offset);

	printf("data_length = %d\n", data_length);

	pbuf = malloc(length);
	memset(pbuf, 0, length);
	memcpy(pbuf, buf, length);
	*data_buf = simple_strtoull(pbuf, NULL, 16);
	for(i = 0; i < word_num; i++) {
		printf("databuf[%d]: %x\n", i, data_buf[i]);
	}

	if (end > EFU_ROM_END)
		return -EINVAL;
	printf("offset %x length %x start %x end %x\n", offset, length ,start, end);
	if ((start < EFU_ROOT_KEY_BASE && end <= EFU_ROOT_KEY_BASE) ||
			(start > EFU_MD5_BASE && end <= EFU_MD5_END) ||
			(start > EFU_FIX_BT_BASE && end <= EFU_FIX_BT_END) ||
			(start > EFU_PROT_BIT_BASE && end <= EFU_PROT_BIT_END))
		ret = efuse_write_data(data_buf,data_length,start);
	else if (buf == NULL && length == 0) {
		ret = efuse_write_sc_key(offset);
	}

#ifdef EFUSE_CHECK
	memset(read_data, 0, word_num * 4);
	ret = efuse_read_data(read_data, data_length, start_read);
	if (ret < 0)
		return ret;
	else
		ret = 0;

	if (memcmp(read_data, data_buf, data_length)) {
		error(" compare error . write efuse failed");
		ret = -EFAULT;
	}

#endif
	free(read_data);
	free(data_buf);
	return ret;
}

int efuse_read(void *buf, int length, off_t offset)
{
	int start = (offset + EFU_ROM_BASE);
	int end = (offset + length + EFU_ROM_BASE - 1);
	int ret = -EPERM;

	if (end > EFU_ROM_END)
		return -EINVAL;

	printf("offset %x length %x start %x end %x\n", offset, length ,start, end);
	if ((start < EFU_ROOT_KEY_BASE && end <= EFU_ROOT_KEY_BASE) ||
			(start > EFU_MD5_BASE && end <= EFU_MD5_END) ||
			(start > EFU_FIX_BT_BASE && end <= EFU_FIX_BT_END) ||
			(start > EFU_PROT_BIT_BASE && end <= EFU_PROT_BIT_END))
		ret = efuse_read_data(buf,length,offset);
	else if (buf == NULL && length == 0) {
		ret = efuse_read_sc_key(offset);
	}

	return ret;
}

int efuse_read_id(void *buf, int length, int id)
{
	uint32_t *ptmp_buf = (uint32_t *)buf;
	int ret = -EPERM;
	int offset = 0;
	switch(id) {
		case EFUSE_R_CHIP_ID:
			offset = EFU_CHIP_ID_BASE;
			break;
		case EFUSE_R_USER_ID:
			offset = EFU_COMS_ID_BASE;
			break;
		default:
			printf("Unkown id !\n");
			return -EPERM;
			break;
	}

	offset = offset - EFU_ROM_BASE;

	ret = efuse_read(buf, length, offset);
	if(ret < 0) {
		printf("efuse_read_id: read id error\n");
		return ret;
	}

	return ret;
}
int efuse_init(int gpio_pin)
{
	if (gpio_pin >= 0) {
		if (efuse_gpio >= 0) gpio_free(efuse_gpio);
		efuse_gpio = gpio_request(gpio_pin, "VDDQ");
		if (efuse_gpio < 0) return efuse_gpio;
	} else {
		efuse_gpio = -1;
	}
	return 0;
}

void efuse_deinit(void)
{
	if (efuse_gpio >= 0)
		gpio_free(efuse_gpio);
	efuse_gpio = -1;
	return;
}

void efuse_debug_enable(int enable)
{
	efuse_debug = !!enable;
	return;
}
