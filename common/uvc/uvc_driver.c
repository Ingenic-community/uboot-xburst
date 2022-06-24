#include <common.h>
#include <errno.h>

#include <linux/list.h>
#include <linux/usb/video.h>
#include <usb.h>

#include "uvcvideo.h"
#include "videodev2.h"

#define USB_MAX_VIDEO_DEVICE 5
static int usb_max_devs;

unsigned int uvc_clock_param;
unsigned int uvc_no_drop_param;
unsigned int uvc_trace_param = 0xffff;
unsigned int uvc_timeout_param = 100 * 5;

struct uvc_device uvc_dev[USB_MAX_VIDEO_DEVICE];


static struct usb_interface_2 *usb_ifnum_to_if(struct usb_device *dev,
					unsigned ifnum)
{

	return &dev->config.if_desc2[ifnum];
}

struct usb_host_endpoint *uvc_find_endpoint(struct usb_host_interface *alts,
                __u8 epaddr)
{
        struct usb_host_endpoint *ep;
        unsigned int i;

        for (i = 0; i < alts->desc.bNumEndpoints; ++i) {
                ep = &alts->endpoint[i];
                if (ep->desc.bEndpointAddress == epaddr)
                        return ep;
        }

        return NULL;
}

static struct uvc_streaming *uvc_stream_by_id(struct uvc_device *dev, int id)
{
        struct uvc_streaming *stream;

        list_for_each_entry(stream, &dev->streams, list) {
                if (stream->header.bTerminalLink == id)
                        return stream;
        }

        return NULL;
}


static struct uvc_format_desc uvc_fmts[] = {
        {
                .name           = "YUV 4:2:2 (YUYV)",
                .guid           = UVC_GUID_FORMAT_YUY2,
                .fcc            = V4L2_PIX_FMT_YUYV,
        },
        {
                .name           = "YUV 4:2:2 (YUYV)",
                .guid           = UVC_GUID_FORMAT_YUY2_ISIGHT,
                .fcc            = V4L2_PIX_FMT_YUYV,
        },
        {
                .name           = "YUV 4:2:0 (NV12)",
                .guid           = UVC_GUID_FORMAT_NV12,
                .fcc            = V4L2_PIX_FMT_NV12,
        },
        {
                .name           = "MJPEG",
                .guid           = UVC_GUID_FORMAT_MJPEG,
                .fcc            = V4L2_PIX_FMT_MJPEG,
        },
        {
                .name           = "YVU 4:2:0 (YV12)",
                .guid           = UVC_GUID_FORMAT_YV12,
                .fcc            = V4L2_PIX_FMT_YVU420,
        },
        {
                .name           = "YUV 4:2:0 (I420)",
                .guid           = UVC_GUID_FORMAT_I420,
                .fcc            = V4L2_PIX_FMT_YUV420,
	},
	{
		.name           = "YUV 4:2:0 (M420)",
		.guid           = UVC_GUID_FORMAT_M420,
		.fcc            = V4L2_PIX_FMT_M420,
	},
	{
		.name           = "YUV 4:2:2 (UYVY)",
		.guid           = UVC_GUID_FORMAT_UYVY,
		.fcc            = V4L2_PIX_FMT_UYVY,
	},
	{
		.name           = "Greyscale 8-bit (Y800)",
		.guid           = UVC_GUID_FORMAT_Y800,
		.fcc            = V4L2_PIX_FMT_GREY,
	},
	{
		.name           = "Greyscale 8-bit (Y8  )",
		.guid           = UVC_GUID_FORMAT_Y8,
		.fcc            = V4L2_PIX_FMT_GREY,
	},
	{
		.name           = "Greyscale 10-bit (Y10 )",
		.guid           = UVC_GUID_FORMAT_Y10,
		.fcc            = V4L2_PIX_FMT_Y10,
	},
	{
		.name           = "Greyscale 12-bit (Y12 )",
		.guid           = UVC_GUID_FORMAT_Y12,
		.fcc            = V4L2_PIX_FMT_Y12,
	},
	{
		.name           = "Greyscale 16-bit (Y16 )",
		.guid           = UVC_GUID_FORMAT_Y16,
		.fcc            = V4L2_PIX_FMT_Y16,
	},
	{
		.name           = "RGB Bayer",
		.guid           = UVC_GUID_FORMAT_BY8,
		.fcc            = V4L2_PIX_FMT_SBGGR8,
	},
	{
		.name           = "RGB565",
		.guid           = UVC_GUID_FORMAT_RGBP,
		.fcc            = V4L2_PIX_FMT_RGB565,
	},
	{
		.name           = "H.264",
		.guid           = UVC_GUID_FORMAT_H264,
		.fcc            = V4L2_PIX_FMT_H264,
	},
};

static struct uvc_format_desc *uvc_format_by_guid(const __u8 guid[16])
{
        unsigned int len = ARRAY_SIZE(uvc_fmts);
        unsigned int i;

        for (i = 0; i < len; ++i) {
                if (memcmp(guid, uvc_fmts[i].guid, 16) == 0)
                        return &uvc_fmts[i];
        }

        return NULL;
}



