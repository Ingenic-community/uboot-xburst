#include <common.h>
#include <errno.h>

#include <fat.h>
#include <part.h>
#include <fs.h>

#include <linux/list.h>
#include <linux/usb/video.h>
#include <usb.h>

#include "uvcvideo.h"
#include "videodev2.h"


/*
 * Compute the maximum number of bytes per interval for an endpoint.
 */
static unsigned int uvc_endpoint_max_bpi(struct usb_device *dev,
                                         struct usb_host_endpoint *ep)
{
        u16 psize;

        switch (dev->speed) {
#if 0	/*TODO:*/
        case USB_SPEED_SUPER:
                return ep->ss_ep_comp.wBytesPerInterval;
#endif
        case USB_SPEED_HIGH:
                psize = usb_endpoint_maxp(&ep->desc);
                return (psize & 0x07ff) * (1 + ((psize >> 11) & 3));
        default:
                psize = usb_endpoint_maxp(&ep->desc);
                return psize & 0x07ff;
        }
}

struct uvc_buffer *uvc_video_get_buffer(struct uvc_streaming *stream)
{
	return &stream->video_buffer[stream->next_buffer];
}

/* UVC video handle */
static int __uvc_query_ctrl(struct uvc_device *dev, __u8 query, __u8 unit,
		__u8 intfnum, __u8 cs, void *data, __u16 size,
		int timeout)
{
	__u8 type = USB_TYPE_CLASS | USB_RECIP_INTERFACE;
	unsigned int pipe;

	pipe = (query & 0x80) ? usb_rcvctrlpipe(dev->udev, 0)
		: usb_sndctrlpipe(dev->udev, 0);
	type |= (query & 0x80) ? USB_DIR_IN : USB_DIR_OUT;

	return usb_control_msg(dev->udev, pipe, query, type, cs << 8,
			unit << 8 | intfnum, data, size, timeout);
}

static int uvc_get_video_ctrl(struct uvc_streaming *stream,
        struct uvc_streaming_control *ctrl, int probe, __u8 query)
{
	__u8 *data;
	__u16 size;
	int ret;

	size = stream->dev->uvc_version >= 0x0110 ? 34 : 26;
	if ((stream->dev->quirks & UVC_QUIRK_PROBE_DEF) &&
			query == UVC_GET_DEF)
		return -EIO;

	data = uvc_malloc(size);
	if (data == NULL)
		return -ENOMEM;

	ret = __uvc_query_ctrl(stream->dev, query, 0, stream->intfnum,
			probe ? UVC_VS_PROBE_CONTROL : UVC_VS_COMMIT_CONTROL, data,
			size, uvc_timeout_param);

	if ((query == UVC_GET_MIN || query == UVC_GET_MAX) && ret == 2) {
		/* Some cameras, mostly based on Bison Electronics chipsets,
		 * answer a GET_MIN or GET_MAX request with the wCompQuality
		 * field only.
		 */
		uvc_warn_once(stream->dev, UVC_WARN_MINMAX, "UVC non "
				"compliance - GET_MIN/MAX(PROBE) incorrectly "
				"supported. Enabling workaround.\n");
		memset(ctrl, 0, sizeof *ctrl);
		ctrl->wCompQuality = le16_to_cpup((__le16 *)data);
		ret = 0;
		goto out;
	} else if (query == UVC_GET_DEF && probe == 1 && ret != size) {
		/* Many cameras don't support the GET_DEF request on their
		 * video probe control. Warn once and return, the caller will
		 * fall back to GET_CUR.
		 */
		uvc_warn_once(stream->dev, UVC_WARN_PROBE_DEF, "UVC non "
				"compliance - GET_DEF(PROBE) not supported. "
				"Enabling workaround.\n");
		ret = -EIO;
		goto out;
	} else if (ret != size) {
		uvc_printk(KERN_ERR, "Failed to query (%u) UVC %s control : "
				"%d (exp. %u).\n", query, probe ? "probe" : "commit",
				ret, size);
		ret = -EIO;
		goto out;
	}

