/*
 * kernel/drivers/video/jz4775_android_epd.h
 *
 * Copyright (C) 2005-2013, Ingenic Semiconductor Inc.
 * http://www.ingenic.cn/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef __JZ47xx_EPD_H__
#define __JZ47xx_EPD_H__

#include <common.h>
#include <linux/types.h>
#ifdef CONFIG_VIDEO_ED060XD1
#include "lcd-ED060XD1.h"
#endif


#define NUM_FRAME_BUFFERS 1

#define PIXEL_ALIGN 16
#define MODE_NAME_LEN 32

#define PICOS2KHZ(a) (1000000000/(a))
#define KHZ2PICOS(a) (1000000000/(a))
#define FB_SYNC_HOR_HIGH_ACT    1   /* horizontal sync high active  */
#define FB_SYNC_VERT_HIGH_ACT   2   /* vertical sync high active    */
#define FB_SYNC_EXT		4   /* external sync        */
#define FB_SYNC_COMP_HIGH_ACT   8   /* composite sync high active   */
#define FB_SYNC_BROADCAST	16  /* broadcast video timings      */
/* vtotal = 144d/288n/576i => PAL  */
/* vtotal = 121d/242n/484i => NTSC */
#define FB_SYNC_ON_GREEN	32  /* sync on green */

#define FB_VMODE_NONINTERLACED  0   /* non interlaced */
#define FB_VMODE_INTERLACED	1   /* interlaced	*/
#define FB_VMODE_DOUBLE		2   /* double scan */
#define FB_VMODE_ODD_FLD_FIRST	4   /* interlaced: top line first */
#define FB_VMODE_MASK		255
typedef enum {
	MODE_INIT,
	MODE_DU,
	MODE_GC16,
	MODE_EMPTY,
	MODE_A2,
	MODE_GL16,
	MODE_A2IN,
	MODE_A2OUT,
	MODE_GC16GU,
	MODE_AUTO_DU,
	MODE_AUTO_DUGC4,
}epd_mode_e;

typedef enum{
	PARALLEL_REF,
	PARTIAL_REF
}ref_mode_e;

struct compression_lut{
	unsigned int subscript;
	unsigned int number;
};

struct fb_bitfield {
	__u32 offset;			/* beginning of bitfield	*/
	__u32 length;			/* length of bitfield		*/
	__u32 msb_right;

};

struct fb_var_screeninfo {
	__u32 xres;			/* visible resolution		*/
	__u32 yres;
	__u32 xres_virtual;		/* virtual resolution		*/
	__u32 yres_virtual;
	__u32 xoffset;			/* offset from virtual to visible */
	__u32 yoffset;			/* resolution			*/

	__u32 bits_per_pixel;		/* guess what			*/
	__u32 grayscale;		/* != 0 Graylevels instead of colors */

	struct fb_bitfield red;		/* bitfield in fb mem if true color, */
	struct fb_bitfield green;	/* else only length is significant */
	struct fb_bitfield blue;
	struct fb_bitfield transp;	/* transparency			*/

	__u32 nonstd;			/* != 0 Non standard pixel format */

	__u32 activate;			/* see FB_ACTIVATE_*		*/

	__u32 height;			/* height of picture in mm    */
	__u32 width;			/* width of picture in mm     */

	__u32 accel_flags;		/* (OBSOLETE) see fb_info.flags */

	/* Timing: All values in pixclocks, except pixclock (of course) */
	__u32 pixclock;			/* pixel clock in ps (pico seconds) */
	__u32 left_margin;		/* time from sync to picture	*/
	__u32 right_margin;		/* time from picture to sync	*/
	__u32 upper_margin;		/* time from sync to picture	*/
	__u32 lower_margin;
	__u32 hsync_len;		/* length of horizontal sync	*/
	__u32 vsync_len;		/* length of vertical sync	*/
	__u32 sync;			/* see FB_SYNC_*		*/
	__u32 vmode;			/* see FB_VMODE_*		*/
	__u32 rotate;			/* angle we rotate counter clockwise */
	__u32 reserved[5];		/* Reserved for future compatibility */
};

struct fb_fix_screeninfo {
	char id[16];			/* identification string eg "TT Builtin" */
	unsigned long smem_start;	/* Start of frame buffer mem */
					/* (physical address) */
	__u32 smem_len;			/* Length of frame buffer mem */
	__u32 type;			/* see FB_TYPE_*		*/
	__u32 type_aux;			/* Interleave for interleaved Planes */
	__u32 visual;			/* see FB_VISUAL_*		*/
	__u16 xpanstep;			/* zero if no hardware panning	*/
	__u16 ypanstep;			/* zero if no hardware panning	*/
	__u16 ywrapstep;		/* zero if no hardware ywrap	*/
	__u32 line_length;		/* length of a line in bytes	*/
	unsigned long mmio_start;	/* Start of Memory Mapped I/O	*/
					/* (physical address) */
	__u32 mmio_len;			/* Length of Memory Mapped I/O	*/
	__u32 accel;			/* Indicate to driver which	*/
					/*  specific chip/card we have	*/
	__u16 reserved[3];		/* Reserved for future compatibility */
};

struct epd_config_info {
	int node;
	int flags;
	struct fb_var_screeninfo var;	/* Current var */
	struct fb_fix_screeninfo fix;	/* Current fix */
	struct fb_videomode *mode;	/* current mode */

	char *screen_base;	/* Virtual address */
	unsigned long screen_size;	/* Amount of ioremapped VRAM or 0 */
	void *pseudo_palette;		/* Fake palette of 16 colors */
#define FBINFO_STATE_RUNNING	0
#define FBINFO_STATE_SUSPENDED	1
	__u32 state;			/* Hardware state i.e suspend */
	void *fbcon_par;		/* fbcon use-only private area */
	/* From here on everything is device dependent */
	void *par;
};

#endif /* __JZ47xx_EPD_H__ */
