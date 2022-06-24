/*
  * M200 EPD_LCDC DRIVER
  *
  * Copyright (c) 2015 Ingenic Semiconductor Co.,Ltd
  *
  * This program is free software; you can redistribute it and/or
  * modify it under the terms of the GNU General Public License as
  * published by the Free Software Foundation; either version 2 of
  * the License, or (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program; if not, write to the Free Software
  * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
  * MA 02111-1307 USA
  */

#include <asm/io.h>
#include <config.h>
#include <serial.h>
#include <common.h>
#include <malloc.h>
#include <lcd.h>
#include <regulator.h>
#include <asm/arch/gpio.h>
#include <asm/arch/clk.h>
#include <asm/arch-m200/epdc.h>
#include <jz_epd/jz_epd.h>
#include <jz_epd/epd_panel_info.h>


#define	CURBUF_NUM	1
#define EPD_BPP		4
#define	MAX_FRAME_COUNT	256
#define	SIZE_PER_FRAME	64
#define WFM_LUT_SIZE	MAX_FRAME_COUNT * SIZE_PER_FRAME

#define DEBUG 0
#if DEBUG
#define dprintf(format, arg...) printf(format, ## arg)
#else
#define dprintf(format, arg...) do {} while (0)
#endif


#define WAIT_STATUS(condition)	{\
	int timeout = 2000; \
	dprintf("status waitting\n"); \
	while(!(condition) && timeout){ \
		mdelay(1);\
		timeout--; \
	}; \
	if(!timeout){ \
		dprintf("wait error:REG_EPD_STA = 0x%08x\n",REG_EPD_STA); \
	} \
}



DECLARE_GLOBAL_DATA_PTR;

extern void board_set_lcd_power_on(void);
extern unsigned char WAVEFORM_LUT[];
extern struct compression_lut COMPRESSION_LUT[];
extern struct jz_epd_power_ctrl epd_power_ctrl;
extern vidinfo_t panel_info;

extern int lcd_get_size(int *line_length);
extern struct epd_panel_info_t  epd_panel_info_init(void);


static unsigned int frame_buffer;
static unsigned int current_buffer;
static unsigned int working_buffer;
static unsigned int pixcnt_buffer;
static unsigned int waveform_buffer;
static unsigned int wfm_lut_buf;


static int line_length;
static unsigned int frm_buf_size;
static unsigned int cur_buf_size;
static unsigned int wrk0_buf_size;				/* new + old pix buffer */
static unsigned int wrk1_buf_size;	/* pixcnt buffer */
static unsigned int wfm_buf_size;	/*COMPRESSION_LUT[0].number*/
static struct epd_panel_info_t epd_panel_info;

static unsigned int frame_num;
static unsigned int epd_temp = 5;

static unsigned int  jz_epd_base = 0x88000000;   //epd buffer addr base

