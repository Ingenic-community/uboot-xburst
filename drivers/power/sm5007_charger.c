#include <config.h>
#include <common.h>
#include <power/sm5007.h>
#include <power/sm5007_charger.h>

struct sm5007_charger_type_data charger_type = {
        .chg_batreg = 4200, /* mV */
        .chg_fastchg = 200, /* mA */
        .chg_autostop = SM5007_AUTOSTOP_ENABLED, /* autostop enable */
        .chg_rechg = SM5007_RECHG_M100, /* -100mV */
        .chg_topoff = 10, /* mA */
};

static void sm5007_enable_autostop(struct sm5007_charger_info *info, int onoff)
{
    if (onoff)
        sm5007_set_bits(SM5007_PMU_ADDR, SM5007_CHGCNTL1, SM5007_AUTOSTOP_MASK);
    else
        sm5007_clr_bits(SM5007_PMU_ADDR, SM5007_CHGCNTL1, SM5007_AUTOSTOP_MASK);
}

static void sm5007_set_rechg(struct sm5007_charger_info *info, int threshold)
{
    if (threshold == SM5007_RECHG_M50)
        sm5007_set_bits(SM5007_PMU_ADDR, SM5007_CHGCNTL1, SM5007_RECHG_MASK);
    else
        sm5007_clr_bits(SM5007_PMU_ADDR, SM5007_CHGCNTL1, SM5007_RECHG_MASK);
}

void sm5007_enable_chgen(int onoff)
{
    if (onoff)
        sm5007_set_bits(SM5007_PMU_ADDR, SM5007_CHGCNTL1, SM5007_CHGEN_MASK);
    else
        sm5007_clr_bits(SM5007_PMU_ADDR, SM5007_CHGCNTL1, SM5007_CHGEN_MASK);
}

static void sm5007_set_topoff_current(struct sm5007_charger_info *info,
        int topoff_current_ma)
{
    int temp = 0, reg_val = 0, data = 0;

    if (topoff_current_ma <= 10)
        topoff_current_ma = 10;
    else if (topoff_current_ma >= 80)
        topoff_current_ma = 80;

    data = (topoff_current_ma - 10) / 10;

    sm5007_read(SM5007_PMU_ADDR, SM5007_CHGCNTL3, (uint8_t *) &reg_val);

    temp = ((reg_val & ~SM5007_TOPOFF_MASK) | (data << SM5007_TOPOFF_SHIFT));

    sm5007_write(SM5007_PMU_ADDR, SM5007_CHGCNTL3, temp);

}

static void sm5007_set_fast_charging_current(struct sm5007_charger_info *info,
        int charging_current_ma)
{
    int temp = 0, reg_val = 0, data = 0;

    if (charging_current_ma <= 60)
        charging_current_ma = 60;
    else if (charging_current_ma >= 600)
        charging_current_ma = 600;

    data = (charging_current_ma - 60) / 20;

    sm5007_read(SM5007_PMU_ADDR, SM5007_CHGCNTL3, (uint8_t *) &reg_val);

    temp = ((reg_val & ~SM5007_FASTCHG_MASK) | (data << SM5007_FASTCHG_SHIFT));

    sm5007_write(SM5007_PMU_ADDR, SM5007_CHGCNTL3, temp);

}

static void sm5007_set_regulation_voltage(struct sm5007_charger_info *info,
        int float_voltage_mv)
{
    int data = 0;

    if ((float_voltage_mv) <= 4000)
        float_voltage_mv = 4000;
    else if ((float_voltage_mv) >= 4430)
        float_voltage_mv = 4300;

    data = ((float_voltage_mv - 4000) / 10);

    sm5007_write(SM5007_PMU_ADDR, SM5007_CHGCNTL2, data);

}

static int sm5007_init_charger(struct sm5007_charger_info *info)
{
    int err = 0;

    sm5007_enable_autostop(info, info->type.chg_autostop); //AUTOSTOP : Enable
    sm5007_set_rechg(info, info->type.chg_autostop); //RECHG : -100mV
    sm5007_set_regulation_voltage(info, info->type.chg_batreg); //BATREG : 4350mV
    sm5007_set_topoff_current(info, info->type.chg_topoff); //Topoff current : 10mA
    sm5007_set_fast_charging_current(info, info->type.chg_fastchg); //Fastcharging current : 200mA
    //	sm5007_enable_chgen(1);

    return err;
}

struct sm5007_charger_info *info;
int sm5007_charger_probe(void)
{
    info->type = charger_type;
    sm5007_init_charger(info);
    return 0;
}