static int uvc_parse_format(struct uvc_device *dev,
        struct uvc_streaming *streaming, struct uvc_format *format,
        __u32 **intervals, unsigned char *buffer, int buflen)
{
        struct usb_interface_2 *intf = streaming->intf;
        struct usb_host_interface *alts = &intf->alts[intf->act_altsetting];
        struct uvc_format_desc *fmtdesc;
        struct uvc_frame *frame;
        const unsigned char *start = buffer;
        unsigned int interval;
        unsigned int i, n;
        __u8 ftype;

        format->type = buffer[2];
        format->index = buffer[3];

        switch (buffer[2]) {
        case UVC_VS_FORMAT_UNCOMPRESSED:
        case UVC_VS_FORMAT_FRAME_BASED:
                n = buffer[2] == UVC_VS_FORMAT_UNCOMPRESSED ? 27 : 28;
                if (buflen < n) {
                        uvc_trace(UVC_TRACE_DESCR, "device %d videostreaming "
                               "interface %d FORMAT error\n",
                               dev->udev->devnum,
                               alts->desc.bInterfaceNumber);
                        return -EINVAL;
                }

		/* Find the format descriptor from its GUID. */
		fmtdesc = uvc_format_by_guid(&buffer[5]);

		if (fmtdesc != NULL) {
			memcpy(format->name, fmtdesc->name,
					sizeof format->name);
			format->fcc = fmtdesc->fcc;
		} else {
			uvc_printk(KERN_INFO, "Unknown video format %pUl\n",
					&buffer[5]);
			snprintf(format->name, sizeof(format->name), "%pUl\n",
					&buffer[5]);
			format->fcc = 0;
		}

		format->bpp = buffer[21];
		if (buffer[2] == UVC_VS_FORMAT_UNCOMPRESSED) {
			ftype = UVC_VS_FRAME_UNCOMPRESSED;
		} else {
			ftype = UVC_VS_FRAME_FRAME_BASED;
			if (buffer[27])
				format->flags = UVC_FMT_FLAG_COMPRESSED;
		}
		break;

	case UVC_VS_FORMAT_MJPEG:
		if (buflen < 11) {
			uvc_trace(UVC_TRACE_DESCR, "device %d videostreaming "
					"interface %d FORMAT error\n",
					dev->udev->devnum,
					alts->desc.bInterfaceNumber);
			return -EINVAL;
		}

		memcpy(format->name, "MJPEG", sizeof format->name);
		format->fcc = V4L2_PIX_FMT_MJPEG;
		format->flags = UVC_FMT_FLAG_COMPRESSED;
		format->bpp = 0;
		ftype = UVC_VS_FRAME_MJPEG;
		break;

	case UVC_VS_FORMAT_DV:
		if (buflen < 9) {
			uvc_trace(UVC_TRACE_DESCR, "device %d videostreaming "
					"interface %d FORMAT error\n",
					dev->udev->devnum,
					alts->desc.bInterfaceNumber);
			return -EINVAL;
		}

		switch (buffer[8] & 0x7f) {
			case 0:
				memcpy(format->name, "SD-DV", sizeof format->name);
				break;
			case 1:
				memcpy(format->name, "SDL-DV", sizeof format->name);
				break;
			case 2:
				memcpy(format->name, "HD-DV", sizeof format->name);
				break;
			default:
				uvc_trace(UVC_TRACE_DESCR, "device %d videostreaming "
						"interface %d: unknown DV format %u\n",
						dev->udev->devnum,
						alts->desc.bInterfaceNumber, buffer[8]);
				return -EINVAL;
		}

		sprintf(format->name, buffer[8] & (1 << 7) ? "%s 60Hz" : "%s 50Hz",
				format->name);

		format->fcc = V4L2_PIX_FMT_DV;
		format->flags = UVC_FMT_FLAG_COMPRESSED | UVC_FMT_FLAG_STREAM;
		format->bpp = 0;
		ftype = 0;

		/* Create a dummy frame descriptor. */
		frame = &format->frame[0];
		memset(&format->frame[0], 0, sizeof format->frame[0]);
		frame->bFrameIntervalType = 1;
		frame->dwDefaultFrameInterval = 1;
		frame->dwFrameInterval = *intervals;
		*(*intervals)++ = 1;
		format->nframes = 1;
		break;

	case UVC_VS_FORMAT_MPEG2TS:
	case UVC_VS_FORMAT_STREAM_BASED:
		/* Not supported yet. */
	default:
		uvc_trace(UVC_TRACE_DESCR, "device %d videostreaming "
				"interface %d unsupported format %u\n",
				dev->udev->devnum, alts->desc.bInterfaceNumber,
				buffer[2]);
		return -EINVAL;
	}

	uvc_trace(UVC_TRACE_DESCR, "Found format %s.\n", format->name);

	buflen -= buffer[0];
	buffer += buffer[0];

	/* Parse the frame descriptors. Only uncompressed, MJPEG and frame
	 * based formats have frame descriptors.
	 */
	while (buflen > 2 && buffer[1] == USB_DT_CS_INTERFACE &&
			buffer[2] == ftype) {
		frame = &format->frame[format->nframes];
		if (ftype != UVC_VS_FRAME_FRAME_BASED)
			n = buflen > 25 ? buffer[25] : 0;
		else
			n = buflen > 21 ? buffer[21] : 0;

		n = n ? n : 3;

		if (buflen < 26 + 4*n) {
			uvc_trace(UVC_TRACE_DESCR, "device %d videostreaming "
					"interface %d FRAME error\n", dev->udev->devnum,
					alts->desc.bInterfaceNumber);
			return -EINVAL;
		}

		frame->bFrameIndex = buffer[3];
		frame->bmCapabilities = buffer[4];
		frame->wWidth = get_unaligned_le16(&buffer[5]);
		frame->wHeight = get_unaligned_le16(&buffer[7]);
		frame->dwMinBitRate = get_unaligned_le32(&buffer[9]);
		frame->dwMaxBitRate = get_unaligned_le32(&buffer[13]);
		if (ftype != UVC_VS_FRAME_FRAME_BASED) {
			frame->dwMaxVideoFrameBufferSize =
				get_unaligned_le32(&buffer[17]);
			frame->dwDefaultFrameInterval =
				get_unaligned_le32(&buffer[21]);
			frame->bFrameIntervalType = buffer[25];
		} else {
			frame->dwMaxVideoFrameBufferSize = 0;
			frame->dwDefaultFrameInterval =
				get_unaligned_le32(&buffer[17]);
			frame->bFrameIntervalType = buffer[21];
		}
		frame->dwFrameInterval = *intervals;

		/* Several UVC chipsets screw up dwMaxVideoFrameBufferSize
		 * completely. Observed behaviours range from setting the
		 * value to 1.1x the actual frame size to hardwiring the
		 * 16 low bits to 0. This results in a higher than necessary
		 * memory usage as well as a wrong image size information. For
		 * uncompressed formats this can be fixed by computing the
		 * value from the frame size.
		 */
		if (!(format->flags & UVC_FMT_FLAG_COMPRESSED))
			frame->dwMaxVideoFrameBufferSize = format->bpp
				* frame->wWidth * frame->wHeight / 8;

		/* Some bogus devices report dwMinFrameInterval equal to
		 * dwMaxFrameInterval and have dwFrameIntervalStep set to
		 * zero. Setting all null intervals to 1 fixes the problem and
		 * some other divisions by zero that could happen.
		 */
		for (i = 0; i < n; ++i) {
			interval = get_unaligned_le32(&buffer[26+4*i]);
			*(*intervals)++ = interval ? interval : 1;
		}

		/* Make sure that the default frame interval stays between
		 * the boundaries.
		 */
		n -= frame->bFrameIntervalType ? 1 : 2;
		frame->dwDefaultFrameInterval =
			min(frame->dwFrameInterval[n],
					max(frame->dwFrameInterval[0],
						frame->dwDefaultFrameInterval));

		if (dev->quirks & UVC_QUIRK_RESTRICT_FRAME_RATE) {
			frame->bFrameIntervalType = 1;
			frame->dwFrameInterval[0] =
				frame->dwDefaultFrameInterval;
		}

		uvc_trace(UVC_TRACE_DESCR, "- %ux%u (%u.%u fps)\n",
				frame->wWidth, frame->wHeight,
				10000000/frame->dwDefaultFrameInterval,
				(100000000/frame->dwDefaultFrameInterval)%10);

		format->nframes++;
		buflen -= buffer[0];
		buffer += buffer[0];
	}

	if (buflen > 2 && buffer[1] == USB_DT_CS_INTERFACE &&
			buffer[2] == UVC_VS_COLORFORMAT) {
		if (buflen < 6) {
			uvc_trace(UVC_TRACE_DESCR, "device %d videostreaming "
					"interface %d COLORFORMAT error\n",
					dev->udev->devnum,
					alts->desc.bInterfaceNumber);
			return -EINVAL;
		}

		//format->colorspace = uvc_colorspace(buffer[3]);

		buflen -= buffer[0];
		buffer += buffer[0];
	}

	return buffer - start;
}


