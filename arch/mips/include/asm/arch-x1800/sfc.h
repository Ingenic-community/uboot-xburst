#ifndef __SFC_H
#define __SFC_H

#include <linux/mtd/mtd.h>
#include <asm/arch/sfc_register.h>
#ifndef CONFIG_SPL_BUILD
#include <linux/list.h>
#endif

struct cmd_info {
	uint8_t cmd;
	uint8_t dataen;
#ifndef CONFIG_SPL_BUILD
	uint8_t pollen;
	uint8_t sta_exp;
	uint8_t sta_msk;
#endif
};

struct sfc_transfer {

	struct cmd_info cmd_info;

	uint8_t addr_len;
	uint8_t direction;
	uint8_t data_dummy_bits;/*addr + data_dummy_bits + data*/
	uint32_t addr;
	uint32_t addr_plus;

	uint8_t sfc_mode;
	uint8_t ops_mode;
	uint8_t phase_format;/*we just use default value;phase1:cmd+dummy+addr... phase0:cmd+addr+dummy...*/
	uint8_t *data;
	uint32_t len;
	uint32_t cur_len;

#ifndef CONFIG_SPL_BUILD
	struct list_head list;
#endif
};

struct sfc{

#ifndef CONFIG_SPL_BUILD
	unsigned long long src_clk;
#endif
	uint32_t threshold;

	struct sfc_transfer *transfer;
};

#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_SFC_NAND)
#include <asm/arch/spinand.h>

typedef union sfc_tranconf_r {
	/** raw register data */
	unsigned int d32;
	/** register bits */
	struct {
		unsigned cmd:16;
		unsigned data_en:1;
		unsigned dmy_bits:6;
		unsigned phase_format:1;
		unsigned cmd_en:1;
		unsigned poll_en:1;
		unsigned addr_width:3;
		unsigned tran_mode:3;
	} reg;
} sfc_tranconf_r;

struct jz_sfc {
	sfc_tranconf_r tranconf;
    unsigned int  addr;
    unsigned int  len;
    unsigned int  addr_plus;
};

#ifdef CONFIG_SFC_DEBUG
#define sfc_debug(fmt, args...)         \
    do {                    \
        printf(fmt, ##args);        \
    } while (0)
#else
#define sfc_debug(fmt, args...)         \
    do {                    \
    } while (0)
#endif

#define  SFC_SEND_COMMAND(sfc, a, b, c, d, e, f, g)   do{						\
        ((struct jz_sfc *)sfc)->tranconf.d32 = 0;							\
        ((struct jz_sfc *)sfc)->tranconf.reg.cmd_en = 1;						\
		((struct jz_sfc *)sfc)->tranconf.reg.cmd = a;						\
        ((struct jz_sfc *)sfc)->len = b;								\
        ((struct jz_sfc *)sfc)->addr = c;								\
        ((struct jz_sfc *)sfc)->tranconf.reg.addr_width = d;						\
        ((struct jz_sfc *)sfc)->addr_plus = 0;								\
        ((struct jz_sfc *)sfc)->tranconf.reg.dmy_bits = e;						\
        ((struct jz_sfc *)sfc)->tranconf.reg.data_en = f;						\
		if(a == SPINAND_CMD_RDCH_X4) {								\
			((struct jz_sfc *)sfc)->tranconf.reg.tran_mode = TRAN_SPI_QUAD;			\
		} else {										\
			((struct jz_sfc *)sfc)->tranconf.reg.tran_mode = TRAN_SPI_STANDARD;		\
		}											\
        sfc_send_cmd(sfc, g);										\
	} while(0)



#endif


#endif