	ctrl->bmHint = le16_to_cpup((__le16 *)&data[0]);
	ctrl->bFormatIndex = data[2];
	ctrl->bFrameIndex = data[3];
	ctrl->dwFrameInterval = le32_to_cpup((__le32 *)&data[4]);
	ctrl->wKeyFrameRate = le16_to_cpup((__le16 *)&data[8]);
	ctrl->wPFrameRate = le16_to_cpup((__le16 *)&data[10]);
	ctrl->wCompQuality = le16_to_cpup((__le16 *)&data[12]);
	ctrl->wCompWindowSize = le16_to_cpup((__le16 *)&data[14]);
	ctrl->wDelay = le16_to_cpup((__le16 *)&data[16]);
	ctrl->dwMaxVideoFrameSize = get_unaligned_le32(&data[18]);
	ctrl->dwMaxPayloadTransferSize = get_unaligned_le32(&data[22]);

	if (size == 34) {
		ctrl->dwClockFrequency = get_unaligned_le32(&data[26]);
		ctrl->bmFramingInfo = data[30];
		ctrl->bPreferedVersion = data[31];
		ctrl->bMinVersion = data[32];
		ctrl->bMaxVersion = data[33];
	} else {
		ctrl->dwClockFrequency = stream->dev->clock_frequency;
		ctrl->bmFramingInfo = 0;
		ctrl->bPreferedVersion = 0;
		ctrl->bMinVersion = 0;
		ctrl->bMaxVersion = 0;
	}

	/* Some broken devices return null or wrong dwMaxVideoFrameSize and
	 * dwMaxPayloadTransferSize fields. Try to get the value from the
	 * format and frame descriptors.
	 */
	//uvc_fixup_video_ctrl(stream, ctrl);
	ret = 0;

out:
	uvc_free(data);
	return ret;
}

static int uvc_set_video_ctrl(struct uvc_streaming *stream,
        struct uvc_streaming_control *ctrl, int probe)
{
        __u8 *data;
        __u16 size;
        int ret;

        size = stream->dev->uvc_version >= 0x0110 ? 34 : 26;
        data = uvc_malloc(size);
        if (data == NULL)
                return -ENOMEM;

        *(__le16 *)&data[0] = cpu_to_le16(ctrl->bmHint);
        data[2] = ctrl->bFormatIndex;
        data[3] = ctrl->bFrameIndex;
        *(__le32 *)&data[4] = cpu_to_le32(ctrl->dwFrameInterval);
        *(__le16 *)&data[8] = cpu_to_le16(ctrl->wKeyFrameRate);
        *(__le16 *)&data[10] = cpu_to_le16(ctrl->wPFrameRate);
        *(__le16 *)&data[12] = cpu_to_le16(ctrl->wCompQuality);
        *(__le16 *)&data[14] = cpu_to_le16(ctrl->wCompWindowSize);
        *(__le16 *)&data[16] = cpu_to_le16(ctrl->wDelay);
        put_unaligned_le32(ctrl->dwMaxVideoFrameSize, &data[18]);
        put_unaligned_le32(ctrl->dwMaxPayloadTransferSize, &data[22]);

        if (size == 34) {
                put_unaligned_le32(ctrl->dwClockFrequency, &data[26]);
                data[30] = ctrl->bmFramingInfo;
                data[31] = ctrl->bPreferedVersion;
                data[32] = ctrl->bMinVersion;
                data[33] = ctrl->bMaxVersion;
        }

        ret = __uvc_query_ctrl(stream->dev, UVC_SET_CUR, 0, stream->intfnum,
                probe ? UVC_VS_PROBE_CONTROL : UVC_VS_COMMIT_CONTROL, data,
                size, uvc_timeout_param);
        if (ret != size) {
                uvc_printk(KERN_ERR, "Failed to set UVC %s control : "
                        "%d (exp. %u).\n", probe ? "probe" : "commit",
                        ret, size);
                ret = -EIO;
        }

        uvc_free(data);
        return ret;
}