static void print_epdc_registers(void)
{
	dprintf("REG_EPD_CTRL		= 0x%08x\n", REG_EPD_CTRL);
	dprintf("REG_EPD_CFG 		= 0x%08x\n", REG_EPD_CFG);
	dprintf("REG_EPD_STA 		= 0x%08x\n", REG_EPD_STA);
	dprintf("REG_EPD_ISRC		= 0x%08x\n", REG_EPD_ISRC);
	dprintf("REG_EPD_DIS0		= 0x%08x\n", REG_EPD_DIS0);
	dprintf("REG_EPD_DIS1		= 0x%08x\n", REG_EPD_DIS1);
	dprintf("REG_EPD_SIZE		= 0x%08x\n", REG_EPD_SIZE);
	dprintf("REG_EPD_VAT		= 0x%08x\n", REG_EPD_VAT);
	dprintf("REG_EPD_DAV		= 0x%08x\n", REG_EPD_DAV);
	dprintf("REG_EPD_DAH		= 0x%08x\n", REG_EPD_DAH);
	dprintf("REG_EPD_VSYNC		= 0x%08x\n", REG_EPD_VSYNC);
	dprintf("REG_EPD_HSYNC		= 0x%08x\n", REG_EPD_HSYNC);
	dprintf("REG_EPD_GDCLK		= 0x%08x\n", REG_EPD_GDCLK);
	dprintf("REG_EPD_GDOE		= 0x%08x\n", REG_EPD_GDOE);
	dprintf("REG_EPD_GDSP		= 0x%08x\n", REG_EPD_GDSP);
	dprintf("REG_EPD_SDOE		= 0x%08x\n", REG_EPD_SDOE);
	dprintf("REG_EPD_SDSP		= 0x%08x\n", REG_EPD_SDSP);
	dprintf("REG_EPD_PMGR0		= 0x%08x\n", REG_EPD_PMGR0);
	dprintf("REG_EPD_PMGR1		= 0x%08x\n", REG_EPD_PMGR1);
	dprintf("REG_EPD_PMGR2		= 0x%08x\n", REG_EPD_PMGR2);
	dprintf("REG_EPD_PMGR3		= 0x%08x\n", REG_EPD_PMGR3);
	dprintf("REG_EPD_PMGR4		= 0x%08x\n", REG_EPD_PMGR4);
	dprintf("REG_EPD_LUTBF		= 0x%08x\n", REG_EPD_LUTBF);
	dprintf("REG_EPD_LUTSIZE		= 0x%08x\n", REG_EPD_LUTSIZE);
	dprintf("REG_EPD_CURBF		= 0x%08x\n", REG_EPD_CURBF);
	dprintf("REG_EPD_CURSIZE		= 0x%08x\n", REG_EPD_CURSIZE);
	dprintf("REG_EPD_WRK0BF		= 0x%08x\n", REG_EPD_WRK0BF);
	dprintf("REG_EPD_WRK0SIZE	= 0x%08x\n", REG_EPD_WRK0SIZE);
	dprintf("REG_EPD_WRK1BF		= 0x%08x\n", REG_EPD_WRK1BF);
	dprintf("REG_EPD_WRK1SIZE	= 0x%08x\n", REG_EPD_WRK1SIZE);
	dprintf("REG_EPD_PRI		= 0x%08x\n", REG_EPD_PRI);
	dprintf("REG_EPD_VCOMBD0		= 0x%08x\n", REG_EPD_VCOMBD0);
	dprintf("REG_EPD_VCOMBD1		= 0x%08x\n", REG_EPD_VCOMBD1);
	dprintf("REG_EPD_VCOMBD2		= 0x%08x\n", REG_EPD_VCOMBD2);
	dprintf("REG_EPD_VCOMBD3		= 0x%08x\n", REG_EPD_VCOMBD3);
	dprintf("REG_EPD_VCOMBD4		= 0x%08x\n", REG_EPD_VCOMBD4);
	dprintf("REG_EPD_VCOMBD5		= 0x%08x\n", REG_EPD_VCOMBD5);
	dprintf("REG_EPD_VCOMBD6		= 0x%08x\n", REG_EPD_VCOMBD6);
	dprintf("REG_EPD_VCOMBD7		= 0x%08x\n", REG_EPD_VCOMBD7);
	dprintf("REG_EPD_VCOMBD8		= 0x%08x\n", REG_EPD_VCOMBD8);
	dprintf("REG_EPD_VCOMBD9		= 0x%08x\n", REG_EPD_VCOMBD9);
	dprintf("REG_EPD_VCOMBD10	= 0x%08x\n", REG_EPD_VCOMBD10);
	dprintf("REG_EPD_VCOMBD11	= 0x%08x\n", REG_EPD_VCOMBD11);
	dprintf("REG_EPD_VCOMBD12	= 0x%08x\n", REG_EPD_VCOMBD12);
	dprintf("REG_EPD_VCOMBD13	= 0x%08x\n", REG_EPD_VCOMBD13);
	dprintf("REG_EPD_VCOMBD14	= 0x%08x\n", REG_EPD_VCOMBD14);
	dprintf("REG_EPD_VCOMBD15	= 0x%08x\n", REG_EPD_VCOMBD15);
	dprintf("REG_DDR_SNP_HIGH	= 0x%08x\n", REG_DDR_SNP_HIGH);
	dprintf("REG_DDR_SNP_LOW		= 0x%08x\n", REG_DDR_SNP_LOW);
	dprintf("==================================\n");
	dprintf("reg:0x10000020 value=0x%08x  (24bit) Clock Gate Register0\n",
			*(unsigned int *)0xb0000020);
	dprintf("reg:0x10000028 value=0x%08x  (24bit) Clock Gate Register1\n",
			*(unsigned int *)0xb0000028);
	dprintf("reg:0x100000e4 value=0x%08x  (5bit_lcdc 21bit_lcdcs) Power Gate Register: \n",
			*(unsigned int *)0xb00000e4);
	dprintf("reg:0x100000b8 value=0x%08x  (10bit) SRAM Power Control Register0 \n",
			*(unsigned int *)0xb00000b8);
	dprintf("reg:0x10000064 value=0x%08x  Lcd pixclock \n",
			*(unsigned int *)0xb0000064);
	dprintf("==================================\n");
	dprintf("PCINT:\t0x%08x\n", *(unsigned int *)0xb0010210);
	dprintf("PCMASK:\t0x%08x\n",*(unsigned int *)0xb0010220);
	dprintf("PCPAT1:\t0x%08x\n",*(unsigned int *)0xb0010230);
	dprintf("PCPAT0:\t0x%08x\n",*(unsigned int *)0xb0010240);
	dprintf("==================================\n");

}




static int epd_wfm_lut_decompression(unsigned char *source_data,
		unsigned char *data, struct compression_lut *lut)
{
	unsigned int data_length = lut[0].number;
	unsigned int source_data_subscript = 0;
	unsigned int data_subscript = 0;
	unsigned int lut_subscript = 3;
	unsigned int temp_i = 0;

	while (data_subscript < data_length) {
		if (source_data_subscript == lut[lut_subscript].subscript) {
			for (temp_i = 0; temp_i < lut[lut_subscript].number + 1; temp_i++){
				data[data_subscript] = source_data[source_data_subscript];
				data_subscript++;
			}
			lut_subscript++;
		} else {
			data[data_subscript] = source_data[source_data_subscript];
			data_subscript++;
		}
		source_data_subscript++;
	}
	return 0;
}


static int epd_map_smem(unsigned int jz_epd_base)
{
	unsigned int size;
	unsigned int cur_buf_width	= epd_panel_info.epd_var.xres;
	unsigned int cur_buf_height	 =epd_panel_info.epd_var.yres;
	unsigned int cur_buf_bpp	= EPD_BPP;

	size = lcd_get_size(&line_length);
	size = (size + PAGE_SIZE + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1);

	frm_buf_size = size;
	cur_buf_size = cur_buf_width * cur_buf_height * cur_buf_bpp / 8;
	wrk0_buf_size	= cur_buf_size	* 2;				/* new + old pix buffer */
	wrk1_buf_size	= cur_buf_width	* cur_buf_height * 8 / 8;	/* pixcnt buffer */
	wfm_buf_size	= 1024 * 1024;


	frame_buffer = gd->fb_base;
	memset((char *)(frame_buffer | 0xA0000000),0,frm_buf_size);

	current_buffer = jz_epd_base;
	jz_epd_base += cur_buf_size;
	memset((char *)(current_buffer | 0xA0000000),0,cur_buf_size);

	jz_epd_base = (jz_epd_base + 4096) & ~0xfff;
	working_buffer = jz_epd_base;
	jz_epd_base +=wrk0_buf_size;
	memset((char *)(working_buffer | 0xA0000000),0,wrk0_buf_size);

	jz_epd_base = (jz_epd_base + 4096) & ~0xfff;
	pixcnt_buffer = jz_epd_base;
	jz_epd_base += wrk1_buf_size;
	memset((char *)(pixcnt_buffer | 0xA0000000),0,wrk1_buf_size);

	jz_epd_base = (jz_epd_base + 4096) & ~0xfff;
	waveform_buffer = jz_epd_base;
	jz_epd_base += wfm_buf_size;
	memset((char *)(waveform_buffer | 0xA0000000),0,wfm_buf_size);

	jz_epd_base = (jz_epd_base + 4096) & ~0xfff;
	wfm_lut_buf = jz_epd_base;
	jz_epd_base += WFM_LUT_SIZE;
	memset((char *)(wfm_lut_buf | 0xA0000000),0,WFM_LUT_SIZE);

	return 0;
}



