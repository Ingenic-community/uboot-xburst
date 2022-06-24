#ifndef _USB_VIDEO_H_
#define _USB_VIDEO_H_

#ifndef __KERNEL__
#error "The uvcvideo.h header is deprecated, use linux/uvcvideo.h instead."
#endif /* __KERNEL__ */

#include <malloc.h>
//#include <linux/usb.h>
//#include <linux/usb/video.h>

/* --------------------------------------------------------------------------
 * UVC constants
 */

#define UVC_TERM_INPUT			0x0000
#define UVC_TERM_OUTPUT			0x8000
#define UVC_TERM_DIRECTION(term)	((term)->type & 0x8000)

#define UVC_ENTITY_TYPE(entity)		((entity)->type & 0x7fff)
#define UVC_ENTITY_IS_UNIT(entity)	(((entity)->type & 0xff00) == 0)
#define UVC_ENTITY_IS_TERM(entity)	(((entity)->type & 0xff00) != 0)
#define UVC_ENTITY_IS_ITERM(entity) \
	(UVC_ENTITY_IS_TERM(entity) && \
	((entity)->type & 0x8000) == UVC_TERM_INPUT)
#define UVC_ENTITY_IS_OTERM(entity) \
	(UVC_ENTITY_IS_TERM(entity) && \
	((entity)->type & 0x8000) == UVC_TERM_OUTPUT)


/* ------------------------------------------------------------------------
 * GUIDs
 */
#define UVC_GUID_UVC_CAMERA \
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01}
#define UVC_GUID_UVC_OUTPUT \
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02}
#define UVC_GUID_UVC_MEDIA_TRANSPORT_INPUT \
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03}
#define UVC_GUID_UVC_PROCESSING \
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01}
#define UVC_GUID_UVC_SELECTOR \
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02}

#define UVC_GUID_FORMAT_MJPEG \
	{ 'M',  'J',  'P',  'G', 0x00, 0x00, 0x10, 0x00, \
	 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}
#define UVC_GUID_FORMAT_YUY2 \
	{ 'Y',  'U',  'Y',  '2', 0x00, 0x00, 0x10, 0x00, \
	 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}
#define UVC_GUID_FORMAT_YUY2_ISIGHT \
	{ 'Y',  'U',  'Y',  '2', 0x00, 0x00, 0x10, 0x00, \
	 0x80, 0x00, 0x00, 0x00, 0x00, 0x38, 0x9b, 0x71}
#define UVC_GUID_FORMAT_NV12 \
	{ 'N',  'V',  '1',  '2', 0x00, 0x00, 0x10, 0x00, \
	 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}
#define UVC_GUID_FORMAT_YV12 \
	{ 'Y',  'V',  '1',  '2', 0x00, 0x00, 0x10, 0x00, \
	 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}
#define UVC_GUID_FORMAT_I420 \
	{ 'I',  '4',  '2',  '0', 0x00, 0x00, 0x10, 0x00, \
	 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}
#define UVC_GUID_FORMAT_UYVY \
	{ 'U',  'Y',  'V',  'Y', 0x00, 0x00, 0x10, 0x00, \
	 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}
#define UVC_GUID_FORMAT_Y800 \
	{ 'Y',  '8',  '0',  '0', 0x00, 0x00, 0x10, 0x00, \
	 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}
#define UVC_GUID_FORMAT_Y8 \
	{ 'Y',  '8',  ' ',  ' ', 0x00, 0x00, 0x10, 0x00, \
	 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}
#define UVC_GUID_FORMAT_Y10 \
	{ 'Y',  '1',  '0',  ' ', 0x00, 0x00, 0x10, 0x00, \
	 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}
#define UVC_GUID_FORMAT_Y12 \
	{ 'Y',  '1',  '2',  ' ', 0x00, 0x00, 0x10, 0x00, \
	 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}
#define UVC_GUID_FORMAT_Y16 \
	{ 'Y',  '1',  '6',  ' ', 0x00, 0x00, 0x10, 0x00, \
	 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}
#define UVC_GUID_FORMAT_BY8 \
	{ 'B',  'Y',  '8',  ' ', 0x00, 0x00, 0x10, 0x00, \
	 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}
#define UVC_GUID_FORMAT_RGBP \
	{ 'R',  'G',  'B',  'P', 0x00, 0x00, 0x10, 0x00, \
	 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}
