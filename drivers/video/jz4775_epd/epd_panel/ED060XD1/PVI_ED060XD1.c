/*
 * kernel/drivers/video/panel/epd_panel/PVI_ED060XD1.c for PVI ED060XD1 panel
 *
 * According to the panel param to init the
 * struct xd1_var_screeninfo and struct reg_timing for epd driver.
 *
 * Copyright (C) 2005-2013, Ingenic Semiconductor Inc.
 * http://www.ingenic.cn/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/string.h>

#include <jz4775_epd/epd_panel_info.h>
#include <jz4775_epd/jz4775_epd.h>

/*
 * GDRL		GPC0
 * GDSP		GPC1
 * GDOE		GPC2
 * SDSHR	GPC3
 * SDDO0	GPC4
 * SDDO1	GPC5
 * SDDO2	GPC6
 * SDDO3	GPC7
 * SDCK		GPC8
 * SDOE		GPC9
 * SDCEL0	GPC10
 * SDCEL1	GPC11
 * SDDO4	GPC12
 * SDDO5	GPC13
 * SDDO6	GPC14
 * SDDO7	GPC15
 * SDLE		GPC18
 * GDCLK	GPC19
 */

struct epd_panel_info_t epd_panel_info_init(void)
{
	/* epd panel param definition */
	unsigned int xres	= 1024;		/* resolution in x coordinate */
	unsigned int yres	= 758;		/* resolution in y coordinate */
	unsigned int LSL	= 6;		/* line sync length */
	unsigned int LBL	= 6;		/* line begin length */
	unsigned int LDL	= 256;		/* line data length */
	unsigned int LEL	= 38;		/* line end length */
	unsigned int GDCK_STA	= 4;		/* line end length */
	unsigned int LGONL	= 262;		/* line gate on length? */
	unsigned int FSL	= 2;		/* frame sync length */
	unsigned int FBL	= 4;		/* frame begin length */
	unsigned int FDL	= 758;		/* frame data length */
	unsigned int FEL	= 5;		/* frame end length */

	unsigned int wtr[20] = {0x0, 0x3, 0x6, 0x9, 0xc, 0xf, 0x12, 0x15, 0x18, 0x1b, 0x1e, 0x21, 0x26, 0x2b, 0x30, 0x47};

	struct epd_timing xd1_tm;
	struct fb_var_screeninfo xd1_var;
	struct epd_panel_info_t  xd1_info;

	strcpy(xd1_info.model, "PVI_ED060XD1");
	xd1_info.wfm_trc = 0xd;		/* waveform temperature region count, this value could smaller than the real region count */
	memcpy(xd1_info.wfm_tr,	wtr, sizeof(wtr));	/* waveform temperature region */

	/* timing variable init for jz47xxfb_epd_controller_init registers in epd driver */
	xd1_info.epd_tm.vt	= FSL + FBL + FDL + FEL;
	xd1_info.epd_tm.ht	= LSL + LBL + LDL + LEL;
	xd1_info.epd_tm.hpe	= LSL;
	xd1_info.epd_tm.hps	= 0;
	xd1_info.epd_tm.hds	= LSL + LBL;
	xd1_info.epd_tm.hde	= LSL + LBL + LDL;

	xd1_info.epd_tm.sdspd = 0x8088;
	xd1_info.epd_tm.sdspe = 0x8bcc;

	xd1_info.epd_tm.sdoee	= LSL;
	xd1_info.epd_tm.sdoed	= LSL + LBL + LDL + LEL;
	xd1_info.epd_tm.gdclke	= LSL + (LBL / 2);//GDCK_STA;
	xd1_info.epd_tm.gdclkd	= LSL + (LBL / 2) + LGONL;
	xd1_info.epd_tm.vps	= 0;
	xd1_info.epd_tm.vpe	= FSL;
	xd1_info.epd_tm.gdspe	= (LSL + LBL + LDL + 1) / 2;
	xd1_info.epd_tm.gdspd	= (LSL + LBL + LDL + 1) / 2;
	xd1_info.epd_tm.vds	= FSL + FBL;
	xd1_info.epd_tm.vde	= FSL + FBL + FDL;
	xd1_info.epd_tm.gdoee	= FSL;
	xd1_info.epd_tm.gdoed	= FSL + FBL + FDL + FEL;

	/* xd1_info.epd_var init for fb_info->var of jz47xxfb_set_info() in epd driver */
	xd1_info.epd_var.xres		= xres;
	xd1_info.epd_var.yres		= yres;
	xd1_info.epd_var.width		= 758;
	xd1_info.epd_var.height		= 1024;
	xd1_info.epd_var.xoffset	= 0;
	xd1_info.epd_var.yoffset	= 0;
	xd1_info.epd_var.nonstd		= 0;
	xd1_info.epd_var.bits_per_pixel	= 16;
	xd1_info.epd_var.grayscale	= 0;
	xd1_info.epd_var.red.offset	= 11;
	xd1_info.epd_var.red.length	= 5;
	xd1_info.epd_var.red.msb_right	= 0;
	xd1_info.epd_var.green.offset	= 5;
	xd1_info.epd_var.green.length	= 6;
	xd1_info.epd_var.green.msb_right	= 0;
	xd1_info.epd_var.blue.offset	= 0;
	xd1_info.epd_var.blue.length	= 5;
	xd1_info.epd_var.blue.msb_right	= 0;
	xd1_info.epd_var.transp.offset	= 0;
	xd1_info.epd_var.transp.length	= 0;
	xd1_info.epd_var.transp.msb_right = 0;
	/* Timing: All values in pixclocks; except pixclock (of course) */
	xd1_info.epd_var.pixclock		= KHZ2PICOS(40000);	/* pixel clock in ps (pico seconds) */
	xd1_info.epd_var.left_margin	= LBL;		/* time from sync to picture	*/
	xd1_info.epd_var.right_margin	= LEL;		/* time from picture to sync	*/
	xd1_info.epd_var.upper_margin	= FBL;		/* time from sync to picture	*/
	xd1_info.epd_var.lower_margin	= FEL;
	xd1_info.epd_var.hsync_len	= LSL;		/* length of horizontal sync	*/
	xd1_info.epd_var.vsync_len	= FSL;		/* length of vertical sync	*/
	xd1_info.epd_var.rotate		= 0;		/* angle we rotate counter clockwise */

	return xd1_info;
}
