#ifndef __SM5007_FUELGAUGE_H__
#define __SM5007_FUELGAUGE_H__

/* Definitions of SM5007 Fuelgauge Registers */
// I2C Register
#define SM5007_REG_DEVICE_ID                 0x00
#define SM5007_REG_CNTL                      0x01
#define SM5007_REG_INTFG                     0x02
#define SM5007_REG_INTFG_MASK                0x03
#define SM5007_REG_STATUS                    0x04
#define SM5007_REG_SOC                       0x05
#define SM5007_REG_OCV                       0x06
#define SM5007_REG_VOLTAGE                   0x07
#define SM5007_REG_CURRENT                   0x08
#define SM5007_REG_TEMPERATURE               0x09
#define SM5007_REG_FG_OP_STATUS              0x10

#define SM5007_REG_V_ALARM                   0x0C
#define SM5007_REG_T_ALARM                   0x0D
#define SM5007_REG_SOC_ALARM                 0x0E

#define SM5007_REG_TOPOFFSOC                 0x12
#define SM5007_REG_PARAM_CTRL                0x13
#define SM5007_REG_PARAM_RUN_UPDATE          0x14
#define SM5007_REG_VIT_PERIOD                0x1A
#define SM5007_REG_MIX_RATE                  0x1B
#define SM5007_REG_MIX_INIT_BLANK            0x1C

//for cal
#define SM5007_REG_VOLT_CAL         0x50
#define SM5007_REG_CURR_CAL         0x51

#define SM5007_REG_RCE0             0x20
#define SM5007_REG_RCE1             0x21
#define SM5007_REG_RCE2             0x22
#define SM5007_REG_DTCD             0x23
#define SM5007_REG_RS               0x24

#define SM5007_REG_RS_MIX_FACTOR    0x25
#define SM5007_REG_RS_MAX           0x26
#define SM5007_REG_RS_MIN           0x27
#define SM5007_REG_RS_MAN           0x29

#define SM5007_REG_RS_TUNE          0x28
#define SM5007_REG_CURRENT_EST      0x85
#define SM5007_REG_CURRENT_ERR      0x86
#define SM5007_REG_Q_EST            0x87

#define SM5007_VIT_PERIOD			0x1A

#define SM5007_FG_PARAM_UNLOCK_CODE     0x3700
#define SM5007_FG_PARAM_LOCK_CODE     0x0000

#define SM5007_FG_TABLE_LEN    0xF//real table length -1
//start reg addr for table
#define SM5007_REG_TABLE_START     0xA0

#define RS_MAN_CNTL         0x0800

// control register value
#define ENABLE_MIX_MODE         0x8000
#define ENABLE_TEMP_MEASURE     0x4000
#define ENABLE_TOPOFF_SOC       0x2000
#define ENABLE_RS_MAN_MODE      0x0400
#define ENABLE_SOC_ALARM        0x0008
#define ENABLE_T_H_ALARM        0x0004
#define ENABLE_T_L_ALARM        0x0002
#define ENABLE_V_ALARM          0x0001

#define DISABLE_RE_INIT         0x0007

#define TOPOFF_SOC_100  0x111
#define TOPOFF_SOC_99   0x110
#define TOPOFF_SOC_98   0x101
#define TOPOFF_SOC_97   0x100
#define TOPOFF_SOC_96   0x011
#define TOPOFF_SOC_95   0x010
#define TOPOFF_SOC_94   0x001
#define TOPOFF_SOC_93   0x000

#define ENABLE_L_SOC_INT  0x0008
#define ENABLE_H_TEM_INT  0x0004
#define ENABLE_L_TEM_INT  0x0002
#define ENABLE_L_VOL_INT  0x0001

#define ENABLE_L_VOL_STATUS  0x0001

#define SM5007_EN_LP_MODE	0x0800
#define ENABLE_N_VS_LPM		0x0000

#define FULL_SOC						100

int battery_table_para[][16] = {
	{ 0x1400, 0x1AE1, 0x1BA6, 0x1C62, 0x1D22,0x1D61, 0x1DC9, 0x1E13, 0x1E2F, 0x1E44, 0x1E78, 0x1EBC, 0x1F39, 0x1FF9, 0x215B, 0x2266 },
	{ 0x1400, 0x1AE1, 0x1BA6, 0x1C62, 0x1D22, 0x1D61,0x1DC9, 0x1E13, 0x1E2F, 0x1E44, 0x1E78, 0x1EBC, 0x1F39, 0x1FF9, 0x215B, 0x2266 },
	{ 0x0, 0x3, 0xD, 0x1D, 0x34, 0x44, 0x75, 0xB7, 0xD8, 0x119, 0x15B, 0x18C, 0x1BD, 0x21F, 0x29C, 0x2A2 }
};

struct sm5007_fg_info {
	/* State Of Connect */
	int online;
	/* battery SOC (capacity) */
	int batt_soc;
	/* battery voltage */
	int batt_voltage;
	/* battery AvgVoltage */
	int batt_avgvoltage;
	/* battery OCV */
	int batt_ocv;
	/* Current */
	int batt_current;
	/* battery Avg Current */
	int batt_avgcurrent;

	struct device *dev;
	int32_t temperature;
	; /* 0.1 deg C*/
	/* register programming */
	int reg_addr;
	u8 reg_data[2];

	int battery_table[3][16];
	int rce_value[3];
	int dtcd_value;
	int rs_value[4]; /*rs mix_factor max min*/
	int vit_period;
	int mix_value[2]; /*mix_rate init_blank*/

	int enable_topoff_soc;
	int topoff_soc;

	int volt_cal;
	int curr_cal;

	int temp_std;
	int temp_offset;
	int temp_offset_cal;
	int charge_offset_cal;

	int battery_type; /* 4200 or 4350 or 4400*/
	uint32_t soc_alert_flag :1; /* 0 : nu-occur, 1: occur */
	uint32_t volt_alert_flag :1; /* 0 : nu-occur, 1: occur */
	uint32_t flag_full_charge :1; /* 0 : no , 1 : yes*/
	uint32_t flag_chg_status :1; /* 0 : discharging, 1: charging*/
	int is_charging;

	int iocv_error_count;
	/* previous battery voltage */
	int p_batt_voltage;
	int p_batt_current;

	/*low voltage alarm mV level*/
	int alarm_vol_mv;

	int32_t irq_ctrl;

};

struct sm5007_fuelgauge_type_data {
	int id;
	int battery_type;
	int *battery_table;
	int temp_std;
	int temp_offset;
	int temp_offset_cal;
	int charge_offset_cal;
	int rce_value[3];
	int dtcd_value;
	int rs_value[4];
	int vit_period;
	int mix_value[2];
	int topoff_soc[2];
	int volt_cal;
	int curr_cal;
};

#endif