static void jz4775fb_fill_wfm_lut(epd_mode_e epd_mode)
{
	int i;
	unsigned int pos = 512;

	unsigned char *waveform_addr;
	unsigned int waveform_addr_index, mode_addr_index;
    unsigned char * wfm_buf;   // = WAVEFORM_LUT;
    unsigned char * lut_buf = (unsigned char *)(wfm_lut_buf | 0xA0000000);
	unsigned char rvdata;
	unsigned char g0, g1, g2, g3, w;

	epd_wfm_lut_decompression(WAVEFORM_LUT,(unsigned char *)waveform_buffer,COMPRESSION_LUT);
	wfm_buf = (unsigned char *)waveform_buffer;

	if(wfm_buf[0] == 0xc5)		//From PVI
	{
		if(epd_mode == MODE_GC16GU)
			epd_mode = MODE_GC16;

		memset(lut_buf, 0x00, WFM_LUT_SIZE);

		waveform_addr_index =	(wfm_buf[pos / 2 + 64 * epd_mode + 4 * (epd_temp) + 2] << 16)	|
			(wfm_buf[pos / 2 + 64 * epd_mode + 4 * (epd_temp) + 1] << 8)	|
			(wfm_buf[pos / 2 + 64 * epd_mode + 4 * (epd_temp) + 0] << 0);

		waveform_addr = &wfm_buf[waveform_addr_index];

		frame_num = wfm_buf[pos / 2 + 64 * epd_mode + 4 * (epd_temp) + 3];

		dprintf("PVI epd_mode = %d, epd_temp = %d\t", epd_mode, epd_temp);
		dprintf("waveform_addr = 0x%x\n", (unsigned int)waveform_addr);
		dprintf("frame_num = %d\n", frame_num);

		memcpy(lut_buf,waveform_addr, SIZE_PER_FRAME * frame_num);
	}
	else if(wfm_buf[0] == 0xa2)		//From OED
	{
		epd_mode = 2;
		epd_temp = 8;
		waveform_addr_index = wfm_buf[pos + 0x30 + 5 * epd_temp + 1] << 24 |
			wfm_buf[pos + 0x30 + 5 * epd_temp + 2] << 16 |
			wfm_buf[pos + 0x30 + 5 * epd_temp + 3] << 8 |
			wfm_buf[pos + 0x30 + 5 * epd_temp + 4];
		frame_num = wfm_buf[pos + waveform_addr_index + 6 * epd_mode] << 8 | wfm_buf[pos + waveform_addr_index + 6 * epd_mode + 1];
		mode_addr_index = wfm_buf[pos + waveform_addr_index + 6 * epd_mode + 2] << 24 |
			wfm_buf[pos + waveform_addr_index + 6 * epd_mode + 3] << 16 |
			wfm_buf[pos + waveform_addr_index + 6 * epd_mode + 4] << 8 |
			wfm_buf[pos + waveform_addr_index + 6 * epd_mode + 5];
		waveform_addr = &wfm_buf[pos + waveform_addr_index + mode_addr_index];

		memcpy(lut_buf, waveform_addr, SIZE_PER_FRAME * frame_num);

		for(i = 0; i < SIZE_PER_FRAME * frame_num; i++)
		{
			w = lut_buf[i];
			g0 = (w >> 6) & 0x3;
			g1 = (w >> 4) & 0x3;
			g2 = (w >> 2) & 0x3;
			g3 = (w >> 0) & 0x3;

			rvdata =  (g3 << 6) | (g2 << 4) | (g1 << 2) | (g0 << 0);

			lut_buf[i] = rvdata;
		}

		dprintf("OED epd_mode = %d  epd_temp = %d\t", epd_mode, epd_temp);
		dprintf("mode_addr_index = 0x%x\n", mode_addr_index);
		dprintf("waveform_addr = 0x%p\n", waveform_addr);
		dprintf("max_frm = %d\n", frame_num);
	}
	else
	{
		printf("error:Not Ingenic waveform mode, check it with R&D\n");
	}
}




static int jz4775fb_load_wfm_lut(epd_mode_e epd_mode)
{
	WAIT_STATUS(REG_EPD_STA & EPD_STA_EPD_IDLE);

	REG_EPD_CFG &= 0x1f;
	REG_EPD_CFG |= EPD_CFG_LUT_1MODE;

	jz4775fb_fill_wfm_lut(epd_mode);

	REG_EPD_LUTBF	= wfm_lut_buf & 0x1fffffff;
	REG_EPD_LUTSIZE	= EPD_LUT_POS0 | (frame_num * 64 / 4);
	REG_EPD_CFG	|= (frame_num - 1) << EPD_CFG_STEP;
	REG_EPD_CTRL	|= EPD_CTRL_LUT_STRT ;

	WAIT_STATUS(REG_EPD_STA & EPD_STA_LUT_DONE);
	REG_EPD_STA &= ~EPD_STA_LUT_DONE;

	return 0;
}


