#ifndef __SM5007_I2C_H__
#define __SM5007_I2C_H__

#define SM5007_PMU_ADDR  0x47
#define SM5007_FG_ADDR   0x71

#define SM5007_PWRONREG          0x00
#define SM5007_PWROFFREG         0x01
#define SM5007_REBOOTREG         0x02
#define SM5007_INTSOURCE         0x03
#define SM5007_INT1              0x04
#define SM5007_INT2              0x05
#define SM5007_INT3              0x06
#define SM5007_INT4              0x07
#define SM5007_INT5              0x08
#define SM5007_INT6              0x09
#define SM5007_INT1MSK           0x0A
#define SM5007_INT2MSK           0x0B
#define SM5007_INT3MSK           0x0C
#define SM5007_INT4MSK           0x0D
#define SM5007_INT5MSK           0x0E
#define SM5007_INT6MSK           0x0F
#define SM5007_STATUS            0x10
#define SM5007_CNTL1             0x11
#define SM5007_CNTL2             0x12

#define SM5007_CHGCNTL1          0x20
#define SM5007_CHGCNTL2          0x21
#define SM5007_CHGCNTL3          0x22

#define SM5007_BUCK1CNTL1        0x30
#define SM5007_BUCK1CNTL2        0x31
#define SM5007_BUCK1CNTL3        0x32
#define SM5007_BUCK2CNTL         0x33
#define SM5007_BUCK4CNTL         0x34

#define SM5007_LDO1CNTL          0x35
#define SM5007_LDO3CNTL          0x36
#define SM5007_LDO4CNTL          0x37
#define SM5007_LDO5CNTL          0x38
#define SM5007_LDO8CNTL          0x39
#define SM5007_LDO12OUT          0x3A
#define SM5007_LDO78OUT          0x3B
#define SM5007_LDO9OUT           0x3C

#define SM5007_PS1OUTCNTL        0x3D
#define SM5007_PS2OUTCNTL        0x3E
#define SM5007_PS3OUTCNTL        0x3F
#define SM5007_PS4OUTCNTL        0x40
#define SM5007_PS5OUTCNTL        0x41

#define SM5007_ONOFFCNTL         0x42

#define SM5007_LPMMODECNTL1      0x43
#define SM5007_LPMMODECNTL2      0x44
#define SM5007_LPMMODECNTL3      0x45

#define SM5007_DCDCMODECNTL      0x46

#define SM5007_ADISCNTL1         0x47
#define SM5007_ADISCNTL2         0x48
#define SM5007_ADISCNTL3         0x49

#define SM5007_CHIPID            0x4A

//CNTL1
#define SM5007_MASK_INT_MASK     0x02
#define SM5007_MASK_INT_SHIFT    0x1
//CNTL2
#define SM5007_GLOBAL_SHUTDOWN   0x04

/* Defined charger information */
#define	ADC_VDD_MV	2800
#define	MIN_VOLTAGE	3100
#define	MAX_VOLTAGE	4200
#define	B_VALUE		3435

/* SM5007 Register information */
/* CHGCNTL1 */
#define SM5007_TOPOFFTIMER_20MIN    0x0
#define SM5007_TOPOFFTIMER_30MIN    0x1
#define SM5007_TOPOFFTIMER_45MIN    0x2
#define SM5007_TOPOFFTIMER_DISABLED 0x3

#define SM5007_FASTTIMMER_3P5HOURS  0x0
#define SM5007_FASTTIMMER_4P5HOURS  0x1
#define SM5007_FASTTIMMER_5P5HOURS  0x2
#define SM5007_FASTTIMMER_DISABLED  0x3

#define SM5007_RECHG_M100           0x0
#define SM5007_RECHG_M50            0x1
#define SM5007_RECHG_MASK           0x04
#define SM5007_RECHG_SHIFT          0x2

#define SM5007_AUTOSTOP_DISABLED    0x0
#define SM5007_AUTOSTOP_ENABLED     0x1
#define SM5007_AUTOSTOP_MASK        0x02
#define SM5007_AUTOSTOP_SHIFT       0x1

#define SM5007_CHGEN_DISABLED     0x0
#define SM5007_CHGEN_ENABLED      0x1
#define SM5007_CHGEN_MASK         0x01
#define SM5007_CHGEN_SHIFT        0x0
/* CHGCNTL3 */
#define SM5007_TOPOFF_10MA          0x0
#define SM5007_TOPOFF_20MA          0x1
#define SM5007_TOPOFF_30MA          0x2
#define SM5007_TOPOFF_40MA          0x3
#define SM5007_TOPOFF_50MA          0x4
#define SM5007_TOPOFF_60MA          0x5
#define SM5007_TOPOFF_70MA          0x6
#define SM5007_TOPOFF_80MA          0x7

#define SM5007_TOPOFF_MASK         0xE0
#define SM5007_TOPOFF_SHIFT        0x5

#define SM5007_FASTCHG_MASK         0x1F
#define SM5007_FASTCHG_SHIFT        0x0

/**************************/

/* detailed status in STATUS (0x10) */
#define SM5007_STATE_VBATOVP    0x01
#define SM5007_STATE_VBUSPOK    0x02
#define SM5007_STATE_VBUSUVLO   0x04
#define SM5007_STATE_VBUSOVP    0x08
#define SM5007_STATE_CHGON      0x10
#define SM5007_STATE_TOPOFF     0x20
#define SM5007_STATE_DONE       0x40
#define SM5007_STATE_THEMSAFE   0x80

int sm5007_set_bits(unsigned char addr, u8 reg, uint8_t bit_mask);
int sm5007_clr_bits(unsigned char addr, u8 reg, uint8_t bit_mask);
int sm5007_read(unsigned char addr, u8 reg, uint8_t *val);
int sm5007_write(unsigned char addr, u8 reg, uint8_t val);

#endif
