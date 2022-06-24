#include <config.h>
#include <common.h>
#include <power/sm5007.h>
#include <power/sm5007_fuelgauge.h>

static struct sm5007_fg_info info;

enum battery_table_type {
	DISCHARGE_TABLE = 0,
	CHARGE_TABLE,
	Q_TABLE,
	TABLE_MAX,
};

static struct sm5007_fuelgauge_type_data sm5007_fuelgauge_data = {
	.id = 0,
	.battery_type = 4200, /* 4.20V for 320mAh */
	.temp_std = 25,
	.temp_offset = 10,
	.temp_offset_cal = 0x01,
	.charge_offset_cal = 0x00,
	.rce_value[0] = 0x0749,
	.rce_value[1] = 0x0580,
	.rce_value[2] = 0x371,
	.dtcd_value = 0x1,
	.rs_value[0] = 0x1ae, /*rs mix_factor max min*/
	.rs_value[1] = 0x47a,
	.rs_value[2] = 0x3800,
	.rs_value[3] = 0x00a4,
	.vit_period = 0x3506,
	.mix_value[0] = 0x0a03, /*mix_rate init_blank*/
	.mix_value[1] = 0x0004,
	.topoff_soc[0] = 0x0, /*enable topoff_soc  1: enable, 0 disable*/
	.topoff_soc[1] = 0x000, /*topoff_soc threadhold*/
	.volt_cal = 0x8000,
	.curr_cal = 0x8000,
};

int sm5007_fg_bulk_reads_bank1(unsigned char addr, u8 reg, u8 len, uint8_t *val)
{
	int ret = -1;

	ret = i2c_read(addr, reg, 1, val, len);

	if(ret != 0)
		ret = -1;
	return ret;
}

int sm5007_fg_bulk_writes_bank1(unsigned char addr, u8 reg,
		u8 len, uint8_t *val)
{
	int ret = -1;

	ret = i2c_write(addr, reg, 1, val, len);

	if(ret != 0)
		ret = -1;
	return ret;
}

static int32_t sm5007_fg_i2c_read_word(unsigned char addr, unsigned char reg)
{
	uint16_t data = -1;
	int ret;
	ret = sm5007_fg_bulk_reads_bank1(addr, reg, 2, (uint8_t *) &data);

	if (ret != 0)
		return ret;
	else
		return data;
}

static int32_t sm5007_fg_i2c_write_word(unsigned char addr,
		unsigned char reg, uint16_t data)
{
	int ret;

	ret = sm5007_fg_bulk_writes_bank1(addr, reg, 2, (uint8_t *) &data);

	return ret;
}

