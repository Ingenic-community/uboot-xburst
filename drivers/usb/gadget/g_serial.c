/* Ingenic Fastboot Command Explain Gadget Driver
 *
 *  Copyright (C) 2013 Ingenic Semiconductor Co., LTD.
 *  Sun Jiwei <jwsun@ingenic.cn>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <errno.h>
#include <common.h>
#include <malloc.h>

#include <linux/usb/composite.h>

#include "gadget_chips.h"
#include "composite.c"

#define GS_VERSION_STR			"v2.4"
#define GS_VERSION_NUM			0x2400

#define GS_LONG_NAME			"Gadget Serial"
#define GS_VERSION_NAME			GS_LONG_NAME " " GS_VERSION_STR

/*-------------------------------------------------------------------------*/
USB_GADGET_COMPOSITE_OPTIONS();

/* Thanks to NetChip Technologies for donating this product ID.
*
* DO NOT REUSE THESE IDs with a protocol-incompatible driver!!  Ever!!
* Instead:  allocate your own, using normal USB-IF procedures.
*/
#define GS_VENDOR_ID			0x0525	/* NetChip */
#define GS_PRODUCT_ID			0xa4a6	/* Linux-USB Serial Gadget */
#define GS_CDC_PRODUCT_ID		0xa4a7	/* ... as CDC-ACM */
#define GS_CDC_OBEX_PRODUCT_ID		0xa4a9	/* ... as CDC-OBEX */

/* string IDs are assigned dynamically */

#define STRING_DESCRIPTION_IDX		USB_GADGET_FIRST_AVAIL_IDX
#define USB_CLASS_COMM			2

enum {
	USB_GADGET_MANUFACTURER_IDX	= 0,
	USB_GADGET_PRODUCT_IDX,
	USB_GADGET_SERIAL_IDX,
	USB_GADGET_FIRST_AVAIL_IDX,
	ACM_CTRL_IDX,
	ACM_DATA_IDX,
	ACM_IAD_IDX,
};

static struct usb_string strings_dev[] = {
	[USB_GADGET_MANUFACTURER_IDX].s = "Linux 3.10.14-00004-g79978f3-dirty with dwc2-gadget",
	[USB_GADGET_PRODUCT_IDX].s = GS_VERSION_NAME,
	[USB_GADGET_SERIAL_IDX].s = "",
	[STRING_DESCRIPTION_IDX].s = NULL /* updated; f(use_acm) */,
	[ACM_CTRL_IDX].s = "CDC Abstract Control Model (ACM)",
	[ACM_DATA_IDX].s = "CDC ACM Data",
	[ACM_IAD_IDX ].s = "CDC Serial",
	{  } /* end of list */
};

static struct usb_gadget_strings stringtab_dev = {
	.language	= 0x0409,	/* en-us */
	.strings	= strings_dev,
};

static struct usb_gadget_strings *dev_strings[] = {
	&stringtab_dev,
	NULL,
};

static struct usb_device_descriptor device_desc = {
	.bLength =		USB_DT_DEVICE_SIZE,
	.bDescriptorType =	USB_DT_DEVICE,
	.bcdUSB =		cpu_to_le16(0x0200),
	/* .bDeviceClass = f(use_acm) */
	.bDeviceSubClass =	0,
	.bDeviceProtocol =	0,
	/* .bMaxPacketSize0 = f(hardware) */
	.idVendor =		cpu_to_le16(GS_VENDOR_ID),
	/* .idProduct =	f(use_acm) */
	.bcdDevice = cpu_to_le16(GS_VERSION_NUM),
	/* .iManufacturer = DYNAMIC */
	/* .iProduct = DYNAMIC */
	.bNumConfigurations =	1,
};

static struct usb_otg_descriptor otg_descriptor = {
	.bLength =		sizeof otg_descriptor,
	.bDescriptorType =	USB_DT_OTG,

	/* REVISIT SRP-only hardware is possible, although
	 * it would not be called "OTG" ...
	 */
	.bmAttributes =		USB_OTG_SRP | USB_OTG_HNP,
};