static void jz4775fb_epd_controller_init(void)
{
	struct epd_timing etm = epd_panel_info.epd_tm;

	REG_EPD_VAT   =  etm.vt		<< EPD_VAT_VT		|	etm.ht		<< EPD_VAT_HT;
	REG_EPD_HSYNC =  etm.hpe	<< EPD_HSYNC_HPE	|	etm.hps		<< EPD_HSYNC_HPS;
	REG_EPD_SDSP  =	 etm.sdspd	<< EPD_SDSP_DIS		|	etm.sdspe	<< EPD_SDSP_ENA;
	REG_EPD_DAH   =  etm.hde	<< EPD_DAH_HDE		|	etm.hds		<< EPD_DAH_HDS;
	REG_EPD_SDOE  =  etm.sdoed	<< EPD_SDOE_DIS		|	etm.sdoee	<< EPD_SDOE_ENA;

	REG_EPD_GDCLK =  etm.gdclkd	<< EPD_GDCLK_DIS	|	etm.gdclke	<< EPD_GDCLK_ENA;
	REG_EPD_VSYNC =  etm.vpe	<< EPD_VSYNC_VPE	|	etm.vps		<< EPD_VSYNC_VPS;
	REG_EPD_GDSP  =  etm.gdspd	<< EPD_GDSP_DIS		|	etm.gdspe	<< EPD_GDSP_ENA;
	REG_EPD_DAV   =  etm.vde	<< EPD_DAV_VDE		|	etm.vds		<< EPD_DAV_VDS;
	REG_EPD_GDOE  =  etm.gdoed	<< EPD_GDOE_DIS		|	etm.gdoee	<< EPD_GDOE_ENA;

	REG_EPD_DIS0 = EPD_DIS0_SDSP_CAS | EPD_DIS0_GDCLK_MODE | EPD_DIS0_GDUD | EPD_DIS0_GDCLK_POL | EPD_DIS0_GDOE_POL | EPD_DIS0_SDCLK_POL |   EPD_DIS0_SDOE_POL | EPD_DIS0_SDLE_POL;
	REG_EPD_DIS1 = EPD_DIS1_SDDO_REV | (epd_panel_info.epd_var.xres / 4) << EPD_DIS1_SDOS | 1 << EPD_DIS1_SDCE_NUM;

	REG_EPD_CTRL |= EPD_CTRL_EPD_ENA;
	REG_EPD_CFG |= EPD_CFG_AUTO_GATED;

	jz4775fb_load_wfm_lut(MODE_GC16);
	REG_EPD_SIZE = epd_panel_info.epd_var.yres << EPD_SIZE_VSIZE | epd_panel_info.epd_var.xres << EPD_SIZE_HSIZE;
	REG_EPD_CURBF = current_buffer & 0x1fffffff;
	REG_EPD_CURSIZE	= cur_buf_size / 4;
	// if this is no value, then can be did
	REG_EPD_WRK0BF	= working_buffer & 0x1fffffff;
	REG_EPD_WRK0SIZE = wrk0_buf_size / 4;
	REG_EPD_WRK1BF	= pixcnt_buffer & 0x1fffffff;
	REG_EPD_WRK1SIZE = wrk1_buf_size / 4;
	REG_EPD_CFG	|= EPD_CFG_IBPP_4BITS;

	REG_EPD_STA = 0;  /*clear status*/
}


static void jz4775fb_start_epd_refresh(void)
{
	REG_EPD_CTRL |= 1 << 3;
	REG_EPD_CTRL |= 1 << 2;
}

static int jz4775fb_clear_screen(unsigned char value)
{
	jz4775fb_load_wfm_lut(MODE_GC16);
	memset((char *)(current_buffer | 0xA0000000), value, cur_buf_size);

	REG_EPD_CTRL |= EPD_CTRL_PWRON;
	WAIT_STATUS(REG_EPD_STA & EPD_STA_PWRON);
	REG_EPD_STA &= ~EPD_STA_PWRON;

	if(epd_power_ctrl.epd_power_on) {
		epd_power_ctrl.epd_power_on();
	}
	jz4775fb_start_epd_refresh();
	WAIT_STATUS(REG_EPD_STA & EPD_STA_REF_STOP);
	REG_EPD_STA &= ~EPD_STA_REF_STOP;

	REG_EPD_CTRL |= EPD_CTRL_PWROFF;
	if(epd_power_ctrl.epd_power_off) {
		epd_power_ctrl.epd_power_off();
	}
	return 0;
}

static inline unsigned int gc162gc4(unsigned int dat, unsigned int flag)
{
	unsigned int ret = 0;
	dat >>= (flag * 4);
	switch(dat){
		case 0 ... 2:
			ret = 0x0;
			break;
		case 3 ... 7:
			ret = 0x5;
			break;
		case 8 ... 12:
			ret = 0xa;
			break;
		case 13 ... 15:
			ret = 0xf;
			break;
	}
	return ret;
}