static bool sm5007_fg_reg_init(void)
{
	int i, j, value;
	uint8_t table_reg;

	sm5007_fg_i2c_write_word(SM5007_FG_ADDR, SM5007_REG_PARAM_CTRL,
			SM5007_FG_PARAM_UNLOCK_CODE);

	/* RCE write*/
	for (i = 0; i < 3; i++) {
		sm5007_fg_i2c_write_word(SM5007_FG_ADDR, SM5007_REG_RCE0 + i,
				info.rce_value[i]);
	}

	/* DTCD write*/
	sm5007_fg_i2c_write_word(SM5007_FG_ADDR, SM5007_REG_DTCD, info.dtcd_value);

	/* RS write*/
	sm5007_fg_i2c_write_word(SM5007_FG_ADDR, SM5007_REG_RS, info.rs_value[0]);

	/*/ VIT_PERIOD write*/
	sm5007_fg_i2c_write_word(SM5007_FG_ADDR, SM5007_REG_VIT_PERIOD,
			info.vit_period);

	/*/ TABLE_LEN write & pram unlock*/
	sm5007_fg_i2c_write_word(SM5007_FG_ADDR, SM5007_REG_PARAM_CTRL,
			SM5007_FG_PARAM_UNLOCK_CODE | SM5007_FG_TABLE_LEN);

	for (i = 0; i < 3; i++) {
		table_reg = SM5007_REG_TABLE_START + (i << 4);
		for (j = 0; j <= SM5007_FG_TABLE_LEN; j++) {
			sm5007_fg_i2c_write_word(SM5007_FG_ADDR, (table_reg + j),
					info.battery_table[i][j]);
		}
	}

	/* MIX_MODE write*/
	sm5007_fg_i2c_write_word(SM5007_FG_ADDR, SM5007_REG_RS_MIX_FACTOR,
			info.rs_value[1]);
	sm5007_fg_i2c_write_word(SM5007_FG_ADDR, SM5007_REG_RS_MAX,
			info.rs_value[2]);
	sm5007_fg_i2c_write_word(SM5007_FG_ADDR, SM5007_REG_RS_MIN,
			info.rs_value[3]);
	sm5007_fg_i2c_write_word(SM5007_FG_ADDR, SM5007_REG_MIX_RATE,
			info.mix_value[0]);
	sm5007_fg_i2c_write_word(SM5007_FG_ADDR, SM5007_REG_MIX_INIT_BLANK,
			info.mix_value[1]);

	/* CAL write*/
	sm5007_fg_i2c_write_word(SM5007_FG_ADDR, SM5007_REG_CURR_CAL, info.curr_cal);

	/*INIT_last -  control register set*/
	value = sm5007_fg_i2c_read_word(SM5007_FG_ADDR, SM5007_REG_CNTL);
	value |= ENABLE_MIX_MODE | ENABLE_TEMP_MEASURE | ENABLE_V_ALARM;

	/*TOPOFFSOC*/
	if (info.enable_topoff_soc)
		value |= ENABLE_TOPOFF_SOC;
	else
		value &= (~ENABLE_TOPOFF_SOC);

	sm5007_fg_i2c_write_word(SM5007_FG_ADDR, SM5007_REG_CNTL, value);

	/* LOCK ?*/
	value = SM5007_FG_PARAM_LOCK_CODE | SM5007_FG_TABLE_LEN;
	sm5007_fg_i2c_write_word(SM5007_FG_ADDR, SM5007_REG_PARAM_CTRL, value);

	return 1;
}

unsigned int fg_get_vbat(void)
{
	int ret;
	unsigned int vbat;/* = 3500; 3500 means 3500mV*/
	ret = sm5007_fg_i2c_read_word(SM5007_FG_ADDR, SM5007_REG_VOLTAGE);

	if (ret < 0) {
		vbat = 4000;
	} else {
		/*integer bit*/
		vbat = ((ret & 0xf000) >> 0xc) * 1000;
		/* integer + fractional bit*/
		vbat = vbat + (((ret & 0x0fff) * 1000) / 4096);
	}
	info.batt_voltage = vbat;

	/*cal avgvoltage*/
	info.batt_avgvoltage = (((info.batt_avgvoltage) * 4) + vbat) / 5;

	return vbat;
}

int fg_get_curr(void)
{
	int ret;

	int curr;/* = 1000; 1000 means 1000mA*/
	ret = sm5007_fg_i2c_read_word(SM5007_FG_ADDR, SM5007_REG_CURRENT);
	if (ret < 0) {
		curr = 0;
	} else {
		/*integer bit*/
		curr = ((ret & 0x7800) >> 0xb) * 1000;
		/* integer + fractional bit*/
		curr = curr + (((ret & 0x07ff) * 1000) / 2048);
		if (ret & 0x8000)
			curr *= -1;
	}
	info.batt_current = curr;

	/*cal avgcurr*/
	info.batt_avgcurrent = (((info.batt_avgcurrent) * 4) + curr) / 5;

	return curr;
}

int fg_get_temp(void)
{
	int ret;

	int temp;/* = 250; 250 means 25.0oC*/
	ret = sm5007_fg_i2c_read_word(SM5007_FG_ADDR, SM5007_REG_TEMPERATURE);
	if (ret < 0) {
		temp = 0;
	} else {
		/*integer bit*/
		temp = ((ret & 0x7F00) >> 8) * 10;
		/* integer + fractional bit*/
		temp = temp + ((((ret & 0x00f0) >> 4) * 10) / 16);
		if (ret & 0x8000)
			temp *= -1;
	}
	info.temperature = temp;

	return temp;
}

