#include <config.h>
#include <common.h>
#include <ddr/ddr_common.h>
#include <generated/ddr_reg_values.h>


unsigned int DDRC_CFG_VALUE;
unsigned int DDRC_CTRL_VALUE;
unsigned int DDRC_DLMR_VALUE;
unsigned int DDRC_DDLP_VALUE;
unsigned int DDRC_MMAP0_VALUE;
unsigned int DDRC_MMAP1_VALUE;
unsigned int DDRC_REFCNT_VALUE;
unsigned int DDRC_TIMING1_VALUE;
unsigned int DDRC_TIMING2_VALUE;
unsigned int DDRC_TIMING3_VALUE;
unsigned int DDRC_TIMING4_VALUE;
unsigned int DDRC_TIMING5_VALUE;
unsigned int DDRC_TIMING6_VALUE;
unsigned int DDRC_AUTOSR_EN_VALUE;
#ifndef CONFIG_X1XXX_INNOPHY
unsigned int DDRP_DCR_VALUE;
unsigned int DDRP_MR0_VALUE;
unsigned int DDRP_MR1_VALUE;
unsigned int DDRP_MR2_VALUE;
unsigned int DDRP_MR3_VALUE;
unsigned int DDRP_PTR0_VALUE;
unsigned int DDRP_PTR1_VALUE;
unsigned int DDRP_PTR2_VALUE;
unsigned int DDRP_DTPR0_VALUE;
unsigned int DDRP_DTPR1_VALUE;
unsigned int DDRP_DTPR2_VALUE;
unsigned int DDRP_PGCR_VALUE;
unsigned int DDRP_ODTCR_VALUE;
unsigned int DDRP_DX0GCR_VALUE;
unsigned int DDRP_DX1GCR_VALUE;
unsigned int DDRP_DX2GCR_VALUE;
unsigned int DDRP_DX3GCR_VALUE;
unsigned int DDRP_ZQNCR1_VALUE;
unsigned int DDRP_IMPANDCE_ARRAY[2];
unsigned int DDRP_ODT_IMPANDCE_ARRAY[2];
unsigned char DDRP_RZQ_TABLE[32];
#else
unsigned int DDRP_MEMCFG_VALUE;
unsigned int DDRP_CL_VALUE;
unsigned int DDRP_CWL_VALUE;

unsigned int DDR_MR0_VALUE;
unsigned int DDR_MR1_VALUE;
unsigned int DDR_MR2_VALUE;
unsigned int DDR_MR3_VALUE;
unsigned int DDR_MR10_VALUE;
unsigned int DDR_MR63_VALUE;
#endif

unsigned int DDR_CHIP_0_SIZE;
unsigned int DDR_CHIP_1_SIZE;
unsigned int REMMAP_ARRAY[5];


#define _DDRC_CFG_VALUE(type) 	DDRC_CFG_VALUE_##type
#define _DDRC_CTRL_VALUE(type)	DDRC_CTRL_VALUE_##type
#define _DDRC_MMAP0_VALUE(type)	DDRC_MMAP0_VALUE_##type
#define _DDRC_MMAP1_VALUE(type)	DDRC_MMAP1_VALUE_##type
#define _DDRC_DLMR_VALUE(type)	DDRC_DLMR_VALUE_##type
#define _DDRC_DDLP_VALUE(type)	DDRC_DDLP_VALUE_##type
#define _DDRC_REFCNT_VALUE(type)	DDRC_REFCNT_VALUE_##type
#define _DDRC_TIMING1_VALUE(type)	DDRC_TIMING1_VALUE_##type
#define _DDRC_TIMING2_VALUE(type)	DDRC_TIMING2_VALUE_##type
#define _DDRC_TIMING3_VALUE(type)	DDRC_TIMING3_VALUE_##type
#define _DDRC_TIMING4_VALUE(type)	DDRC_TIMING4_VALUE_##type
#define _DDRC_TIMING5_VALUE(type)	DDRC_TIMING5_VALUE_##type
#define _DDRC_TIMING6_VALUE(type)	DDRC_TIMING6_VALUE_##type
#define _DDRC_AUTOSR_EN_VALUE(type)	DDRC_AUTOSR_EN_VALUE_##type

#ifndef CONFIG_X1XXX_INNOPHY