/* parse vendor-specific extensions */
static int uvc_parse_vendor_control(struct uvc_device *dev, const unsigned char *buffer, int buflen)
{
	/*TODO: add vendor specific extensions here. */
#if 0
	struct usb_device *udev = dev->udev;
	struct usb_host_interface *alts = &dev->intf->alts[dev->intf->act_altsetting];
#endif

	return 0;
}

static int uvc_parse_streaming(struct uvc_device *dev, struct usb_interface_2 *intf)
{
	struct uvc_streaming *streaming = NULL;
	struct uvc_format *format;
	struct uvc_frame *frame;
	struct usb_host_interface *alts = &intf->alts[0];
	unsigned char *_buffer, *buffer = alts->extra;
	int _buflen, buflen = alts->extralen;
	unsigned int nformats = 0, nframes = 0, nintervals = 0;
	unsigned int size, i, n, p;
	__u32 *interval;
	__u16 psize;
	int ret = -EINVAL;

	if (intf->alts[intf->act_altsetting].desc.bInterfaceSubClass
			!= UVC_SC_VIDEOSTREAMING) {
		uvc_trace(UVC_TRACE_DESCR, "device %d interface %d isn't a "
				"video streaming interface\n", dev->udev->devnum,
				intf->alts[0].desc.bInterfaceNumber);
		return -EINVAL;
	}

	streaming = uvc_malloc(sizeof *streaming);
	if(streaming == NULL)
		return -EINVAL;
	memset(streaming, 0, sizeof(*streaming));

	streaming->dev = dev;
	streaming->intf = intf;
	streaming->intfnum = intf->alts[intf->act_altsetting].desc.bInterfaceNumber;

	if(buflen == 0) {
		for (i = 0; i < alts->desc.bNumEndpoints; ++i) {
			struct usb_host_endpoint *ep = &alts->endpoint[i];

			if (ep->extralen == 0)
				continue;

			if (ep->extralen > 2 &&
					ep->extra[1] == USB_DT_CS_INTERFACE) {
				uvc_trace(UVC_TRACE_DESCR, "trying extra data "
						"from endpoint %u.\n", i);
				buffer = alts->endpoint[i].extra;
				buflen = alts->endpoint[i].extralen;
				break;
			}
		}
	}
	/* Skip the standard interface descriptors. */
	while (buflen > 2 && buffer[1] != USB_DT_CS_INTERFACE) {
		buflen -= buffer[0];
		buffer += buffer[0];
	}

	if (buflen <= 2) {
		uvc_trace(UVC_TRACE_DESCR, "no class-specific streaming "
				"interface descriptors found.\n");
		goto error;
	}

	/* Parse the header descriptor. */
	switch (buffer[2]) {
		case UVC_VS_OUTPUT_HEADER:
			streaming->type = BUF_TYPE_VIDEO_OUTPUT;
			size = 9;
			break;

		case UVC_VS_INPUT_HEADER:
			streaming->type = BUF_TYPE_VIDEO_CAPTURE;
			size = 13;
			break;

		default:
			uvc_trace(UVC_TRACE_DESCR, "device %d videostreaming interface "
					"%d HEADER descriptor not found.\n", dev->udev->devnum,
					alts->desc.bInterfaceNumber);
			goto error;
	}

	p = buflen >= 4 ? buffer[3] : 0;
	n = buflen >= size ? buffer[size-1] : 0;

	if (buflen < size + p*n) {
		uvc_trace(UVC_TRACE_DESCR, "device %d videostreaming "
				"interface %d HEADER descriptor is invalid.\n",
				dev->udev->devnum, alts->desc.bInterfaceNumber);
		goto error;
	}

	streaming->header.bNumFormats = p;
	streaming->header.bEndpointAddress = buffer[6];
	if (buffer[2] == UVC_VS_INPUT_HEADER) {
		streaming->header.bmInfo = buffer[7];
		streaming->header.bTerminalLink = buffer[8];
		streaming->header.bStillCaptureMethod = buffer[9];
		streaming->header.bTriggerSupport = buffer[10];
		streaming->header.bTriggerUsage = buffer[11];
	} else {
		streaming->header.bTerminalLink = buffer[7];
	}
	streaming->header.bControlSize = n;

	unsigned char *ptr = uvc_malloc(p * n);
	if(ptr)
		memcpy(ptr, &buffer[size], p * n);
	streaming->header.bmaControls = ptr;
	if (streaming->header.bmaControls == NULL) {
		ret = -ENOMEM;
		goto error;
	}

	buflen -= buffer[0];
	buffer += buffer[0];

	_buffer = buffer;
	_buflen = buflen;

	/* Count the format and frame descriptors. */
	while (_buflen > 2 && _buffer[1] == USB_DT_CS_INTERFACE) {
		switch (_buffer[2]) {
			case UVC_VS_FORMAT_UNCOMPRESSED:
			case UVC_VS_FORMAT_MJPEG:
			case UVC_VS_FORMAT_FRAME_BASED:
				nformats++;
				break;

			case UVC_VS_FORMAT_DV:
				/* DV format has no frame descriptor. We will create a
				 * dummy frame descriptor with a dummy frame interval.
				 */
				nformats++;
				nframes++;
				nintervals++;
				break;

			case UVC_VS_FORMAT_MPEG2TS:
			case UVC_VS_FORMAT_STREAM_BASED:
				uvc_trace(UVC_TRACE_DESCR, "device %d videostreaming "
						"interface %d FORMAT %u is not supported.\n",
						dev->udev->devnum,
						alts->desc.bInterfaceNumber, _buffer[2]);
				break;

			case UVC_VS_FRAME_UNCOMPRESSED:
			case UVC_VS_FRAME_MJPEG:
				nframes++;
				if (_buflen > 25)
					nintervals += _buffer[25] ? _buffer[25] : 3;
				break;
			case UVC_VS_FRAME_FRAME_BASED:
				nframes++;
				if (_buflen > 21)
					nintervals += _buffer[21] ? _buffer[21] : 3;
				break;
		}

		_buflen -= _buffer[0];
		_buffer += _buffer[0];
	}

	if (nformats == 0) {
		uvc_trace(UVC_TRACE_DESCR, "device %d videostreaming interface "
				"%d has no supported formats defined.\n",
				dev->udev->devnum, alts->desc.bInterfaceNumber);
		goto error;
	}
	size = nformats * sizeof *format + nframes * sizeof *frame
		+ nintervals * sizeof *interval;
	format = uvc_malloc(size);
	if(format == NULL) {
		ret = -ENOMEM;
		goto error;
	}

	frame = (struct uvc_frame *)&format[nformats];
	interval = (__u32 *)&frame[nframes];

	streaming->format = format;
	streaming->nformats = nformats;

	/* Parse the format descriptors. */
	while (buflen > 2 && buffer[1] == USB_DT_CS_INTERFACE) {
		switch (buffer[2]) {
			case UVC_VS_FORMAT_UNCOMPRESSED:
			case UVC_VS_FORMAT_MJPEG:
			case UVC_VS_FORMAT_DV:
			case UVC_VS_FORMAT_FRAME_BASED:
				format->frame = frame;
				ret = uvc_parse_format(dev, streaming, format,
						&interval, buffer, buflen);
				if (ret < 0)
					goto error;

				frame += format->nframes;
				format++;

				buflen -= ret;
				buffer += ret;
				continue;

			default:
				break;
		}

		buflen -= buffer[0];
		buffer += buffer[0];
	}

	if (buflen)
		uvc_trace(UVC_TRACE_DESCR, "device %d videostreaming interface "
				"%d has %u bytes of trailing descriptor garbage.\n",
				dev->udev->devnum, alts->desc.bInterfaceNumber, buflen);

	/* Parse the alternate settings to find the maximum bandwidth. */
	for (i = 0; i < intf->num_altsetting; ++i) {

		struct usb_host_endpoint *ep;
		alts = &intf->alts[i];
		ep = uvc_find_endpoint(alts,
				streaming->header.bEndpointAddress);
		if (ep == NULL)
			continue;

		psize = le16_to_cpu(ep->desc.wMaxPacketSize);
		psize = (psize & 0x07ff) * (1 + ((psize >> 11) & 3));
		if (psize > streaming->maxpsize)
			streaming->maxpsize = psize;
	}

	list_add_tail(&streaming->list, &dev->streams);



	return 0;
error:
	uvc_free(streaming->format);
	uvc_free(streaming->header.bmaControls);
	uvc_free(streaming);
	return ret;
}