static int uvc_commit_video(struct uvc_streaming *stream,
                            struct uvc_streaming_control *probe)
{
        return uvc_set_video_ctrl(stream, probe, 0);
}



static void uvc_video_decode_isight(struct uvc_streaming *stream,
                unsigned char *buf, unsigned int len)
{
	/* TODO: add code here. */
	debug("--- %s, %d \n", __func__, __LINE__);
}


static int uvc_video_decode_start(struct uvc_streaming *stream,
                __u8 *data, int len)
{
	__u8 fid;
	struct uvc_buffer *buf = uvc_video_get_buffer(stream);
	/* Sanity checks:
	 * - packet must be at least 2 bytes long
	 * - bHeaderLength value must be at least 2 bytes (see above)
	 * - bHeaderLength value can't be larger than the packet size.
	 */
	if (len < 2 || data[0] < 2 || data[0] > len) {
		return -EINVAL;
	}

	fid = data[1] & UVC_STREAM_FID;

	/* Increase the sequence number regardless of any buffer states, so
	 * that discontinuous sequence numbers always indicate lost frames.
	 */
	if (stream->last_fid != fid) {
		stream->sequence++;
#if 0
		if (stream->sequence)
			uvc_video_stats_update(stream);
#endif
	}

	/* Mark the buffer as bad if the error bit is set. */
	if (data[1] & UVC_STREAM_ERR) {
		uvc_trace(UVC_TRACE_FRAME, "Marking buffer as bad (error bit "
				"set). %x %x\n", data[0], data[1]);
		buf->error = 1;
	}

	stream->last_fid = fid;

	return data[0];
}

static void uvc_video_decode_data(struct uvc_streaming *stream,
                __u8 *data, int len)
{

	struct uvc_buffer *buf = uvc_video_get_buffer(stream);

	if(len < 0)
		return;

	memcpy(buf->buffer + buf->offset, data, len);

	buf->offset += len;
}

/*
 * Completion handler for video URBs.
 */
static void uvc_video_decode_isoc(struct uvc_streaming *stream,
        unsigned char *mem, unsigned int len)
{
	int ret;

	//debug("decode isoc, expect len %d, actual len %d, mem %x\n",
	//	stream->request_size, udev->act_len, mem);

	/* Decode the payload header */
	ret = uvc_video_decode_start(stream, mem, len);
	if(ret < 0) {
		debug("uvc_video_decode_start error ret %d, mem[0] %x, mem[1] %x, len %d\n", ret, mem[0], mem[1], len);
		return;
	}

	/* Process the payload data */
	uvc_video_decode_data(stream, mem + ret, len - ret);


}

static void uvc_video_decode_bulk(struct uvc_streaming *stream,
        unsigned char *buf, unsigned int len)
{
	debug("--- %s, %d \n", __func__, __LINE__);
}

static void uvc_video_encode_bulk(struct uvc_streaming *stream,
        unsigned char *buf, unsigned int len)
{
	debug("--- %s, %d \n", __func__, __LINE__);
}

/*
	------------------
	VIDEO API exported
*/

/*
 * Initialize isochronous URBs and allocate transfer buffers. The packet size
 * is given by the endpoint.
 */
static int uvc_init_video_isoc(struct uvc_streaming *stream,
        struct usb_host_endpoint *ep)
{
	int i;
	u16 psize;
	u32 size;

	psize = uvc_endpoint_max_bpi(stream->dev->udev, ep);

	size = psize + 32;

	stream->request_buffer = uvc_malloc(size);
	if(stream->request_buffer == NULL) {
		debug("Failed to alloc request_buffer!\n");
		return -ENOMEM;
	}
	stream->request_size = size;
	stream->ep = ep;

	size = stream->cur_frame->dwMaxVideoFrameBufferSize;
	for(i = 0; i < NR_BUFFERS; i++) {
		stream->video_buffer[i].buffer = uvc_malloc(size);
		if(stream->video_buffer[i].buffer == NULL) {
			debug("Failed to alloc video buffer! %d\n", i);
			return -ENOMEM;
		}

		printf("video_buffer[%d]: %p\n", i, stream->video_buffer[i].buffer);

		stream->video_buffer[i].offset = 0;
		stream->video_buffer[i].act_len = 0;
		stream->video_buffer[i].error = 0;
	}
	return 0;
}


