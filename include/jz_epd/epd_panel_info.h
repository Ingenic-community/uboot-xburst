/*
 * Copyright (C) 2005-2013, Ingenic Semiconductor Inc.
 * http://www.ingenic.cn/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef __EPD_PANEL_INFO_H__
#define __EPD_PANEL_INFO_H__
#include "jz_epd.h"
struct epd_timing
{
	unsigned int ht, vt;		/* for REG_EPD_VAT */
	unsigned int hps, hpe;		/* for REG_EPD_HSYNC */
	unsigned int sdspe, sdspd;	/* for REG_EPD_SDSP */
	unsigned int hds, hde;		/* for REG_EPD_DAH */
	unsigned int sdoee, sdoed;	/* for REG_EPD_SDOE */
	unsigned int gdclke, gdclkd;	/* for REG_EPD_GDCLK */
	unsigned int vps, vpe; 		/* for REG_EPD_VSYNC */
	unsigned int gdspe, gdspd;	/* for REG_EPD_GDSP */
	unsigned int vds, vde;		/* for REG_EPD_DAV */
	unsigned int gdoee, gdoed;	/* for REG_EPD_GDOE */
};

struct epd_panel_info_t
{
	unsigned char model[20];	/* model NO. of EPD panel */
	unsigned int wfm_trc;		/* waveform temperature region count */
	unsigned int wfm_tr[16];

	struct epd_timing epd_tm;
	struct fb_var_screeninfo epd_var;
};


#endif  /*__EPD_PANEL_INFO_H__*/