static struct uvc_entity *uvc_alloc_entity(u16 type, u8 id, unsigned int num_pads, unsigned int extra_size)
{
	struct uvc_entity *entity;
	unsigned int size;

	size = sizeof(*entity) + extra_size;
	entity = uvc_malloc(size);
	if (entity == NULL)
		return NULL;

	entity->id = id;
	entity->type = type;


	return entity;
}
static int uvc_parse_standard_control(struct uvc_device *dev,
	const unsigned char *buffer, int buflen)
{
	struct usb_device *udev = dev->udev;
	struct uvc_entity *unit, *term;
	struct usb_interface_2 *intf;
	struct usb_host_interface *alts = &dev->intf->alts[dev->intf->act_altsetting];
	unsigned int i, n, p, len;
	unsigned short type;

	switch(buffer[2]) {

	case UVC_VC_HEADER:
		n = buflen >= 12 ? buffer[11]:0;
		if(buflen < 12 || buflen < 12 + n) {
			uvc_trace(UVC_TRACE_DESCR, "device %d videocontrol "
					"interface %d HEADER error\n", udev->devnum,
					alts->desc.bInterfaceNumber);
			return -EINVAL;
		}

		dev->uvc_version = get_unaligned_le16(&buffer[3]);
		dev->clock_frequency = get_unaligned_le32(&buffer[7]);

		/* Parse all USB Video Streaming Interface */
		for(i = 0; i < n; i++) {
			debug("-- parse streaming %d, %d, ifnum %d\n", i, n, buffer[12 + i]);
			intf = usb_ifnum_to_if(udev, buffer[12+i]);
			if(intf == NULL) {
				uvc_trace(UVC_TRACE_DESCR, "device %d "
						"interface %d doesn't exists\n",
						udev->devnum, i);
				continue;
			}
			uvc_parse_streaming(dev, intf);
		}
		break;
	case UVC_VC_INPUT_TERMINAL:
		if(buflen < 8) {
			uvc_trace(UVC_TRACE_DESCR, "device %d videocontrol "
					"interface %d INPUT_TERMINAL error\n",
					udev->devnum, alts->desc.bInterfaceNumber);
			return -EINVAL;
		}

		type = get_unaligned_le16(&buffer[4]);
		if((type & 0xff00) == 0) {
			uvc_trace(UVC_TRACE_DESCR, "device %d videocontrol "
					"interface %d INPUT_TERMINAL %d has invalid "
					"type 0x%04x, skipping\n", udev->devnum,
					alts->desc.bInterfaceNumber,
					buffer[3], type);
			return 0;
		}

		n = 0;
		p = 0;
		len = 8;
		if(type == UVC_ITT_CAMERA) {
			n = buflen >= 15 ? buffer[14] : 0;
			len = 15;
		} else if (type == UVC_ITT_MEDIA_TRANSPORT_INPUT) {
			n = buflen >= 9 ? buffer[8] : 0;
			p = buflen >= 10 + n ? buffer[9+n] : 0;
			len = 10;
		}

		if (buflen < len + n + p) {
			uvc_trace(UVC_TRACE_DESCR, "device %d videocontrol "
					"interface %d INPUT_TERMINAL error\n",
					udev->devnum, alts->desc.bInterfaceNumber);

			return -EINVAL;
		}

		term = uvc_alloc_entity(type | UVC_TERM_INPUT, buffer[3], 1, n + p);
		if(term == NULL) {
			return -ENOMEM;
		}

		if(UVC_ENTITY_TYPE(term) == UVC_ITT_CAMERA) {
			term->camera.bControlSize = n;
			term->camera.bmControls = (__u8 *)term + sizeof(*term);
			term->camera.wObjectiveFocalLengthMin = get_unaligned_le16(&buffer[8]);
			term->camera.wObjectiveFocalLengthMax = get_unaligned_le16(&buffer[10]);
				term->camera.wOcularFocalLength = get_unaligned_le16(&buffer[12]);
			memcpy(term->camera.bmControls, &buffer[15], n);
		} else if(UVC_ENTITY_TYPE(term) == UVC_ITT_MEDIA_TRANSPORT_INPUT) {
			term->media.bControlSize = n;
			term->media.bmControls = (__u8 *)term + sizeof *term;
			term->media.bTransportModeSize = p;
			term->media.bmTransportModes = (__u8 *)term
				+ sizeof *term + n;
			memcpy(term->media.bmControls, &buffer[9], n);
			memcpy(term->media.bmTransportModes, &buffer[10+n], p);
		}

		if(buffer[7] != 0) {
		} else if(UVC_ENTITY_TYPE(term) == UVC_ITT_CAMERA) {
			sprintf(term->name, "Camera %u", buffer[3]);
		} else if(UVC_ENTITY_TYPE(term) == UVC_ITT_MEDIA_TRANSPORT_INPUT) {
			sprintf(term->name, "Media %u", buffer[3]);
		} else {
			sprintf(term->name, "Input %u", buffer[3]);
		}

		list_add_tail(&term->list, &dev->entities);
		break;

	case UVC_VC_OUTPUT_TERMINAL:
		if (buflen < 9) {
			uvc_trace(UVC_TRACE_DESCR, "device %d videocontrol "
					"interface %d OUTPUT_TERMINAL error\n",
					udev->devnum, alts->desc.bInterfaceNumber);
			return -EINVAL;
		}

		/* Make sure the terminal type MSB is not null, otherwise it
		 * could be confused with a unit.
		 */
		type = get_unaligned_le16(&buffer[4]);
		if ((type & 0xff00) == 0) {
			uvc_trace(UVC_TRACE_DESCR, "device %d videocontrol "
					"interface %d OUTPUT_TERMINAL %d has invalid "
					"type 0x%04x, skipping\n", udev->devnum,
					alts->desc.bInterfaceNumber, buffer[3], type);
			return 0;
		}

		term = uvc_alloc_entity(type | UVC_TERM_OUTPUT, buffer[3],
				1, 0);
		if (term == NULL) {
			return -ENOMEM;
		}
		//memcpy(term->baSourceID, &buffer[7], 1);

		if (buffer[8] != 0)
			usb_string(udev, buffer[8], term->name,
					sizeof term->name);
		else
			sprintf(term->name, "Output %u", buffer[3]);

		list_add_tail(&term->list, &dev->entities);

		break;

	case UVC_VC_SELECTOR_UNIT:
		p = buflen >= 5 ? buffer[4] : 0;

		if (buflen < 5 || buflen < 6 + p) {
			uvc_trace(UVC_TRACE_DESCR, "device %d videocontrol "
					"interface %d SELECTOR_UNIT error\n",
					udev->devnum, alts->desc.bInterfaceNumber);
			return -EINVAL;
		}

		unit = uvc_alloc_entity(buffer[2], buffer[3], p + 1, 0);
		if (unit == NULL) {
			return -ENOMEM;
		}
		//memcpy(unit->baSourceID, &buffer[5], p);

		if (buffer[5+p] != 0)
			usb_string(udev, buffer[5+p], unit->name,
					sizeof unit->name);
		else
			sprintf(unit->name, "Selector %u", buffer[3]);

		list_add_tail(&unit->list, &dev->entities);
		break;

	case UVC_VC_PROCESSING_UNIT:
		n = buflen >= 8 ? buffer[7] : 0;
		p = dev->uvc_version >= 0x0110 ? 10 : 9;

		if (buflen < p + n) {
			uvc_trace(UVC_TRACE_DESCR, "device %d videocontrol "
					"interface %d PROCESSING_UNIT error\n",
					udev->devnum, alts->desc.bInterfaceNumber);
			return -EINVAL;
		}
		unit = uvc_alloc_entity(buffer[2], buffer[3], 2, n);
		if (unit == NULL) {
			return -ENOMEM;
		}
		//memcpy(unit->baSourceID, &buffer[4], 1);
		unit->processing.wMaxMultiplier =
			get_unaligned_le16(&buffer[5]);
		unit->processing.bControlSize = buffer[7];
		unit->processing.bmControls = (__u8 *)unit + sizeof *unit;
		memcpy(unit->processing.bmControls, &buffer[8], n);
		if (dev->uvc_version >= 0x0110)
			unit->processing.bmVideoStandards = buffer[9+n];

		if (buffer[8+n] != 0)
			usb_string(udev, buffer[8+n], unit->name,
					sizeof unit->name);
		else
			sprintf(unit->name, "Processing %u", buffer[3]);

		list_add_tail(&unit->list, &dev->entities);
		break;

	case UVC_VC_EXTENSION_UNIT:
		p = buflen >= 22 ? buffer[21] : 0;
		n = buflen >= 24 + p ? buffer[22+p] : 0;

		if (buflen < 24 + p + n) {
			uvc_trace(UVC_TRACE_DESCR, "device %d videocontrol "
					"interface %d EXTENSION_UNIT error\n",
					udev->devnum, alts->desc.bInterfaceNumber);
			return -EINVAL;
		}

		unit = uvc_alloc_entity(buffer[2], buffer[3], p + 1, n);
		if (unit == NULL) {
			return -ENOMEM;
		}

		memcpy(unit->extension.guidExtensionCode, &buffer[4], 16);
		unit->extension.bNumControls = buffer[20];
		//memcpy(unit->baSourceID, &buffer[22], p);
		unit->extension.bControlSize = buffer[22+p];
		unit->extension.bmControls = (__u8 *)unit + sizeof *unit;
		memcpy(unit->extension.bmControls, &buffer[23+p], n);

		if (buffer[23+p+n] != 0)
			usb_string(udev, buffer[23+p+n], unit->name,
					sizeof unit->name);
		else
			sprintf(unit->name, "Extension %u", buffer[3]);

		list_add_tail(&unit->list, &dev->entities);

		break;

	default:
		uvc_trace(UVC_TRACE_DESCR, "Found an unknown CS_INTERFACE "
				"descriptor (%u)\n", buffer[2]);

		break;

	}

	return 0;
}