#define UVC_GUID_FORMAT_M420 \
	{ 'M',  '4',  '2',  '0', 0x00, 0x00, 0x10, 0x00, \
	 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}

#define UVC_GUID_FORMAT_H264 \
	{ 'H',  '2',  '6',  '4', 0x00, 0x00, 0x10, 0x00, \
	 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}

/* ------------------------------------------------------------------------
 * Driver specific constants.
 */

#define DRIVER_VERSION		"1.1.1"

/* Number of isochronous URBs. */
#define UVC_URBS		5
/* Maximum number of packets per URB. */
#define UVC_MAX_PACKETS		32
/* Maximum number of video buffers. */
#define UVC_MAX_VIDEO_BUFFERS	32
/* Maximum status buffer size in bytes of interrupt URB. */
#define UVC_MAX_STATUS_SIZE	16

#define UVC_CTRL_CONTROL_TIMEOUT	300
#define UVC_CTRL_STREAMING_TIMEOUT	5000

/* Maximum allowed number of control mappings per device */
#define UVC_MAX_CONTROL_MAPPINGS	1024
#define UVC_MAX_CONTROL_MENU_ENTRIES	32

/* Devices quirks */
#define UVC_QUIRK_STATUS_INTERVAL	0x00000001
#define UVC_QUIRK_PROBE_MINMAX		0x00000002
#define UVC_QUIRK_PROBE_EXTRAFIELDS	0x00000004
#define UVC_QUIRK_BUILTIN_ISIGHT	0x00000008
#define UVC_QUIRK_STREAM_NO_FID		0x00000010
#define UVC_QUIRK_IGNORE_SELECTOR_UNIT	0x00000020
#define UVC_QUIRK_FIX_BANDWIDTH		0x00000080
#define UVC_QUIRK_PROBE_DEF		0x00000100
#define UVC_QUIRK_RESTRICT_FRAME_RATE	0x00000200

/* Format flags */
#define UVC_FMT_FLAG_COMPRESSED		0x00000001
#define UVC_FMT_FLAG_STREAM		0x00000002

enum {
	BUF_TYPE_VIDEO_CAPTURE = 1,
	BUF_TYPE_VIDEO_OUTPUT = 2
};

struct uvc_entity {
	struct list_head list;	/* Entity as part of a UVC device */

	unsigned int flags;
	__u8 id;
	__u16 type;
	char name[64];

	union {
		struct {
			__u16 wObjectiveFocalLengthMin;
			__u16 wObjectiveFocalLengthMax;
			__u16 wOcularFocalLength;
			__u8  bControlSize;
			__u8  *bmControls;
		} camera;

		struct {
			__u8  bControlSize;
			__u8  *bmControls;
			__u8  bTransportModeSize;
			__u8  *bmTransportModes;
		} media;

		struct {
		} output;

		struct {
			__u16 wMaxMultiplier;
			__u8  bControlSize;
			__u8  *bmControls;
			__u8  bmVideoStandards;
		} processing;

		struct {
		} selector;

		struct {
			__u8  guidExtensionCode[16];
			__u8  bNumControls;
			__u8  bControlSize;
			__u8  *bmControls;
			__u8  *bmControlsType;
		} extension;
	};

#if 0
	__u8 bNrInPins;
	__u8 *baSourceID;
#endif
	//unsigned int ncontrols;
	//struct uvc_control *controls;
};

struct uvc_format_desc {
        char *name;
        __u8 guid[16];
        __u32 fcc;
};

struct uvc_frame {
        __u8  bFrameIndex;
        __u8  bmCapabilities;
        __u16 wWidth;
        __u16 wHeight;
        __u32 dwMinBitRate;
        __u32 dwMaxBitRate;
        __u32 dwMaxVideoFrameBufferSize;
        __u8  bFrameIntervalType;
        __u32 dwDefaultFrameInterval;
        __u32 *dwFrameInterval;
};


struct uvc_format {
        __u8 type;
        __u8 index;
        __u8 bpp;
        __u8 colorspace;
        __u32 fcc;
        __u32 flags;

        char name[32];

        unsigned int nframes;
        struct uvc_frame *frame;
};

struct uvc_streaming_header {
        __u8 bNumFormats;
        __u8 bEndpointAddress;
        __u8 bTerminalLink;
        __u8 bControlSize;
        __u8 *bmaControls;
        /* The following fields are used by input headers only. */
        __u8 bmInfo;
        __u8 bStillCaptureMethod;
        __u8 bTriggerSupport;
        __u8 bTriggerUsage;
};