#define _DDRP_DCR_VALUE(type)		DDRP_DCR_VALUE_##type
#define _DDRP_MR0_VALUE(type)		DDRP_MR0_VALUE_##type
#define _DDRP_MR1_VALUE(type)		DDRP_MR1_VALUE_##type
#define _DDRP_MR2_VALUE(type)		DDRP_MR2_VALUE_##type
#define _DDRP_MR3_VALUE(type)	DDRP_MR3_VALUE_##type
#define _DDRP_PTR0_VALUE(type)	DDRP_PTR0_VALUE_##type
#define _DDRP_PTR1_VALUE(type)	DDRP_PTR1_VALUE_##type
#define _DDRP_PTR2_VALUE(type)	DDRP_PTR2_VALUE_##type
#define _DDRP_DTPR0_VALUE(type)	DDRP_DTPR0_VALUE_##type
#define _DDRP_DTPR1_VALUE(type)	DDRP_DTPR1_VALUE_##type
#define _DDRP_DTPR2_VALUE(type)	DDRP_DTPR2_VALUE_##type
#define _DDRP_PGCR_VALUE(type)	DDRP_PGCR_VALUE_##type
#define _DDRP_ODTCR_VALUE(type)	DDRP_ODTCR_VALUE_##type
#define _DDRP_DX0GCR_VALUE(type)	DDRP_DX0GCR_VALUE_##type
#define _DDRP_DX1GCR_VALUE(type)	DDRP_DX1GCR_VALUE_##type
#define _DDRP_DX2GCR_VALUE(type)	DDRP_DX2GCR_VALUE_##type
#define _DDRP_DX3GCR_VALUE(type)	DDRP_DX3GCR_VALUE_##type
#define _DDRP_ZQNCR1_VALUE(type)	DDRP_ZQNCR1_VALUE_##type
#define _DDRP_IMPANDCE_ARRAY(type)	DDRP_IMPANDCE_ARRAY_##type
#define _DDRP_ODT_IMPANDCE_ARRAY(type) DDRP_ODT_IMPANDCE_ARRAY_##type
#define _DDRP_RZQ_TABLE(type)		DDRP_RZQ_TABLE_##type

#else

#define _DDRP_MEMCFG_VALUE(type) DDRP_MEMCFG_VALUE_##type
#define _DDRP_CL_VALUE(type)	DDRP_CL_VALUE_##type
#define _DDRP_CWL_VALUE(type)	DDRP_CWL_VALUE_##type

#define _DDR_MR0_VALUE(type)	DDR_MR0_VALUE_##type
#define _DDR_MR1_VALUE(type)	DDR_MR1_VALUE_##type
#define _DDR_MR2_VALUE(type)	DDR_MR2_VALUE_##type
#define _DDR_MR3_VALUE(type)	DDR_MR3_VALUE_##type
#define _DDR_MR10_VALUE(type)	DDR_MR10_VALUE_##type
#define _DDR_MR63_VALUE(type)	DDR_MR63_VALUE_##type


#endif

#define _DDR_CHIP_0_SIZE(type)	DDR_CHIP_0_SIZE_##type
#define _DDR_CHIP_1_SIZE(type)	DDR_CHIP_1_SIZE_##type
#define _REMMAP_ARRAY(type)	REMMAP_ARRAY_##type


