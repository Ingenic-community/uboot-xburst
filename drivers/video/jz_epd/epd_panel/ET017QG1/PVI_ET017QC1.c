
#include <jz_epd/epd_panel_info.h>

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
/*static void epd_pin_mapping()
{
	REG_GPIO_PXINTC(2) = 0x000cffff;
	REG_GPIO_PXMASKC(2) = 0x000cffff;
	REG_GPIO_PXPAT0C(2) = 0x000cffff;
	REG_GPIO_PXPAT1C(2) = 0x000cffff;
}*/

struct epd_panel_info_t epd_panel_info_init(void)
{
	/* epd panel param definition */
	unsigned int xres	= 320;		/* resolution in x coordinate */
	unsigned int yres	= 240;		/* resolution in y coordinate */
	unsigned int LSL	= 4;		/* line sync length */
	unsigned int LBL	= 12;		/* line begin length */
	unsigned int LDL	= 80;		/* line data length */ // 240/4
	unsigned int LEL	= 80;		/* line end length */
	unsigned int LGONL	= 84;		/* line gate on length? */
	unsigned int FSL	= 2;		/* frame sync length */
	unsigned int FBL	= 4;		/* frame begin length */
	unsigned int FDL	= 240;		/* frame data length */ //yres
	unsigned int FEL	= 5;		/* frame end length */

	unsigned int wtr[20] = {0x0, 0x3, 0x6, 0x9, 0xc, 0xf, 0x12, 0x15, 0x18, 0x1b, 0x1e, 0x21, 0x26, 0x2b, 0x30, 0x47};

	struct epd_timing xd1_tm;
	struct fb_var_screeninfo xd1_var;
	struct epd_panel_info_t xd1_info;

	/*epd_pin_mapping();*/
//	strcpy(xd1_info.model, "PVI_ED060XD1");
	xd1_info.wfm_trc = 0xd;		/* waveform temperature region count, this value could smaller than the real region count */
	memcpy(xd1_info.wfm_tr,	wtr, sizeof(wtr));	/* waveform temperature region */

	/* timing variable init for jz47xxfb_epd_controller_init registers in epd driver */
	xd1_tm.vt	= FSL + FBL + FDL + FEL;
	xd1_tm.ht	= LSL + LBL + LDL + LEL;
	xd1_tm.hpe	= LSL;
	xd1_tm.hps	= 0;
	xd1_tm.hds	= LSL + LBL;
	xd1_tm.hde	= LSL + LBL + LDL;
	xd1_tm.sdoee	= LSL;
	xd1_tm.sdoed	= LSL + LBL + LDL + LEL;
	xd1_tm.gdclke	= LSL + (LBL / 2);
	xd1_tm.gdclkd	= LSL + (LBL / 2) + LGONL;
	xd1_tm.vps	= 0;
	xd1_tm.vpe	= FSL;
	xd1_tm.sdspd = 0xffff;
	xd1_tm.sdspe = 0xffff;
	xd1_tm.gdspe	= ((LSL + LBL + LDL + 1) / 2);
	xd1_tm.gdspd	= ((LSL + LBL + LDL + 1) / 2);
	xd1_tm.vds	= FSL + FBL;
	xd1_tm.vde	= FSL + FBL + FDL;
	xd1_tm.gdoee	= FSL;
	xd1_tm.gdoed	= FSL + FBL + FDL + FEL;

	/* xd1_var init for fb_info->var of jz47xxfb_set_info() in epd driver */
	xd1_var.xres		= xres;
	xd1_var.yres		= yres;
	xd1_var.width		= yres;
	xd1_var.height		= xres;
	xd1_var.xoffset		= 0;
	xd1_var.yoffset		= 0;
	xd1_var.nonstd		= 0;
//	xd1_var.activate	= FB_ACTIVATE_NOW;
//	xd1_var.accel_flags	= FB_ACCELF_TEXT;
	xd1_var.bits_per_pixel	= 24; //16;
	xd1_var.grayscale	= 0;
	xd1_var.red.offset	= 11;
	xd1_var.red.length	= 5;
	xd1_var.red.msb_right	= 0;
	xd1_var.green.offset	= 5;
	xd1_var.green.length	= 6;
	xd1_var.green.msb_right	= 0;
	xd1_var.blue.offset	= 0;
	xd1_var.blue.length	= 5;
	xd1_var.blue.msb_right	= 0;
	xd1_var.transp.offset	= 0;
	xd1_var.transp.length	= 0;
	xd1_var.transp.msb_right = 0;
	/* Timing: All values in pixclocks; except pixclock (of course) */
	xd1_var.pixclock		= KHZ2PICOS(16000);	/* pixel clock in ps (pico seconds) */
	xd1_var.left_margin	= LBL;		/* time from sync to picture	*/
	xd1_var.right_margin	= LEL;		/* time from picture to sync	*/
	xd1_var.upper_margin	= FBL;		/* time from sync to picture	*/
	xd1_var.lower_margin	= FEL;
	xd1_var.hsync_len	= LSL;		/* length of horizontal sync	*/
	xd1_var.vsync_len	= FSL;		/* length of vertical sync	*/
	xd1_var.rotate		= 0;		/* angle we rotate counter clockwise */
//	xd1_var.vmode		= FB_VMODE_NONINTERLACED;

	xd1_info.epd_tm		= xd1_tm;
	xd1_info.epd_var	= xd1_var;

	return xd1_info;
}