static const struct usb_descriptor_header *otg_desc[] = {
	(struct usb_descriptor_header *) &otg_descriptor,
	NULL,
};

extern int gser_bind_config(struct usb_configuration *c);
static struct usb_configuration serial_config_driver = {
	/* .label = f(use_acm) */
	/* .bConfigurationValue = f(use_acm) */
	/* .iConfiguration = DYNAMIC */
	.bmAttributes	= USB_CONFIG_ATT_SELFPOWER,
	.bind = gser_bind_config,
};

static int g_serial_unbind(struct usb_composite_dev *cdev)
{
	struct usb_gadget *gadget = cdev->gadget;

	usb_gadget_disconnect(gadget);
	return 0;
}

static int g_serial_do_config(struct usb_configuration *c)
{
	const char *s = c->cdev->driver->name;
	int ret = 0;

	debug("%s, %d,  version= %s\n", __func__, __LINE__, s);
	gser_bind_config(c);

	return ret;
}

#define STRING_MANUFACTURER 25
#define STRING_PRODUCT 2
#define STRING_USBDOWN 0
#define CONFIG_USBDOWNLOADER 1


static int g_serial_register(struct usb_composite_dev *cdev)
{
	debug("%s---%d, iConfiguration = %d, bConfigurationValue = %d, bmAttributes = 0x%x\n", __func__, __LINE__,
			serial_config_driver.iConfiguration, serial_config_driver.bConfigurationValue, serial_config_driver.bmAttributes);
	return usb_add_config(cdev, &serial_config_driver);
}


static int g_serial_bind(struct usb_composite_dev *cdev)
{
	struct usb_gadget *gadget = cdev->gadget;
	int id, ret;
	int gcnum;
	int status;
	int i = 0;

	status = usb_string_ids_tab(cdev, strings_dev);
	if (status < 0)
		goto error;

	device_desc.iManufacturer = strings_dev[USB_GADGET_MANUFACTURER_IDX].id;
	device_desc.iProduct = strings_dev[USB_GADGET_PRODUCT_IDX].id;
	status = strings_dev[STRING_DESCRIPTION_IDX].id;
	serial_config_driver.iConfiguration = status;

	if (gadget_is_otg(cdev->gadget)) {
		serial_config_driver.descriptors = otg_desc;
		serial_config_driver.bmAttributes |= USB_CONFIG_ATT_WAKEUP;
	}

	ret = g_serial_register(cdev);
	if (ret)
		goto error;
	gcnum = usb_gadget_controller_number(gadget);
	if (gcnum >= 0)
		device_desc.bcdDevice = cpu_to_le16(0x0200 + gcnum);
	else {
		device_desc.bcdDevice = __constant_cpu_to_le16(0x9999);
	}

	usb_gadget_connect(gadget);

	return 0;

 error:
	printf("%s--%d, error\n", __func__, __LINE__);
	g_serial_unbind(cdev);
	return -ENOMEM;
}

static struct usb_composite_driver g_serial_driver = {
	.name = "usb_serial",
	.dev = &device_desc,
	.strings = dev_strings,
	.bind = g_serial_bind,
	.unbind = g_serial_unbind,
};

int jz_usb_serial_register(const char *type)
{
	/* We only allow "dfu" atm, so 3 should be enough */
	int ret = 0;

	serial_config_driver.label = "CDC ACM config";
	serial_config_driver.bConfigurationValue = 2;
	device_desc.bDeviceClass = USB_CLASS_COMM;
	device_desc.idProduct = cpu_to_le16(GS_CDC_PRODUCT_ID);
	strings_dev[STRING_DESCRIPTION_IDX].s = serial_config_driver.label;

	ret = usb_composite_register(&g_serial_driver);
	if (ret) {
		printf("%s: failed!, error: %d\n", __func__, ret);
		return ret;
	}

	return 0;
}

void jz_usb_serial_unregister(void)
{
	usb_composite_unregister(&g_serial_driver);
}