int uvc_usb_request_data(struct uvc_streaming *stream, void (*complete)(struct uvc_streaming *stream, struct uvc_buffer *uvc_buffer))
{
	struct usb_device *udev = stream->dev->udev;
	struct usb_host_endpoint *ep = stream->ep;
	int pipe = usb_rcvisocpipe(stream->dev->udev,
			ep->desc.bEndpointAddress);

	int ret;
	unsigned char *mem = stream->request_buffer;
	int framesize = stream->cur_frame->dwMaxVideoFrameBufferSize;
	struct uvc_buffer *buf = uvc_video_get_buffer(stream);

	buf->error = 0;
	buf->offset = 0;

	while(1) {
		ret = usb_submit_isoc_msg(udev, pipe, mem, stream->maxpsize, ep->desc.bInterval);
		if(ret < 0) {
			printf("isoc transfer error %d\n", ret);
			return ret;
		}

		/* decode a usb frame buffer, may contain 1 to 3 packets */
		if(udev->act_len > 0) {
			stream->decode(stream, mem, udev->act_len);
		} else {
			/* continue with act_len == 0;*/
			continue;
		}
		mem = stream->request_buffer;

		if((mem[1] & UVC_STREAM_EOF)) {
#if 1
			if(framesize != buf->offset) {

				buf->error = 1;
#if 0
				debug("New Frame error, framesize %d, video_offset %d, stream->last_fid %d, stream->next_buffer %d\n", framesize, buf->offset, stream->last_fid, stream->next_buffer);
				printf("%02x %02x %02x %02x", mem[0], mem[1], mem[2], mem[3]);
#endif
			}
#endif

			stream->next_buffer ++;
			stream->next_buffer %= NR_BUFFERS;

			break;
		}

	}

	buf->act_len = buf->offset;

	//uvc_video_complete(stream, buf);
	complete(stream, buf);

	return 0;
}

static int uvc_init_video(struct uvc_streaming *stream)
{
	struct usb_interface_2 *intf = stream->intf;
	struct usb_host_endpoint *ep;
	unsigned int i;
	int ret = 0;


	stream->sequence = -1;
	stream->last_fid = -1;
	stream->next_buffer = 0;
	stream->bulk.header_size = 0;
	stream->bulk.skip_payload = 0;
	stream->bulk.payload_size = 0;

	if(intf->num_altsetting > 1) {
		struct usb_host_endpoint *best_ep = NULL;
		unsigned int best_psize = ~0U;
		unsigned int bandwidth;
		unsigned int uninitialized_var(altsetting);
		int intfnum = stream->intfnum;

		/* Isochronous endpoint, select the alternate setting. */
		bandwidth = stream->ctrl.dwMaxPayloadTransferSize;

		printf("bandwidth %d\n", bandwidth);

		if (bandwidth == 0) {
			uvc_trace(UVC_TRACE_VIDEO, "Device requested null "
					"bandwidth, defaulting to lowest.\n");
			bandwidth = 1;
		} else {
			uvc_trace(UVC_TRACE_VIDEO, "Device requested %u "
					"B/frame bandwidth.\n", bandwidth);
		}

		for (i = 0; i < intf->num_altsetting; ++i) {
			struct usb_host_interface *alts;
			unsigned int psize;

			alts = &intf->alts[i];
			ep = uvc_find_endpoint(alts,
					stream->header.bEndpointAddress);
			if (ep == NULL)
				continue;

			/* Check if the bandwidth is high enough. */
			psize = uvc_endpoint_max_bpi(stream->dev->udev, ep);
			if (psize >= bandwidth && psize <= best_psize) {
				altsetting = alts->desc.bAlternateSetting;
				best_psize = psize;
				best_ep = ep;
			}
		}

		if (best_ep == NULL) {
			uvc_trace(UVC_TRACE_VIDEO, "No fast enough alt setting "
					"for requested bandwidth.\n");
			return -EIO;
		}

		uvc_trace(UVC_TRACE_VIDEO, "Selecting alternate setting %u "
				"(%u B/frame bandwidth).\n", altsetting, best_psize);

		ret = usb_set_interface(stream->dev->udev, intfnum, altsetting);
		usb_set_maxpacket_ep_2(stream->dev->udev, intfnum, best_ep);
		if (ret < 0)
			return ret;

		ret = uvc_init_video_isoc(stream, best_ep);
	} else {
		/* Bulk endpoint, proceed to URB initialization. */
		ep = uvc_find_endpoint(&intf->alts[0],
				stream->header.bEndpointAddress);
		if (ep == NULL)
			return -EIO;

		//ret = uvc_init_video_bulk(stream, ep);
	}

	if (ret < 0)
		return ret;

#if 0
	/* Send USB request .*/
	ret = uvc_usb_request_data(stream);
	if(ret < 0) {
		printf("failed to request data!\n");
		return ret;
	}
#endif
	return 0;
}