unsigned int fg_get_soc(void)
{
	int ret;
	unsigned int soc;
	int curr_cal;
	int temp_cal_fact;

	info.is_charging = (info.batt_current > 10) ? true : false;

	if (info.is_charging)
		curr_cal = info.curr_cal + (info.charge_offset_cal);
	else
		curr_cal = info.curr_cal;

	temp_cal_fact = info.temp_std - (info.temperature / 10);
	temp_cal_fact = temp_cal_fact / info.temp_offset;
	temp_cal_fact = temp_cal_fact * info.temp_offset_cal;
	curr_cal = curr_cal + (temp_cal_fact);

	sm5007_fg_i2c_write_word(SM5007_FG_ADDR, SM5007_REG_CURR_CAL, curr_cal);

	ret = sm5007_fg_i2c_read_word(SM5007_FG_ADDR, SM5007_REG_SOC);
	if (ret < 0) {
		soc = 500;
	} else {
		/*integer bit;*/
		soc = ((ret & 0xff00) >> 8) * 10;
		/* integer + fractional bit*/
		soc = soc + (((ret & 0x00ff) * 10) / 256);
	}

	info.batt_soc = soc;

	return soc;
}

unsigned int fg_get_ocv(void)
{
	int ret;
	unsigned int ocv;/* = 3500; 3500 means 3500mV*/
	ret = sm5007_fg_i2c_read_word(SM5007_FG_ADDR, SM5007_REG_OCV);
	if (ret < 0) {
		ocv = 4000;
	} else {
		/*integer bit;*/
		ocv = ((ret & 0xf000) >> 0xc) * 1000;
		/* integer + fractional bit*/
		ocv = ocv + (((ret & 0x0fff) * 1000) / 4096);
	}

	info.batt_ocv = ocv;

	return ocv;
}

static bool sm5007_fg_init(void)
{
	sm5007_fg_reg_init();

	/* get first voltage measure to avgvoltage*/
	info.batt_avgvoltage = fg_get_vbat();

	/* get first temperature*/
	info.temperature = fg_get_temp();

	return true;
}

int sm5007_fg_probe(void)
{
	int i, j;

	/***********these valuse are set in platform ************/
	/* get battery_table*/
	for (i = DISCHARGE_TABLE; i < TABLE_MAX; i++) {
		for (j = 0; j <= SM5007_FG_TABLE_LEN; j++) {
			info.battery_table[i][j] = battery_table_para[i][j];
		}
	}

	/* get rce*/
	info.rce_value[0] = sm5007_fuelgauge_data.rce_value[0];
	info.rce_value[1] = sm5007_fuelgauge_data.rce_value[1];
	info.rce_value[2] = sm5007_fuelgauge_data.rce_value[2];

	/* get dtcd_value*/
	info.dtcd_value = sm5007_fuelgauge_data.dtcd_value;

	/* get rs_value*/
	info.rs_value[0] = sm5007_fuelgauge_data.rs_value[0];
	info.rs_value[1] = sm5007_fuelgauge_data.rs_value[1];
	info.rs_value[2] = sm5007_fuelgauge_data.rs_value[2];
	info.rs_value[3] = sm5007_fuelgauge_data.rs_value[3];

	/* get vit_period*/
	info.vit_period = sm5007_fuelgauge_data.vit_period;

	/* get mix_value*/
	info.mix_value[0] = sm5007_fuelgauge_data.mix_value[0];
	info.mix_value[1] = sm5007_fuelgauge_data.mix_value[1];

	/* battery_type*/
	info.battery_type = sm5007_fuelgauge_data.battery_type;

	/* VOL & CURR CAL*/
	info.volt_cal = sm5007_fuelgauge_data.volt_cal;
	info.curr_cal = sm5007_fuelgauge_data.curr_cal;

	/* temp_std*/
	info.temp_std = sm5007_fuelgauge_data.temp_std;

	/* temp_offset*/
	info.temp_offset = sm5007_fuelgauge_data.temp_offset;

	/* temp_offset_cal*/
	info.temp_offset_cal = sm5007_fuelgauge_data.temp_offset_cal;

	/* charge_offset_cal*/
	info.charge_offset_cal = sm5007_fuelgauge_data.charge_offset_cal;

	/*TOPOFF_ENABLE*/
	info.enable_topoff_soc = sm5007_fuelgauge_data.topoff_soc[0];

	/*TOPOFF_SOC*/
	info.topoff_soc = sm5007_fuelgauge_data.topoff_soc[1];
	/**************************************************/
	sm5007_fg_init();
	return 0;
}

void sm5007_fuelgauge_work(void)
{
	fg_get_vbat();
	fg_get_ocv();
	fg_get_curr();
	fg_get_temp();
	fg_get_soc();
}