static void rgb565_to_gray(unsigned int frame_buffer, unsigned int current_buffer, epd_mode_e epdmode)
{
	int i, j;
	unsigned int bpp, w, h,tmp;
	unsigned int data0, data1;
	unsigned char r0, g0, b0, r1, g1, b1;
	unsigned char r2, g2, b2, r3, g3, b3;
	unsigned int y0, y1, y2, y3;

	unsigned int * lcd_frame = (unsigned int *)frame_buffer;
	unsigned char * epd_frame = (unsigned char *)(current_buffer | 0xA0000000);

	w = epd_panel_info.epd_var.xres;
	h = epd_panel_info.epd_var.yres;
	int write_region[4] = {0,0, w, h};

	printf("frame_buffer = %08x,current_buffer = %08x,epdmode = %d\n",frame_buffer,current_buffer,epdmode);
	bpp = epd_panel_info.epd_var.bits_per_pixel;
	if ( bpp == 18 || bpp == 24)
		bpp = 32;
	if ( bpp == 15 )
		bpp = 16;
	printf("bpp = %d\n",bpp);

	if (epdmode == MODE_A2) {
		for(i = write_region[1]; i < (write_region[1] + write_region[3]); i++) {
			for(j = write_region[0]; j < (write_region[0] + write_region[2]) * bpp / 8; j += bpp / 4) {

				r0 = (lcd_frame[i * w * bpp / 8 + j + 1] & 0x7c) << 1;
				g0 = (((lcd_frame[i * w * bpp / 8 + j + 1] & 0x3) << 3) | ((lcd_frame[i * w * bpp / 8 + j + 0] >> 5) & 0x7)) << 3;
				b0 = (lcd_frame[i * w * bpp / 8 + j + 0] & 0x1f) << 3;

				y0 = (299 * r0 + 587 * g0 + 114 * b0) / 1000;

				r1 = (lcd_frame[i * w * bpp / 8 + j + 3] & 0x7c) << 1;
				g1 = (((lcd_frame[i * w * bpp / 8 + j + 3] & 0x3) << 3) | ((lcd_frame[i * w * bpp / 8 + j + 2] >> 5) & 0x7)) << 3;
				b1 = (lcd_frame[i * w * bpp / 8 + j + 2] & 0x1f) << 3;

				y1 = (299 * r1 + 587 * g1 + 114 * b1) / 1000;

				y0 = ((y0 & 0xf) >= 0x8) ? ((y0 & 0xf0) + 0x10) : y0;
				y1 = ((y1 & 0xf) >= 0x8) ? ((y1 & 0xf0) + 0x10) : y1;

				y0 = (y0 < 0xff) ? y0 : 0xff;
				y1 = (y1 < 0xff) ? y1 : 0xff;

				y0 = (y0 < 0x7f) ? 0x00 : 0xff;
				y1 = (y1 < 0x7f) ? 0x00 : 0xff;

				epd_frame[(i * w * bpp / 8 + j) / 4] = (y0 >> 4) | (y1 & 0xf0);
			}
		}
	} else if (epdmode != 3) {
#if 1
		for(i = write_region[1]; i < (write_region[1] + write_region[3]); i++) {
			for(j = write_region[0]; j < (write_region[0] + write_region[2]); j++ ) {


				tmp = lcd_frame[i * w + j * 2];
				r0 = ((((tmp >> 19) & 0xf) << 1) | ((tmp >> 15) & 1 )) << 1 ;
				g0 = ((tmp >> 10) & 0x1f)  << 3;
				b0 = ((tmp >> 3) & 0x1f) << 3;
				y0 = (299 * r0 + 587 * g0 + 114 * b0) / 1000;

				tmp = lcd_frame[i * w + j * 2 + 1];
				r1 = ((((tmp >> 19) & 0xf) << 1) | ((tmp >> 15) & 1 )) << 1 ;
				g1 = ((tmp >> 10) & 0x1f)  << 3;
				b1 = ((tmp >> 3) & 0x1f) << 3;
				y1 = (299 * r1 + 587 * g1 + 114 * b1) / 1000;

				y0 = ((y0 & 0xf) >= 0x8) ? ((y0 & 0xf0) + 0x10) : y0;
				y1 = ((y1 & 0xf) >= 0x8) ? ((y1 & 0xf0) + 0x10) : y1;

				y0 = (y0 < 0xff) ? y0 : 0xff;
				y1 = (y1 < 0xff) ? y1 : 0xff;

				y0 = (y0 < 0x7f) ? 0x00 : 0xff;
				y1 = (y1 < 0x7f) ? 0x00 : 0xff;

				epd_frame[i * w * EPD_BPP / 8 + j] = (y0 >> 4) | (y1 & 0xf0);
			}
		}
#else
		for(i = write_region[1]; i < (write_region[1] + write_region[3]); i++){
			for(j = write_region[0]; j < (write_region[0] + write_region[2]) * bpp / 8; j += bpp / 2){

				r0 = lcd_frame[i * w * bpp / 8 + j + 1] & 0xf8;
				g0 = (((lcd_frame[i * w * bpp / 8 + j + 1] & 0x7) << 3) | ((lcd_frame[i * w * bpp / 8 + j + 0] >> 5) & 0x7)) << 2;
				b0 = (lcd_frame[i * w * bpp / 8 + j + 0] & 0x1f) << 3;

				y0 = (299 * r0 + 587 * g0 + 114 * b0) / 1000;

				r1 = lcd_frame[i * w * bpp / 8 + j + 3] & 0xf8;
				g1 = (((lcd_frame[i * w * bpp / 8 + j + 3] & 0x7) << 3) | ((lcd_frame[i * w * bpp / 8 + j + 2] >> 5) & 0x7)) << 2;
				b1 = (lcd_frame[i * w * bpp / 8 + j + 2] & 0x1f) << 3;

				y1 = (299 * r1 + 587 * g1 + 114 * b1) / 1000;

				y0 = ((y0 & 0xf) >= 0x8) ? ((y0 & 0xf0) + 0x10) : y0;
				y1 = ((y1 & 0xf) >= 0x8) ? ((y1 & 0xf0) + 0x10) : y1;

				y0 = (y0 < 0xff) ? y0 : 0xff;
				y1 = (y1 < 0xff) ? y1 : 0xff;

				r2 = lcd_frame[i * w * bpp / 8 + j + 5] & 0xf8;
				g2 = (((lcd_frame[i * w * bpp / 8 + j + 5] & 0x7) << 3) | ((lcd_frame[i * w * bpp / 8 + j + 4] >> 5) & 0x7)) << 2;
				b2 = (lcd_frame[i * w * bpp / 8 + j + 4] & 0x1f) << 3;

				y2 = (299 * r2 + 587 * g2 + 114 * b2) / 1000;

				r3 = lcd_frame[i * w * bpp / 8 + j + 7] & 0xf8;
				g3 = (((lcd_frame[i * w * bpp / 8 + j + 7] & 0x7) << 3) | ((lcd_frame[i * w * bpp / 8 + j + 6] >> 5) & 0x7)) << 2;
				b3 = (lcd_frame[i * w * bpp / 8 + j + 6] & 0x1f) << 3;

				y3 = (299 * r3 + 587 * g3 + 114 * b3) / 1000;

				y2 = ((y2 & 0xf) >= 0x8) ? ((y2 & 0xf0) + 0x10) : y2;
				y3 = ((y3 & 0xf) >= 0x8) ? ((y3 & 0xf0) + 0x10) : y3;

				y2 = (y2 < 0xff) ? y2 : 0xff;
				y3 = (y3 < 0xff) ? y3 : 0xff;

				epd_frame[(i * w * bpp / 8 + j) / 4] = (y0 >> 4) | (y1 & 0xf0);
				epd_frame[(i * w * bpp / 8 + j + 4) / 4] = (y2 >> 4) | (y3 & 0xf0);
			}
		}
#endif
	} else {
		for(i = write_region[1]; i < (write_region[1] + write_region[3]); i++) {
			for(j = write_region[0]; j < (write_region[0] + write_region[2]) * bpp / 8; j += bpp / 4) {
				data0 = (((lcd_frame[i * w * bpp / 8 + j + 1] >> 4) & 0xf) * 299
						+ (((lcd_frame[i * w * bpp / 8 + j + 1] & 0x7) << 1) |
							(((lcd_frame[i * w * bpp / 8 + j + 0] >> 7) & 0x1))) * 587
						+ ((lcd_frame[i * w * bpp / 8 + j + 0] >> 1) & 0xf) * 114) / 1000;
				data1 = (((lcd_frame[i * w * bpp / 8 + j + 3] >> 4) & 0xf) * 299
						+ (((lcd_frame[i * w * bpp / 8 + j + 3] & 0x7) << 1) |
							(((lcd_frame[i * w * bpp / 8 + j + 2] >> 7) & 0x1))) * 587
						+ ((lcd_frame[i * w * bpp / 8 + j + 2] >> 1) & 0xf) * 114) / 1000;
				data0 = (data0 < 0xf) ? data0 : 0xf;
				data1 = (data1 < 0xf) ? data1 : 0xf;

				data0 = gc162gc4(data0, 0);
				data1 = gc162gc4(data1, 0);
				epd_frame[(i * w * bpp / 8 + j) / 4] = (data0 << 4) | data1;
			}
		}
	}
}