static int uvc_parse_control(struct uvc_device *dev)
{
	struct usb_host_interface *alts = &dev->intf->alts[dev->intf->act_altsetting];
	unsigned char *buffer = alts->extra;
	int buflen = alts->extralen;
	int ret;

	/* Parse the default alternate setting only, as the UVC specification
	 * defines a single alternate setting, the default alternate setting
	 * zero.
	 */

	while(buflen > 2) {

		if(uvc_parse_vendor_control(dev, buffer, buflen) ||
				buffer[1] != USB_DT_CS_INTERFACE)
			goto next_descriptor;

		if((ret = uvc_parse_standard_control(dev, buffer, buflen)) < 0) {
			return ret;
		}
next_descriptor:
		buflen -= buffer[0];
		buffer += buffer[0];
	}

#if 1
	/* Check if the optional status endpoint is present. Built-in iSight
	 * webcams have an interrupt endpoint but spit proprietary data that
	 * don't conform to the UVC status endpoint messages. Don't try to
	 * handle the interrupt endpoint for those cameras.
	 */
	if (alts->desc.bNumEndpoints == 1 &&
			!(dev->quirks & UVC_QUIRK_BUILTIN_ISIGHT)) {
		struct usb_host_endpoint *ep = &alts->endpoint[0];
		struct usb_endpoint_descriptor *desc = &ep->desc;

		if (usb_endpoint_is_int_in(desc) &&
				le16_to_cpu(desc->wMaxPacketSize) >= 8 &&
				desc->bInterval != 0) {
			uvc_trace(UVC_TRACE_DESCR, "Found a Status endpoint "
					"(addr %02x).\n", desc->bEndpointAddress);
			dev->int_ep = ep;
		}
	}
#endif

	return 0;
}