struct uvc_buffer {
	char *buffer;
	unsigned int offset;
	unsigned int act_len;
	int error;
};

struct uvc_streaming {
	struct list_head list;
	struct uvc_device *dev;


	int type;
	struct usb_interface_2 *intf;
	int intfnum;
	__u16 maxpsize;

	struct uvc_streaming_header header;

	unsigned int nformats;
	struct uvc_format *format;

	struct uvc_streaming_control ctrl;
	struct uvc_format *def_format;
	struct uvc_format *cur_format;
	struct uvc_frame *cur_frame;

	struct usb_host_endpoint *ep;	/*current using ep*/

#if 0
	unsigned int frozen : 1;
	struct uvc_video_queue queue;
#endif
	void (*decode) (struct uvc_streaming *video,
			unsigned char *buf, unsigned int len);

	/*
		one usb request buffer;
	*/
	unsigned char *request_buffer;
	unsigned int request_size;

	#define NR_BUFFERS	(3)
	struct uvc_buffer video_buffer[NR_BUFFERS];
	int next_buffer;

	__u32 sequence;
	__u8 last_fid;

	/* Context data used by the bulk completion handler. */
	struct {
		__u8 header[256];
		unsigned int header_size;
		int skip_payload;
		__u32 payload_size;
		__u32 max_payload_size;
	} bulk;

};

struct uvc_device {
	struct usb_device *udev;
	struct usb_interface_2 *intf;
	int intfnum;
	__u32 quirks;
	char name[32];

	__u16 uvc_version;
	__u32 clock_frequency;

	struct list_head entities;

	struct list_head streams;
	int nstreams;

	struct uvc_streaming *video;	/*current video stream.*/

	struct usb_host_endpoint *int_ep;
};

static inline void *uvc_malloc(int size)
{
	void *p = malloc(size);
	if(p == NULL) {
		printf("uvc_malloc failed!!\n");
	}
	return p;
}

static inline void uvc_free(void *m)
{
	free(m);
}


/* Exported video interface */
extern int uvc_video_probe(struct uvc_device *dev, struct uvc_streaming *stream);

extern struct usb_host_endpoint *uvc_find_endpoint(struct usb_host_interface *alts, __u8 epaddr);

struct uvc_device *uvc_get_device(int dev_num);
extern int uvc_video_enable(struct uvc_streaming *stream, int enable);
int uvc_usb_request_data(struct uvc_streaming *stream, void (*complete)(struct uvc_streaming *stream, struct uvc_buffer *uvc_buffer));

/* ------------------------------------------------------------------------
 * Debugging, printing and logging
 */

#define UVC_TRACE_PROBE         (1 << 0)
#define UVC_TRACE_DESCR         (1 << 1)
#define UVC_TRACE_CONTROL       (1 << 2)
#define UVC_TRACE_FORMAT        (1 << 3)
#define UVC_TRACE_CAPTURE       (1 << 4)
#define UVC_TRACE_CALLS         (1 << 5)
#define UVC_TRACE_IOCTL         (1 << 6)
#define UVC_TRACE_FRAME         (1 << 7)
#define UVC_TRACE_SUSPEND       (1 << 8)
#define UVC_TRACE_STATUS        (1 << 9)
#define UVC_TRACE_VIDEO         (1 << 10)
#define UVC_TRACE_STATS         (1 << 11)
#define UVC_TRACE_CLOCK         (1 << 12)


#define UVC_WARN_MINMAX         0
#define UVC_WARN_PROBE_DEF      1
#define UVC_WARN_XU_GET_RES     2

extern unsigned int uvc_clock_param;
extern unsigned int uvc_no_drop_param;
extern unsigned int uvc_trace_param;
extern unsigned int uvc_timeout_param;

#define uvc_trace(flag, msg...) \
        do { \
                if (uvc_trace_param & flag) \
                        debug("uvcvideo: " msg); \
        } while (0)

#define uvc_warn_once(dev, warn, msg...) \
        do { \
                        debug("uvcvideo: " msg); \
        } while (0)


#define uvc_printk(level, msg...) \
        debug("uvcvideo: " msg)

extern void dump_uvc_streaming_ctrl(struct uvc_streaming_control *ctrl);
extern void dump_uvc_streaming_format(struct uvc_streaming *stream);
extern void dump_uvc_streaming(struct uvc_streaming *stream);

#endif
