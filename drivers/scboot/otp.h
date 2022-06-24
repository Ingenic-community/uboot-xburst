#ifndef __OTP_H__
#define __OTP_H__

#define SC_OTP_SEL_RKEY 0x1
#define SC_OTP_SEL_CKEY 0x2
#define SC_OTP_SEL_UKEY 0x4
#define SC_OTP_SEL_NKU 0x8
#define SC_OTP_RW  (0x1 << 5) //0:R  1:W
#define SC_OTP_RD  0

#define SC_OTP_RKEY   EFUSE_OTP_BASE
#define SC_OTP_CKEY   (EFUSE_OTP_RKEY + 16)
#define SC_OTP_UKEY   (SC_OTP_CKEY + 16)
#define SC_OTP_NKU    (SC_OTP_UKEY + 16)

#define EFUSE_SECURE_RKEY_ADDR 0xb3540240
#define EFUSE_SECURE_CKEY_ADDR 0xb3540250
#define EFUSE_SECURE_UKEY_ADDR 0xb3540260
#define EFUSE_SECURE_MD5_ADDR 0xb3540270
#define EFUSE_ADDR_PROT  0x3E

#define EFUSE_REGOFF_CRTL_ADDR 21
#define EFUSE_REGOFF_CRTL_LENG 16
#define EFUSE_REG_CTRL_PGEN (0x1 << 15)
#define EFUSE_REG_CTRL_WTEN (0x1 << 1)
#define EFUSE_REG_CTRL_RDEN (0x1)
#define EFUSE_REG_CTRL 0xb3540000

#define EFUSE_REG_CFG  0xb3540004

#define EFUSE_REG_STAT_RDDONE 0x1
#define EFUSE_REG_STAT_WTDONE (0x1 << 1)
#define EFUSE_REG_STAT 0xb3540008

#define EFUSE_REG_DAT1 0xb354000C
#define EFUSE_REG_DAT2 0xb3540010
#define EFUSE_REG_DAT3 0xb3540014
#define EFUSE_REG_DAT4 0xb3540018
#define EFUSE_REG_DAT5 0xb354001C
#define EFUSE_REG_DAT6 0xb3540020
#define EFUSE_REG_DAT7 0xb3540024
#define EFUSE_REG_DAT8 0xb3540028

#define EFUSE_PTCOFF_UKP   15 //user key protect
#define EFUSE_PTCOFF_MD5   14 //md5 sig protect
#define EFUSE_PTCOFF_ADE   13 //
#define EFUSE_PTCOFF_PCP   12 //
#define EFUSE_PTCOFF_TR3   11 //
#define EFUSE_PTCOFF_WDT   10 //
#define EFUSE_PTCOFF_TR1    9 //
#define EFUSE_PTCOFF_TR0    8 //
#define EFUSE_PTCOFF_UID    7 //
#define EFUSE_PTCOFF_CID    6 //
#define EFUSE_PTCOFF_ATB    5 //
#define EFUSE_PTCOFF_SCB    4 //secure boot protect
#define EFUSE_PTCOFF_NNS    3 //
#define EFUSE_PTCOFF_DJG    2 //DISJTAG protect
#define EFUSE_PTCOFF_CDC    1 //
#define EFUSE_PTCOFF_SEC    0 //

#define EFUSE_REG32(a) *(volatile unsigned int *)(a)


#define WT_OTP_UK 0x1
#define WT_OTP_NKU 0x2

#define EFUSTATE_UK_PRT_SFT (0x1 << 23)

#define EFUSTATE_MD5_PRT_SFT (0x1 << 22)
#define EFUSTATE_NKU_PRT_SFT (0x1 << 22)
#define EFUSTATE_SECBOOT_EN_SFT (0x1 << 8)

#define EFUSTATE_UK_PRT (EFUSE_REG32(EFUSE_REG_STAT) & EFUSTATE_UK_PRT_SFT)

#define EFUSTATE_MD5_PRT (EFUSE_REG32(EFUSE_REG_STAT) & EFUSTATE_MD5_PRT_SFT)
#define EFUSTATE_NKU_PRT (EFUSE_REG32(EFUSE_REG_STAT) & EFUSTATE_NKU_PRT_SFT)
#define EFUSTATE_SECBOOT_EN (EFUSE_REG32(EFUSE_REG_STAT) & EFUSTATE_SECBOOT_EN_SFT)

int cpu_burn_rckey(void);
int cpu_burn_nku(void *idata,unsigned int length);
int get_rsakeylen(void);
int cpu_get_enckey(unsigned int *odata);
int cpu_burn_ukey(void *idata);
int cpu_burn_secboot_enable(void);

#endif