void dump_uvc_streaming_ctrl(struct uvc_streaming_control *ctrl)
{
	printf("bmHint %d\n", ctrl->bmHint);
	printf("bFormatIndex %d\n", ctrl->bFormatIndex);
	printf("bFrameIndex %d\n", ctrl->bFrameIndex);
	printf("dwFrameInterval %d\n", ctrl->dwFrameInterval);
	printf("wKeyFrameRate %d\n", ctrl->wKeyFrameRate);
	printf("wPFrameRate %d\n", ctrl->wPFrameRate);
	printf("wCompQuality %d\n", ctrl->wCompQuality);
	printf("wCompWindowSize %d\n", ctrl->wCompWindowSize);
	printf("wDelay %d\n", ctrl->wDelay);
	printf("dwMaxVideoFrameSize %d\n", ctrl->dwMaxVideoFrameSize);
	printf("dwMaxPayloadTransferSize %d\n", ctrl->dwMaxPayloadTransferSize);
	printf("dwClockFrequency %d\n", ctrl->dwClockFrequency);
	printf("bmFramingInfo %d\n", ctrl->bmFramingInfo);
	printf("bPreferedVersion 0x%x\n", ctrl->bPreferedVersion);
	printf("bMinVersion 0x%x\n", ctrl->bMinVersion);
	printf("bMaxVersion 0x%x\n", ctrl->bMaxVersion);
}