#if defined(CONFIG_LCD_LOGO)

int sepd_convert_RGB_buffer_to_Y_buffer(unsigned short *src_buf, unsigned short *dst_buf)
{
	unsigned char *lcd_frame;
	unsigned char *epd_frame;
	int epdmode = MODE_GC16; // scshin ++-- sould be 16 gray
	lcd_frame = (unsigned char *)src_buf;
	epd_frame = (unsigned char *)dst_buf;

	rgb565_to_gray(lcd_frame, epd_frame, epdmode);

	return 0;
}



static void fbmem_set(void *_ptr, unsigned short val, unsigned count)
{
	int bpp = NBITS(panel_info.vl_bpix);
	if(bpp == 16){
		unsigned short *ptr = _ptr;

		while (count--)
			*ptr++ = val;
	} else if (bpp == 32){
		int val_32 = 0;
		int  rdata, gdata, bdata;
		unsigned int *ptr = (unsigned int *)_ptr;


		rdata = val >> 11;
		rdata = (rdata << 3) | 0x7;

		gdata = (val >> 5) & 0x003F;
		gdata = (gdata << 2) | 0x3;

		bdata = val & 0x001F;
		bdata = (bdata << 3) | 0x7;

		val_32 =  (rdata << 16) | (gdata << 8) | bdata;

		while (count--){
			*ptr++ = val_32;
		}
	}
}
/* 565RLE image format: [count(2 bytes), rle(2 bytes)] */
void rle_plot_biger(unsigned short *src_buf, unsigned short *dst_buf, int bpp)
{
	debug("rle_plot_biger\n");
	int vm_width, vm_height;
	int photo_width, photo_height, photo_size;
	int dis_width, dis_height;
	int flag_bit = (bpp == 16) ? 1 : 2;

	vm_width = panel_info.vl_col;
	vm_height = panel_info.vl_row;
	unsigned short *photo_ptr = (unsigned short *)src_buf;
	unsigned short *lcd_fb = (unsigned short *)dst_buf;

	photo_width = photo_ptr[0];
	photo_height = photo_ptr[1];
	photo_size = photo_ptr[3] << 16 | photo_ptr[2]; 	//photo size
	debug("photo_size =%d photo_width = %d, photo_height = %d\n", photo_size,
	      photo_width, photo_height);
	photo_ptr += 4;

	dis_height = photo_height < vm_height ? photo_height : vm_height;

	unsigned write_count = photo_ptr[0];
	while (photo_size > 0) {
			while (dis_height > 0) {
				dis_width = photo_width < vm_width ? photo_width : vm_width;
				while (dis_width > 0) {
					if (photo_size < 0)
						break;
					fbmem_set(lcd_fb, photo_ptr[1], write_count);
					lcd_fb += write_count * flag_bit;
					photo_ptr += 2;
					photo_size -= 2;
					write_count = photo_ptr[0];
					dis_width -= write_count;
				}
				if (dis_width < 0) {
					photo_ptr -= 2;
					photo_size += 2;
					lcd_fb += dis_width * flag_bit;
					write_count = - dis_width;
				}
				int extra_width = photo_width - vm_width;
				while (extra_width > 0) {
					photo_ptr += 2;
					photo_size -= 2;
					write_count = photo_ptr[0];
					extra_width -= write_count;
				}
				if (extra_width < 0) {
					photo_ptr -= 2;
					photo_size += 2;
					write_count = -extra_width;
				}
				dis_height -= 1;
			}
			if (dis_height <= 0)
				break;
		}
	return;
}
#ifdef CONFIG_LOGO_EXTEND
#ifndef  CONFIG_PANEL_DIV_LOGO
#define  CONFIG_PANEL_DIV_LOGO   (3)
#endif
void rle_plot_extend(unsigned short *src_buf, unsigned short *dst_buf, int bpp)
{
	debug("rle_plot_extend\n");
	int vm_width, vm_height;
	int photo_width, photo_height, photo_size;
	int dis_width, dis_height, ewidth, eheight;
	unsigned short compress_count, compress_val, write_count;
	unsigned short compress_tmp_count, compress_tmp_val;
	unsigned short *photo_tmp_ptr;
	unsigned short *photo_ptr;
	unsigned short *lcd_fb;
	int rate0, rate1, extend_rate;
	int i;

	int flag_bit = 1;
	if(bpp == 16){
		flag_bit = 1;
	}else if(bpp == 32){
		flag_bit = 2;
	}

	vm_width = panel_info.vl_col;
	vm_height = panel_info.vl_row;
	photo_ptr = (unsigned short *)src_buf;
	lcd_fb = (unsigned short *)dst_buf;

	rate0 =  vm_width/photo_ptr[0] > CONFIG_PANEL_DIV_LOGO ? (vm_width/photo_ptr[0])/CONFIG_PANEL_DIV_LOGO : 1;
	rate1 =  vm_height/photo_ptr[1] > CONFIG_PANEL_DIV_LOGO ? (vm_width/photo_ptr[1])/CONFIG_PANEL_DIV_LOGO : 1;
	extend_rate = rate0 < rate1 ?  rate0 : rate1;
	debug("logo extend rate = %d\n", extend_rate);

	photo_width = photo_ptr[0] * extend_rate;
	photo_height = photo_ptr[1] * extend_rate;
	photo_size = ( photo_ptr[3] << 16 | photo_ptr[2]) * extend_rate; 	//photo size
	debug("photo_size =%d photo_width = %d, photo_height = %d\n", photo_size,
	      photo_width, photo_height);
	photo_ptr += 4;

	int test;
	dis_height = photo_height < vm_height ? photo_height : vm_height;
	ewidth = (vm_width - photo_width)/2;
	eheight = (vm_height - photo_height)/2;
	compress_count = compress_tmp_count= photo_ptr[0] * extend_rate;
	compress_val = compress_tmp_val= photo_ptr[1];
	photo_tmp_ptr = photo_ptr;
	write_count = 0;
	debug("0> photo_ptr = %08x, photo_tmp_ptr = %08x\n", photo_ptr, photo_tmp_ptr);
	while (photo_size > 0) {
			if (eheight > 0) {
				lcd_fb += eheight * vm_width * flag_bit;
			}
			while (dis_height > 0) {
				for(i = 0; i < extend_rate && dis_height > 0; i++){
					debug("I am  here\n");
					photo_ptr = photo_tmp_ptr;
					debug("1> photo_ptr = %08x, photo_tmp_ptr = %08x\n", photo_ptr, photo_tmp_ptr);
					compress_count = compress_tmp_count;
					compress_val = compress_tmp_val;

					dis_width = photo_width < vm_width ? photo_width : vm_width;
					if (ewidth > 0) {
						lcd_fb += ewidth*flag_bit;
					}
					while (dis_width > 0) {
						if (photo_size < 0)
							break;
						write_count = compress_count;
						if (write_count > dis_width)
							write_count = dis_width;

						fbmem_set(lcd_fb, compress_val, write_count);
						lcd_fb += write_count * flag_bit;
						if (compress_count > write_count) {
							compress_count = compress_count - write_count;
						} else {
							photo_ptr += 2;
							photo_size -= 2;
							compress_count = photo_ptr[0] * extend_rate;
							compress_val = photo_ptr[1];
						}

						dis_width -= write_count;
					}

					if (ewidth > 0) {
						lcd_fb += ewidth * flag_bit;
					} else {
						int xwidth = -ewidth;
						while (xwidth > 0) {
							unsigned write_count = compress_count;

							if (write_count > xwidth)
								write_count = xwidth;

							if (compress_count > write_count) {
								compress_count = compress_count - write_count;
							} else {
								photo_ptr += 2;
								photo_size -= 2;
								compress_count = photo_ptr[0];
								compress_val = photo_ptr[1];
							}
							xwidth -= write_count;
						}

					}
					dis_height -= 1;
				}
				photo_tmp_ptr = photo_ptr;
				debug("2> photo_ptr = %08x, photo_tmp_ptr = %08x\n", photo_ptr, photo_tmp_ptr);
				compress_tmp_count = compress_count;
				compress_tmp_val = compress_val;
			}

			if (eheight > 0) {
				lcd_fb += eheight * vm_width *flag_bit;
			}
			if (dis_height <= 0)
				return;
		}
	return;
}
#endif