static void uvc_uninit_video(struct uvc_streaming *stream)
{
	int i;

	for(i = 0; i < NR_BUFFERS; i++) {
		uvc_free(stream->video_buffer[i].buffer);
	}

	uvc_free(stream->request_buffer);
}

/*
 * enable or disable the video stream.
 */
int uvc_video_enable(struct uvc_streaming *stream, int enable)
{
	int ret;
	if(!enable) {
		uvc_uninit_video(stream);
		usb_set_interface(stream->dev->udev, stream->intfnum, 0);
		return 0;
	}

#if 0
	ret = uvc_video_clock_init(stream);
	if(ret < 0)
		return ret;
#endif

	usb_disable_asynch(1);	/* async transfer not allowed. */
	/* Commit the streaming parameters. */
	ret = uvc_commit_video(stream, &stream->ctrl);
	if(ret < 0)
		goto error_commit;

	ret = uvc_init_video(stream);
	if(ret < 0)
		goto error_video;

	usb_disable_asynch(0);
	return 0;

error_video:
	usb_set_interface(stream->dev->udev, stream->intfnum, 0);
error_commit:
	//uvc_queue_enable(&stream->queue, 0);

	usb_disable_asynch(0);
	return ret;
}

int uvc_video_init(struct uvc_streaming *stream)
{
	struct uvc_streaming_control *probe = &stream->ctrl;
	struct uvc_format *format = NULL;
	struct uvc_frame *frame = NULL;
	unsigned int i;
	int ret;

	if(stream->nformats == 0) {
		uvc_printk(KERN_INFO, "No supported video formats found.\n");
		return -EINVAL;
	}


	/* Alternate setting 0 should be the default, yet the XBox Live Vision
	 * Cam (and possibly other devices) crash or otherwise misbehave if
	 * they don't receive a SET_INTERFACE request before any other video
	 * control request.
	 */
	usb_set_interface(stream->dev->udev, stream->intfnum, 0);

	/* Set the streaming probe control with default streaming parameters
	 * retrieved from the device. Webcams that don't suport GET_DEF
	 * requests on the probe control will just keep their current streaming
	 * parameters.
	 */
	if (uvc_get_video_ctrl(stream, probe, 1, UVC_GET_DEF) == 0)
		uvc_set_video_ctrl(stream, probe, 1);

	/* Initialize the streaming parameters with the probe control current
	 * value. This makes sure SET_CUR requests on the streaming commit
	 * control will always use values retrieved from a successful GET_CUR
	 * request on the probe control, as required by the UVC specification
	 */
	ret = uvc_get_video_ctrl(stream, probe, 1, UVC_GET_CUR);
	if (ret < 0)
		return ret;

	/* Check if the default format descriptor exists. Use the first
	 * available format otherwise.
	 */
	for (i = stream->nformats; i > 0; --i) {
		format = &stream->format[i-1];
		if (format->index == probe->bFormatIndex)
			break;
	}

	if (format->nframes == 0) {
		uvc_printk(KERN_INFO, "No frame descriptor found for the "
				"default format.\n");
		return -EINVAL;
	}

	/* Zero bFrameIndex might be correct. Stream-based formats (including
	 * MPEG-2 TS and DV) do not support frames but have a dummy frame
	 * descriptor with bFrameIndex set to zero. If the default frame
	 * descriptor is not found, use the first available frame.
	 */
	for (i = format->nframes; i > 0; --i) {
		frame = &format->frame[i-1];
		if (frame->bFrameIndex == probe->bFrameIndex)
			break;
	}

	probe->bFormatIndex = format->index;
	probe->bFrameIndex = frame->bFrameIndex;

	stream->def_format = format;
	stream->cur_format = format;
	stream->cur_frame = frame;

	/* Select the video decoding function */
	if (stream->type == BUF_TYPE_VIDEO_CAPTURE) {
		if (stream->dev->quirks & UVC_QUIRK_BUILTIN_ISIGHT)
			stream->decode = uvc_video_decode_isight;
		else if (stream->intf->num_altsetting > 1)
			stream->decode = uvc_video_decode_isoc;
		else
			stream->decode = uvc_video_decode_bulk;
	} else {
		if (stream->intf->num_altsetting == 1)
			stream->decode = uvc_video_encode_bulk;
		else {
			uvc_printk(KERN_INFO, "Isochronous endpoints are not "
					"supported for video output devices.\n");
			return -EINVAL;
		}
	}

	dump_uvc_streaming_ctrl(probe);
	dump_uvc_streaming_format(stream);

	return 0;
}

