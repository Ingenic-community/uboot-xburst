#ifndef __SM5007_CHARGER_FUELGAUGE_API_H__
#define __SM5007_CHARGER_FUELGAUGE_API_H__
//regulator
int sm5007_regulator_init(void);
void sm5007_shutdown(void);
//regulator end


//charger
extern int sm5007_charger_probe(void);
extern void sm5007_enable_chgen(int onoff);
//charger end


//fuelgauge
extern int sm5007_fg_probe(void);
extern unsigned int fg_get_vbat(void);
extern int fg_get_curr(void);
extern int fg_get_temp(void);
extern unsigned int fg_get_soc(void);
extern unsigned int fg_get_ocv(void);
extern void sm5007_fuelgauge_work(void);
//fuelgauge end
#endif