void dump_uvc_streaming_format(struct uvc_streaming *stream)
{
	struct uvc_format *format = stream->cur_format;
	struct uvc_frame *frame = stream->cur_frame;

	printf("----current streaming format\n");
	printf("\tformat type: %d\n", format->type);
	printf("\tformat index: %d\n", format->index);
	printf("\tformat bpp: %x\n", format->bpp);
	printf("\tformat colorspace: %x\n", format->bpp);
	printf("\tformat name: %s\n", format->name);
	printf("\tformat nframe: %d\n", format->nframes);

	printf("\t----current frame\n");
	printf("\t\tbFrameIndex %d\n", frame->bFrameIndex);
	printf("\t\tbmCapabilities 0x%x\n", frame->bmCapabilities);
	printf("\t\twWidth %d\n", frame->wWidth);
	printf("\t\twHeight %d\n", frame->wHeight);
	printf("\t\tdwMinBitRate %d\n", frame->dwMinBitRate);
	printf("\t\tdwMaxBitRate %d\n", frame->dwMaxBitRate);
	printf("\t\tdwMaxVideoFrameBufferSize %d\n", frame->dwMaxVideoFrameBufferSize);
	printf("\t\tbFrameIntervalType %d\n", frame->bFrameIntervalType);
	printf("\t\tdwDefaultFrameInterval %d\n", frame->dwDefaultFrameInterval);

}