#define inno_get_ddr_params(type)				\
	do {							\
		DDRC_CFG_VALUE = _DDRC_CFG_VALUE(type);		\
		DDRC_CTRL_VALUE = _DDRC_CTRL_VALUE(type);		\
		DDRC_DLMR_VALUE = _DDRC_DLMR_VALUE(type);		\
		DDRC_DDLP_VALUE = _DDRC_DDLP_VALUE(type);		\
		DDRC_MMAP0_VALUE = _DDRC_MMAP0_VALUE(type);		\
		DDRC_MMAP1_VALUE = _DDRC_MMAP1_VALUE(type);		\
		DDRC_REFCNT_VALUE = _DDRC_REFCNT_VALUE(type);		\
		DDRC_TIMING1_VALUE = _DDRC_TIMING1_VALUE(type);		\
		DDRC_TIMING2_VALUE = _DDRC_TIMING2_VALUE(type);		\
		DDRC_TIMING3_VALUE = _DDRC_TIMING3_VALUE(type);		\
		DDRC_TIMING4_VALUE = _DDRC_TIMING4_VALUE(type);		\
		DDRC_TIMING5_VALUE = _DDRC_TIMING5_VALUE(type);		\
		DDRC_TIMING6_VALUE = _DDRC_TIMING6_VALUE(type);		\
		DDRC_AUTOSR_EN_VALUE = _DDRC_AUTOSR_EN_VALUE(type);	\
		DDRP_MEMCFG_VALUE = _DDRP_MEMCFG_VALUE(type);		\
		DDRP_CL_VALUE = _DDRP_CL_VALUE(type);			\
		DDRP_CWL_VALUE = _DDRP_CWL_VALUE(type);			\
		DDR_MR0_VALUE = _DDR_MR0_VALUE(type);			\
		DDR_MR1_VALUE = _DDR_MR1_VALUE(type);			\
		DDR_MR2_VALUE = _DDR_MR2_VALUE(type);			\
		DDR_MR3_VALUE = _DDR_MR3_VALUE(type);			\
		DDR_MR10_VALUE = _DDR_MR10_VALUE(type);			\
		DDR_MR63_VALUE = _DDR_MR63_VALUE(type);			\
		DDR_CHIP_0_SIZE = _DDR_CHIP_0_SIZE(type);		\
		DDR_CHIP_1_SIZE = _DDR_CHIP_1_SIZE(type);		\
	} while(0)
#if 0
	for(i = 0; i < 5; i++) {
		REMMAP_ARRAY[i] = _REMMAP_ARRAY(type)[i];
	}
#endif

#define dwc_get_ddr_params(type)				\
	DDRC_CFG_VALUE = _DDR_CFG_VALUE(type);		\
	DDRC_CTRL_VALUE = _DDRC_CTRL_VALUE(type);		\
	DDRC_MMAP0_VALUE = _DDRC_MMAP0_VALUE(type);		\
	DDRC_MMAP1_VALUE = _DDRC_MMAP1_VALUE(type);		\
	DDRC_REFCNT_VALUE = _DDRC_REFCNT_VALUE(type);		\
	DDRC_TIMING1_VALUE = _DDRC_TIMING1_VALUE(type);		\
	DDRC_TIMING2_VALUE = _DDRC_TIMING2_VALUE(type);		\
	DDRC_TIMING3_VALUE = _DDRC_TIMING3_VALUE(type);		\
	DDRC_TIMING4_VALUE = _DDRC_TIMING4_VALUE(type);		\
	DDRC_TIMING5_VALUE = _DDRC_TIMING5_VALUE(type);		\
	DDRC_TIMING6_VALUE = _DDRC_TIMING6_VALUE(type);		\
	DDRC_AUTOSR_EN_VALUE = _DDRC_AUTOSR_EN_VALUE(type);	\
	DDRP_DCR_VALUE = _DDRP_DCR_VALUE(type);			\
	DDRP_MR0_VALUE = _DDRP_MR0_VALUE(type);			\
	DDRP_MR1_VALUE = _DDRP_MR1_VALUE(type);			\
	DDRP_MR3_VALUE = _DDRP_MR3_VALUE(type);			\
	DDRP_PTR0_VALUE = _DDRP_PTR0_VALUE(type);		\
	DDRP_PTR1_VALUE = _DDRP_PTR1_VALUE(type);		\
	DDRP_PTR2_VALUE = _DDRP_PTR2_VALUE(type);		\
	DDRP_PGCR_VALUE = _DDRP_PGCR_VALUE(type);		\
	DDRP_ODTCR_VALUE = _DDRP_ODTCR_VALUE(type);		\
	DDRP_DX0GCR_VALUE = _DDRP_DX0GCR_VALUE(type);		\
	DDRP_DX1GCR_VALUE = _DDRP_DX1GCR_VALUE(type);		\
	DDRP_DX2GCR_VALUE = _DDRP_DX2GCR_VALUE(type);		\
	DDRP_DX3GCR_VALUE = _DDRP_DX3GCR_VALUE(type);		\
	DDRP_ZQNCR1_VALUE = _DDRP_ZQNCR1_VALUE(type);		\
	DDRP_IMPANDCE_ARRAY[0] = _DDRP_IMPANDCE_ARRAY(type)[0];	\
	DDRP_IMPANDCE_ARRAY[1] = _DDRP_IMPANDCE_ARRAY(type)[1];	\
	DDRP_ODT_IMPANDCE_ARRAY[0] = _DDRP_ODT_IMPANDCE_ARRAY(type)[0]; \
	DDRP_ODT_IMPANDCE_ARRAY[1] = _DDRP_ODT_IMPANDCE_ARRAY(type)[1]; \
	for(i = 0; i < sizeof(ddrp->rzq_table); i++) {			\
		DDRP_RZQ_TABLE[i] = _DDRP_RZQ_TABLE(type)[i];		\
	}								\
	DDR_CHIP_0_SIZE = _DDR_CHIP_0_SIZE(type);			\
	DDR_CHIP_1_SIZE = _DDR_CHIP_1_SIZE(type);			\

