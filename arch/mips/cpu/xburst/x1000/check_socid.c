#include <config.h>
#include <common.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;
#define REG32(addr) *(volatile unsigned int *)(addr)

#define EFUSE_CHIPID_ADDR	    0x00
#define EFUSE_SOCID_ADDR	    0x3c

#define EFUSE_CTRL	    0xb3540000
#define EFUSE_CFG	    0xb3540004
#define EFUSE_STATE	    0xb3540008
#define EFUSE_DATA0	    0xb354000C
#define EFUSE_DATA1	    0xb3540010
#define EFUSE_DATA3	    0xb3540018

#define DDR_TIMING4	    0x07230f31
#define DDR_64M_CFG	    0x0a468a40
#define DDR_64M_MMAP0	    0x000020fc
#define DDR_64M_MMAP1	    0x00002400
#define DDR_64M_REMMAP0	    0x03020d0c
#define DDR_64M_REMMAP2	    0x0b0a0908
#define DDR_64M_REMMAP3	    0x0f0e0100

enum ddr_change_param {
	REMMAP0,
	REMMAP1,
	REMMAP2,
	REMMAP3,
};

static enum socid {
	X1000 = 0xff00,
	X1000E = 0xff01,
	X1500 = 0xff02,
	X1500L_NEW = 0xff04,
	X1000_NEW = 0xff08,
	X1000E_NEW = 0xff09,
	X1500_NEW = 0xff0a,
};

static void read_efuse_segment(unsigned int addr, unsigned int length, unsigned int *buf)
{
	unsigned int val;

	/* clear read done staus */
	REG32(EFUSE_STATE) = 0;
	val = addr << 21 | length << 16 | 1;
	REG32(EFUSE_CTRL) = val;
	/* wait read done status */
	while(!(REG32(EFUSE_STATE) & 1))
		;
	if(addr == EFUSE_SOCID_ADDR) {
		buf[0] = REG32(EFUSE_DATA0) & 0xffff;
	} else if(addr == EFUSE_CHIPID_ADDR) {
		buf[0] = REG32(EFUSE_DATA1);
		buf[1] = REG32(EFUSE_DATA3);
	}
	/* clear read done staus */
	REG32(EFUSE_STATE) = 0;
}

static inline int check_chipid(unsigned int *data)
{
	unsigned int lotid_l, lotid_h;
	unsigned int waferid;
#define LOTID_LOW 0x07
#define LOTID_LOW_MASK 0x1F
#define LOTID_HIGH 0x0E90E02F
#define LOTID_HIGH_MASK 0x3FFFFFFF
#define WAFERID_BIT_OFF 11
#define WAFERID_MASK 0x1f

	lotid_l = data[0] & LOTID_LOW_MASK;
	lotid_h = data[1] & LOTID_HIGH_MASK;
	waferid = (data[0] >> WAFERID_BIT_OFF) & WAFERID_MASK;

	/* printf("lotid_l = %x, lotid_h = %x\n", lotid_l, lotid_h); */
	/* printf("waferid = %x\n", waferid); */
	if(lotid_l == LOTID_LOW && lotid_h == LOTID_HIGH)
		if(waferid >=16 && waferid <= 25)
			return 0;
	return -1;

}
static int read_and_check_chipid()
{
	unsigned int val, data[2];

	read_efuse_segment(EFUSE_CHIPID_ADDR, 15, data);
	return check_chipid(data);
}

static void read_socid(unsigned int *id)
{
	read_efuse_segment(EFUSE_SOCID_ADDR, 1, id);
}

static void ddr_change_64M()
{
	uint32_t *remmap = gd->arch.gi->ddr_change_param.ddr_remap_array;
	gd->arch.gi->ddr_change_param.ddr_cfg = DDR_64M_CFG;
	gd->arch.gi->ddr_change_param.ddr_mmap0 = DDR_64M_MMAP0;
	gd->arch.gi->ddr_change_param.ddr_mmap1 = DDR_64M_MMAP1;
	/*remmap*/
	remmap[REMMAP0] = DDR_64M_REMMAP0;
	remmap[REMMAP2] = DDR_64M_REMMAP2;
	remmap[REMMAP3] = DDR_64M_REMMAP3;

}

int check_socid()
{
	unsigned int socid;

	read_socid(&socid);
	switch(socid) {
	case X1000_NEW:
	case X1500_NEW:
	case X1500L_NEW:
		gd->arch.gi->ddr_change_param.ddr_autosr = 1;
		break;
	case X1000E:
	case X1000E_NEW:
		ddr_change_64M();
		gd->arch.gi->ddr_change_param.ddr_autosr = 1;
		break;
	case X1500:
		if(!read_and_check_chipid()) {
			gd->arch.gi->ddr_change_param.ddr_autosr = 1;
			return 0;
		}
	case X1000:
	case 0:
		gd->arch.gi->ddr_change_param.ddr_timing4 = DDR_TIMING4;
		break;
	default:
		return -1;
	}

	return socid;
}