void rle_plot_smaller(unsigned short *src_buf, unsigned short *dst_buf, int bpp)
{
	debug("rle_plot_smaller\n");
	int vm_width, vm_height;
	int photo_width, photo_height, photo_size;
	int dis_width, dis_height, ewidth, eheight;
	unsigned short compress_count, compress_val, write_count;
	unsigned short *photo_ptr;
	unsigned short *lcd_fb;
	int flag_bit = 1;
	if(bpp == 16){
		flag_bit = 1;
	}else if(bpp == 32){
		flag_bit = 2;
	}

	vm_width = panel_info.vl_col;
	vm_height = panel_info.vl_row;
	photo_ptr = (unsigned short *)src_buf;
	lcd_fb = (unsigned short *)dst_buf;

	photo_width = photo_ptr[0];
	photo_height = photo_ptr[1];
	photo_size = photo_ptr[3] << 16 | photo_ptr[2]; 	//photo size
	debug("photo_size =%d photo_width = %d, photo_height = %d\n", photo_size,
	      photo_width, photo_height);
	photo_ptr += 4;

	dis_height = photo_height < vm_height ? photo_height : vm_height;
	ewidth = (vm_width - photo_width)/2;
	eheight = (vm_height - photo_height)/2;
	compress_count = photo_ptr[0];
	compress_val = photo_ptr[1];
	write_count = 0;
	while (photo_size > 0) {
			if (eheight > 0) {
				lcd_fb += eheight * vm_width * flag_bit;
			}
			while (dis_height > 0) {
				dis_width = photo_width < vm_width ? photo_width : vm_width;
				if (ewidth > 0) {
					lcd_fb += ewidth*flag_bit;
				}
				while (dis_width > 0) {
					if (photo_size < 0)
						break;
					write_count = compress_count;
					if (write_count > dis_width)
						write_count = dis_width;

					fbmem_set(lcd_fb, compress_val, write_count);
					lcd_fb += write_count * flag_bit;
					if (compress_count > write_count) {
						compress_count = compress_count - write_count;
					} else {
						photo_ptr += 2;
						photo_size -= 2;
						compress_count = photo_ptr[0];
						compress_val = photo_ptr[1];
					}

					dis_width -= write_count;
				}

				if (ewidth > 0) {
					lcd_fb += ewidth * flag_bit;
				} else {
					int xwidth = -ewidth;
					while (xwidth > 0) {
						unsigned write_count = compress_count;

						if (write_count > xwidth)
							write_count = xwidth;

						if (compress_count > write_count) {
							compress_count = compress_count - write_count;
						} else {
							photo_ptr += 2;
							photo_size -= 2;
							compress_count = photo_ptr[0];
							compress_val = photo_ptr[1];
						}
						xwidth -= write_count;
					}

				}
				dis_height -= 1;
			}

			if (eheight > 0) {
				lcd_fb += eheight * vm_width *flag_bit;
			}
			if (dis_height <= 0)
				return;
		}
	return;
}

