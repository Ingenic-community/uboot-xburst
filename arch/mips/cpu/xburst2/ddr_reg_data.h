#ifndef DDR_REG_DATA_H
#define DDR_REG_DATA_H
struct ddr_registers
{
	uint32_t ddrc_cfg;
	uint32_t ddrc_ctrl;
	uint32_t ddrc_dlmr;
	uint32_t ddrc_ddlp;
	uint32_t ddrc_mmap[2];
	uint32_t ddrc_refcnt;
	uint32_t ddrc_timing1;
	uint32_t ddrc_timing2;
	uint32_t ddrc_timing3;
	uint32_t ddrc_timing4;
	uint32_t ddrc_timing5;
	uint32_t ddrc_autosr_cnt;
	uint32_t ddrc_autosr;
	uint32_t ddrc_hregpro;
	uint32_t ddrc_pregpro;
	uint32_t ddrc_cguc0;
	uint32_t ddrc_cguc1;
	uint32_t ddrp_memcfg;
	uint32_t ddrp_cl;
	uint32_t ddrp_cwl;
	uint32_t ddr_mr0;
	uint32_t ddr_mr1;
	uint32_t ddr_mr2;
	uint32_t ddr_mr3;
	uint32_t ddr_mr10;
	uint32_t ddr_mr63;
	uint32_t ddr_chip0_size;
	uint32_t ddr_chip1_size;
	unsigned int remap_array[5];
};
extern struct ddr_registers *g_ddr_param;
#define DDRC_CFG_VALUE                  g_ddr_param->ddrc_cfg
#define DDRC_CTRL_VALUE                 g_ddr_param->ddrc_ctrl
#define DDRC_DLMR_VALUE                 g_ddr_param->ddrc_dlmr
#define DDRC_DDLP_VALUE                 g_ddr_param->ddrc_ddlp
#define DDRC_MMAP0_VALUE                g_ddr_param->ddrc_mmap[0]
#define DDRC_MMAP1_VALUE                g_ddr_param->ddrc_mmap[1]
#define DDRC_REFCNT_VALUE               g_ddr_param->ddrc_refcnt
#define DDRC_TIMING1_VALUE              g_ddr_param->ddrc_timing1
#define DDRC_TIMING2_VALUE              g_ddr_param->ddrc_timing2
#define DDRC_TIMING3_VALUE              g_ddr_param->ddrc_timing3
#define DDRC_TIMING4_VALUE              g_ddr_param->ddrc_timing4
#define DDRC_TIMING5_VALUE              g_ddr_param->ddrc_timing5
#define DDRC_AUTOSR_CNT_VALUE           g_ddr_param->ddrc_autosr_cnt
#define DDRC_AUTOSR_EN_VALUE            g_ddr_param->ddrc_autosr
#define DDRC_HREGPRO_VALUE              g_ddr_param->ddrc_hregpro
#define DDRC_PREGPRO_VALUE              g_ddr_param->ddrc_pregpro
#define DDRC_CGUC0_VALUE                g_ddr_param->ddrc_cguc0
#define DDRC_CGUC1_VALUE                g_ddr_param->ddrc_cguc1
#define DDRP_MEMCFG_VALUE               g_ddr_param->ddrp_memcfg
#define DDRP_CL_VALUE                   g_ddr_param->ddrp_cl
#define DDRP_CWL_VALUE                  g_ddr_param->ddrp_cwl
#define DDR_MR0_VALUE                   g_ddr_param->ddr_mr0
#define DDR_MR1_VALUE                   g_ddr_param->ddr_mr1
#define DDR_MR2_VALUE                   g_ddr_param->ddr_mr2
#define DDR_MR3_VALUE                   g_ddr_param->ddr_mr3
#define DDR_MR10_VALUE                  g_ddr_param->ddr_mr10
#define DDR_MR63_VALUE                  g_ddr_param->ddr_mr63
#define DDR_CHIP_0_SIZE                 g_ddr_param->ddr_chip0_size
#define DDR_CHIP_1_SIZE                 g_ddr_param->ddr_chip1_size
#define REMMAP_ARRAY                    g_ddr_param->remap_array



#define timing1_tWL             ((DDRC_TIMING1_VALUE & 0x0000003f) >> 0 )
#define timing1_tWR             ((DDRC_TIMING1_VALUE & 0x00003f00) >> 8 )
#define timing1_tWTR            ((DDRC_TIMING1_VALUE & 0x003f0000) >> 16)
#define timing1_tWDLAT          ((DDRC_TIMING1_VALUE & 0x3f000000) >> 24)
#define timing2_tRL             ((DDRC_TIMING2_VALUE & 0x0000003f) >> 0 )
#define timing2_tRTP            ((DDRC_TIMING2_VALUE & 0x00003f00) >> 8 )
#define timing2_tRTW            ((DDRC_TIMING2_VALUE & 0x003f0000) >> 16)
#define timing2_tRDLAT          ((DDRC_TIMING2_VALUE & 0x3f000000) >> 24)
#define timing3_tRP             ((DDRC_TIMING3_VALUE & 0x0000003f) >> 0 )
#define timing3_tCCD            ((DDRC_TIMING3_VALUE & 0x00003f00) >> 8 )
#define timing3_tRCD            ((DDRC_TIMING3_VALUE & 0x003f0000) >> 16)
#define timing3_ttEXTRW         ((DDRC_TIMING3_VALUE & 0x07000000) >> 24)
#define timing4_tRRD            ((DDRC_TIMING4_VALUE & 0x0000003f) >> 0 )
#define timing4_tRAS            ((DDRC_TIMING4_VALUE & 0x00003f00) >> 8 )
#define timing4_tRC             ((DDRC_TIMING4_VALUE & 0x003f0000) >> 16)
#define timing4_tFAW            ((DDRC_TIMING4_VALUE & 0xff000000) >> 24)
#define timing5_tCKE            ((DDRC_TIMING5_VALUE & 0x00000007) >> 0 )
#define timing5_tXP             ((DDRC_TIMING5_VALUE & 0x000000f0) >> 4 )
#define timing5_tCKSRE          ((DDRC_TIMING5_VALUE & 0x0000f000) >> 12)
#define timing5_tCKESR          ((DDRC_TIMING5_VALUE & 0x00ff0000) >> 16)
#define timing5_tXS             ((DDRC_TIMING5_VALUE & 0xff000000) >> 24)


static unsigned int get_refcnt_value(int div)
{
	switch(div) {
		case 0: return 0xe480000;
		case 1: return 0x7230000;
		case 2: return 0x5170000;
		defalut :
			printf("not support \n");
	}
}


#endif /* DDR_REG_DATA_H */