/* init stream as a video stream*/
int uvc_video_probe(struct uvc_device *dev, struct uvc_streaming *stream)
{
	debug("uvc video probe...\n");
	int ret;

	ret = uvc_video_init(stream);
	if(ret)
		return ret;

	return 0;
}


/* Test ... */

/* Save Data to mmc 0.*/
static int uvc_debug_save_video_data(struct uvc_streaming *stream, char *filename, char *buffer, unsigned int len)
{
	long size;
	block_dev_desc_t *dev_desc = NULL;
	disk_partition_t info;
	int dev = 0;
	int part = 1;

#define DBG_IF	"mmc"
#define DBG_DEV "0"

	part = get_device_and_partition(DBG_IF, DBG_DEV, &dev_desc, &info, 1);
	if(part < 0)
		return 1;

	dev = dev_desc->dev;

	if (fat_set_blk_dev(dev_desc, &info) != 0) {
		printf("\n** Unable to use %s %d:%d for fatwrite **\n",
			DBG_IF, dev, part);
		return 1;
	}

	size = file_fat_write(filename, buffer, len);
	if (size == -1) {
		printf("\n** Unable to write \"%s\" from %s %d:%d **\n",
			filename, DBG_IF, DBG_DEV, part);
		return 1;
	}

	return 0;


}

static void uvc_video_complete(struct uvc_streaming *stream, struct uvc_buffer *buf)
{
	char filename[32];



	if(buf->error == 1 || buf->act_len == 0) {
		debug("Ignore error or empty frame stream->sequence %d, stream->fid %d\n", stream->sequence, stream->last_fid);
	} else {
		sprintf(filename, "uvc_video-%d.raw", stream->sequence);
		uvc_debug_save_video_data(stream, filename, buf->buffer, buf->act_len);
	}
}

int uvc_test(void)
{
	struct uvc_device *dev = uvc_get_device(0);
	int ret;

	/* 1. Init a stream */
	if(uvc_video_probe(dev, dev->video) < 0) {
		debug("Error Failed to probe uvc video device.!\n");
		return -1;
	}

	/* 2. enable stream */
	uvc_video_enable(dev->video, 1);

	/*3. process frame */
	while(1) {

		if(ctrlc()) {
			break;
		}
		ret = uvc_usb_request_data(dev->video, uvc_video_complete);
		if(ret < 0) {
			printf("failed to request data!\n");
			return -1;
		}


	}

	/*4. disable frame */
	uvc_video_enable(dev->video, 0);

	return 0;
}