void rle_plot(unsigned short *buf, unsigned char *dst_buf)
{
	debug("rle_plot\n");
	int vm_width, vm_height;
	int photo_width, photo_height, photo_size;
	int flag;
	int bpp;
	static int i = 0;
	debug("%s()\n", __func__);
	unsigned short *photo_ptr = (unsigned short *)buf;
	unsigned short *lcd_fb = (unsigned short *)dst_buf;
	bpp = NBITS(panel_info.vl_bpix);
	vm_width = panel_info.vl_col;
	vm_height = panel_info.vl_row;

	flag =  photo_ptr[0] * photo_ptr[1] - vm_width * vm_height;
	debug("photo size = %d, vm size = %d\n", photo_ptr[0] * photo_ptr[1], vm_width * vm_height);
	debug("flag = %d\n", flag);
	if(flag <= 0){
#ifdef CONFIG_LOGO_EXTEND
		rle_plot_extend(photo_ptr, lcd_fb, bpp);
#else
		rle_plot_smaller(photo_ptr, lcd_fb, bpp);
#endif
	}else if(flag > 0){
		rle_plot_biger(photo_ptr, lcd_fb, bpp);
	}

	debug("lcd_fb = %p\n",lcd_fb);
	sepd_convert_RGB_buffer_to_Y_buffer(lcd_fb, (unsigned short*)current_buffer);
	if (i > 1) return ; else i++;
	return;
}

#else
void rle_plot(unsigned short *buf, unsigned char *dst_buf)
{
	debug("second rle_plot\n");
}
#endif

void fb_fill(void *logo_addr, void *fb_addr, int count)
{
	static int global_refresh = 1;
	debug("logo_addr = %p, fb_addr = %p, +count = %p %p\n", logo_addr, fb_addr, logo_addr + count, fb_addr + count);
	sepd_convert_RGB_buffer_to_Y_buffer((unsigned short*)logo_addr, (unsigned short*)current_buffer);

	REG_EPD_CTRL |= EPD_CTRL_PWRON;
	WAIT_STATUS(REG_EPD_STA & EPD_STA_PWRON);
	REG_EPD_STA &= ~EPD_STA_PWRON;

	if(epd_power_ctrl.epd_power_on) {
		epd_power_ctrl.epd_power_on();
	}

	jz4775fb_start_epd_refresh();
	WAIT_STATUS(REG_EPD_STA & EPD_STA_REF_STOP);
	REG_EPD_STA &= ~EPD_STA_REF_STOP;
	if(epd_power_ctrl.epd_power_off) {
		epd_power_ctrl.epd_power_off();
	}

}
void  lcd_enable(void)
{
	jz4775fb_load_wfm_lut(MODE_GC16);
	frame_buffer = gd->fb_base;
	rgb565_to_gray(frame_buffer,current_buffer,MODE_GC16);

	REG_EPD_CTRL |= EPD_CTRL_PWRON;
	WAIT_STATUS(REG_EPD_STA & EPD_STA_PWRON);
	REG_EPD_STA &= ~EPD_STA_PWRON;

	if(epd_power_ctrl.epd_power_on) {
		epd_power_ctrl.epd_power_on();
	}

	jz4775fb_start_epd_refresh();
	WAIT_STATUS(REG_EPD_STA & EPD_STA_REF_STOP);
	REG_EPD_STA &= ~EPD_STA_REF_STOP;
	if(epd_power_ctrl.epd_power_off) {
		epd_power_ctrl.epd_power_off();
	}
}

void lcd_ctrl_init(void * lcd_base)
{
	unsigned int pixel_clock_rate;

	board_set_lcd_power_on();
	if(epd_power_ctrl.epd_power_init){
		epd_power_ctrl.epd_power_init();
	}

	epd_panel_info = epd_panel_info_init();

	epd_map_smem(jz_epd_base);

	pixel_clock_rate = PICOS2KHZ(epd_panel_info.epd_var.pixclock) * 1000;
	clk_set_rate(LCD, pixel_clock_rate);

	jz4775fb_epd_controller_init();
    print_epdc_registers();

//	jz4775fb_clear_screen(0xff);  /* just for test*/
}
