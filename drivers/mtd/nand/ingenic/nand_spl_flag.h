#ifndef __JZNAND_SPL_FLAG_H__
#define __JZNAND_SPL_FLAG_H__


#define SPL_SIZE    (16*1024)
#define NAND_PARAMS_LEN     256
#define SPL_BACKUP_NUM      8

#define SPL_ECC_SIZE  256
#define SPL_ECC_BYTE  112
#define SPL_BCH_BIT   64

#if defined(CONFIG_JZ4775) || defined(CONFIG_M150) || defined(CONFIG_JZ4780)
#define BUSWIDTH_FLAG_OFFSET    0               /* [0 : 63] */
#define NANDTYPE_FLAG_OFFSET    (BUSWIDTH_FLAG_OFFSET + 64) /* [64 : 127] */
#define ROWCYCLE_FLAG_OFFSET    (NANDTYPE_FLAG_OFFSET + 64) /* [128 : 159] */
#define PAGESIZE_FLAG2_OFFSET   (ROWCYCLE_FLAG_OFFSET + 32) /* [160 : 191] */
#define PAGESIZE_FLAG1_OFFSET   (ROWCYCLE_FLAG_OFFSET + 32 * 2) /* [192 : 223] */
#define PAGESIZE_FLAG0_OFFSET   (ROWCYCLE_FLAG_OFFSET + 32 * 3) /* [224 : 255] */
#define FLAG_BUSWIDTH_8BIT  0x55
#define FLAG_BUSWIDTH_16BIT 0xaa
#define FLAG_NANDTYPE_COMMON    0x55
#define FLAG_NANDTYPE_TOGGLE    0xaa
#define FLAG_ROWCYCLE_2     0x55
#define FLAG_ROWCYCLE_3     0xaa
#define FLAG_PAGESIZE_512   0x555555
#define FLAG_PAGESIZE_2K    0x55aa55
#define FLAG_PAGESIZE_4K    0xaa5555
#define FLAG_PAGESIZE_8K    0xaaaa55
#define FLAG_PAGESIZE_16K   0xaaaaaa
#else
#define BUSWIDTH_FLAG_OFFSET    0			    /* [0 : 63] */
#define NANDTYPE_FLAG_OFFSET    (BUSWIDTH_FLAG_OFFSET + 64) /* [64 : 127] */
#define NANDTYPE_OTHER_FLAGS	(NANDTYPE_FLAG_OFFSET + 64) /* [128: 255] */
#define NANDTYPE_ROWCYCLE	(NANDTYPE_OTHER_FLAGS + 4)
#define NANDTYPE_PAGESIZE	(NANDTYPE_OTHER_FLAGS + 8)
#define FLAG_BUSWIDTH_8BIT  0x55
#define FLAG_BUSWIDTH_16BIT 0xaa
#define FLAG_NANDTYPE_COMMON    0x55
#define FLAG_NANDTYPE_TOGGLE    0xaa
#endif

struct spl_basic_param{
	unsigned char buswidth;
	unsigned char nandtype;
	unsigned char rowcycles;
	unsigned int pagesize;
};
#endif //__JZNAND_SPL_FLAG_H__