void dump_uvc_streaming(struct uvc_streaming *stream)
{
	printf("streaming intfnum: %d\n", stream->intfnum);
	printf("streaming maxpsize: %d\n", stream->maxpsize);
	printf("streaming nformats: %d\n", stream->nformats);

	int i;
	for(i = 0; i < stream->nformats; i++) {
		struct uvc_format *format = &stream->format[i];
		printf("streaming format %d\n", i);
		printf("\tformat type: %d\n", format->type);
		printf("\tformat index: %d\n", format->index);
		printf("\tformat bpp: %x\n", format->bpp);
		printf("\tformat colorspace: %x\n", format->bpp);
		printf("\tformat name: %s\n", format->name);
		printf("\tformat nframe: %d\n", format->nframes);
		printf("streaming format frame\n");

		int j;
		for(j = 0; j < format->nframes; j++) {
			struct uvc_frame *frame = &format->frame[j];
			printf("\tframe %d\n", j);
			printf("\t\tbFrameIndex %d\n", frame->bFrameIndex);
			printf("\t\tbmCapabilities 0x%x\n", frame->bmCapabilities);
			printf("\t\twWidth %d\n", frame->wWidth);
			printf("\t\twHeight %d\n", frame->wHeight);
			printf("\t\tdwMinBitRate %d\n", frame->dwMinBitRate);
			printf("\t\tdwMaxBitRate %d\n", frame->dwMaxBitRate);
			printf("\t\tdwMaxVideoFrameBufferSize %d\n", frame->dwMaxVideoFrameBufferSize);
			printf("\t\tbFrameIntervalType %d\n", frame->bFrameIntervalType);
			printf("\t\tdwDefaultFrameInterval %d\n", frame->dwDefaultFrameInterval);
		}
	}


}

struct uvc_device *uvc_get_device(int dev_num)
{
	if(dev_num > USB_MAX_VIDEO_DEVICE) {
		printf("uvc get device error, uvc_dev %d max %d", dev_num, USB_MAX_VIDEO_DEVICE);
		return NULL;
	}

	return &uvc_dev[dev_num];
}

/*
	dev: [IN]
	ifnum: [IN]
	uvc_device [OUT]
*/
static int uvc_probe(struct usb_device *udev, int ifnum, struct uvc_device *dev)
{
	struct usb_interface_2 *iface;

	iface = &udev->config.if_desc2[ifnum];

	if(iface->alts[0].desc.bInterfaceClass != USB_CLASS_VIDEO) {
		/* Not a Video device. */
		return 0;
	}

	memset(dev, 0, sizeof(struct uvc_device));

	INIT_LIST_HEAD(&dev->entities);
	INIT_LIST_HEAD(&dev->streams);

	dev->nstreams = 0;


	debug("\n\nUSB Video Device detected\n");

	dev->udev = udev;
	dev->intf = iface;
	dev->intfnum = ifnum;

	if (udev->prod != NULL)
		memcpy(dev->name, udev->prod, sizeof dev->name);
	else
		snprintf(dev->name, sizeof dev->name,
				"UVC Camera (%04x:%04x)",
				le16_to_cpu(udev->descriptor.idVendor),
				le16_to_cpu(udev->descriptor.idProduct));


	/* Parse the Video Class control descriptor. */
	if (uvc_parse_control(dev) < 0) {
		uvc_trace(UVC_TRACE_PROBE, "Unable to parse UVC "
				"descriptors.\n");
		goto error;
	}

	uvc_printk(KERN_INFO, "Found UVC %u.%02x device %s (%04x:%04x)\n",
			dev->uvc_version >> 8, dev->uvc_version & 0xff,
			udev->prod ? udev->prod : "<unnamed>",
			le16_to_cpu(udev->descriptor.idVendor),
			le16_to_cpu(udev->descriptor.idProduct));

#if 0
	if (dev->quirks != id->driver_info) {
		uvc_printk(KERN_INFO, "Forcing device quirks to 0x%x by module "
				"parameter for testing purpose.\n", dev->quirks);
		uvc_printk(KERN_INFO, "Please report required quirks to the "
				"linux-uvc-devel mailing list.\n");
	}
#endif

	/*TODO: parse ...*/

	struct uvc_streaming *stream;
	list_for_each_entry(stream, &dev->streams, list) {
#ifdef DEBUG
		dump_uvc_streaming(stream);
#endif
	}

	struct uvc_entity *entity;
	list_for_each_entry(entity, &dev->entities, list) {
		if(UVC_ENTITY_TYPE(entity) == UVC_TT_STREAMING &&
			UVC_ENTITY_IS_OTERM(entity)) {

			stream = uvc_stream_by_id(dev, entity->id);

			dev->video = stream;
		}
	}

	return 1;

error:
	return 0;
}

int usb_video_scan(int mode)
{
	unsigned char i;
	struct usb_device *dev;

	if(mode == 1)
		printf("	scanning usb for video devices... ");

	usb_disable_asynch(1);	/* async transfer not allowed. */

	usb_max_devs = 0;
	for(i = 0; i < USB_MAX_DEVICE; i++) {
		dev = usb_get_dev_index(i);
		if(dev == NULL)
			break;

		if(uvc_probe(dev, 0, &uvc_dev[usb_max_devs]))	{
			/* use interface0 to probe */

			usb_max_devs ++;
		}

		if(usb_max_devs == USB_MAX_VIDEO_DEVICE) {
			printf("max USB Video Device reached: %d stopping\n",
				usb_max_devs);
			break;
		}
	}


	usb_disable_asynch(0);

	printf("%d Video Device(s) found\n", usb_max_devs);
	if (usb_max_devs > 0)
		return 0;
	return -1;

}

