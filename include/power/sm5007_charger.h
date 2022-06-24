#ifndef __SM5007_CHARGER_H__
#define __SM5007_CHARGER_H__

struct sm5007_charger_type_data {
    int chg_batreg;
    int chg_fastchg;
    int chg_autostop;
    int chg_rechg;
    int chg_topoff;
};

struct sm5007_charger_info {

    uint8_t chg_status; /* charging status */
    int status;

    struct sm5007_charger_type_data type;
};

#endif