#if 0
	for(i = 0; i < 5; i++) {
		REMMAP_ARRAY = _REMMAP_ARRAY(type)[i];
	}
#endif


void get_ddr_params(int type)
{

#ifndef CONFIG_X1XXX_INNOPHY

#error "unsupported DWC phy"

#else

	printf("type ---- %d\n", type);
	switch(type) {
		case DDR3:
#ifdef CONFIG_DDR_TYPE_DDR3
			inno_get_ddr_params(0);
#endif
			break;
		case LPDDR:
#ifdef CONFIG_DDR_TYPE_LPDDR3
			inno_get_ddr_params(1);
#endif
			break;
		case LPDDR2:
#ifdef CONFIG_DDR_TYPE_LPDDR2
			inno_get_ddr_params(2);
#endif
			break;
		case LPDDR3:
#ifdef CONFIG_DDR_TYPE_LPDDR3
			inno_get_ddr_params(3);
#endif
			break;
		case DDR2:
#ifdef CONFIG_DDR_TYPE_DDR2
			inno_get_ddr_params(4);
#endif
			break;
		default:
			debug("errot ddr type %d\n", type);
			break;
	}
#endif

#if 0
	/* dump Params... */

printf("DDRC_CFG_VALUE;		%x\n", DDRC_CFG_VALUE);
printf("DDRC_CTRL_VALUE;        %x\n", DDRC_CTRL_VALUE);
printf("DDRC_DLMR_VALUE;        %x\n", DDRC_DLMR_VALUE);
printf("DDRC_DDLP_VALUE;        %x\n", DDRC_DDLP_VALUE);
printf("DDRC_MMAP0_VALUE;       %x\n", DDRC_MMAP0_VALUE);
printf("DDRC_MMAP1_VALUE;       %x\n", DDRC_MMAP1_VALUE);
printf("DDRC_REFCNT_VALUE;      %x\n", DDRC_REFCNT_VALUE);
printf("DDRC_TIMING1_VALUE;     %x\n", DDRC_TIMING1_VALUE);
printf("DDRC_TIMING2_VALUE;     %x\n", DDRC_TIMING2_VALUE);
printf("DDRC_TIMING3_VALUE;     %x\n", DDRC_TIMING3_VALUE);
printf("DDRC_TIMING4_VALUE;     %x\n", DDRC_TIMING4_VALUE);
printf("DDRC_TIMING5_VALUE;     %x\n", DDRC_TIMING5_VALUE);
printf("DDRC_TIMING6_VALUE;     %x\n", DDRC_TIMING6_VALUE);
printf("DDRC_AUTOSR_EN_VALUE;   %x\n", DDRC_AUTOSR_EN_VALUE);
printf("DDRP_MEMCFG_VALUE;      %x\n", DDRP_MEMCFG_VALUE);
printf("DDRP_CL_VALUE;          %x\n", DDRP_CL_VALUE);
printf("DDRP_CWL_VALUE;         %x\n", DDRP_CWL_VALUE);
printf("DDR_MR0_VALUE;          %x\n", DDR_MR0_VALUE);
printf("DDR_MR1_VALUE;          %x\n", DDR_MR1_VALUE);
printf("DDR_MR2_VALUE;          %x\n", DDR_MR2_VALUE);
printf("DDR_MR3_VALUE;          %x\n", DDR_MR3_VALUE);
printf("DDR_MR10_VALUE;         %x\n", DDR_MR10_VALUE);
printf("DDR_MR63_VALUE;         %x\n", DDR_MR63_VALUE);
#endif

}






