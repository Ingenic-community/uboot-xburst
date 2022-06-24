/*
 * Ingenic burner function Code (Vendor Private Protocol)
 *
 * Copyright (c) 2013 cli <cli@ingenic.cn>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
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

#include <errno.h>
#include <common.h>
#include <malloc.h>
#include <rtc.h>
#include <part.h>
#include <spi.h>
#include <spi_flash.h>
#include <ingenic_soft_i2c.h>
#include <ingenic_soft_spi.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <linux/compiler.h>
#include <linux/usb/composite.h>
#include <cloner/cloner.h>
#include "cloner/burn_printf.h"
#include "cloner/cloner_mmc.c"
#include "cloner/cloner_efuse.c"
#ifdef CONFIG_JZ_SCBOOT
#include "../../scboot/secure.h"
#include "../../scboot/otp.h"
#include "../../scboot/aes.h"
#include "../../scboot/spi_checksum.h"

static bool is_security = false;
static bool is_bootfile = false;
#endif


/**
 * cloner module manage
 **/
static LIST_HEAD(clmg_list);

int register_cloner_moudle(struct cloner_moudle *clmd)
{
	if (!clmd)
		return -EINVAL;
	list_add_tail(&clmd->node, &clmg_list);
	return 0;
}

static inline struct cloner_moudle *find_cloner_moudle_ops(int ops) {

	struct cloner_moudle *pos = NULL;
	list_for_each_entry(pos, &clmg_list, node)
		if (pos->ops == ops)
			return pos;
	return NULL;
}


static inline struct cloner_moudle *find_cloner_moudle_medium(uint32_t medium) {

	struct cloner_moudle *pos = NULL;
	list_for_each_entry(pos, &clmg_list, node)
		if (pos->medium == medium)
			return pos;
	return NULL;
}

int clmg_init(struct cloner *cloner, void *args)
{
	struct cloner_moudle *clmd;

	if (!args)
		return -EINVAL;

	struct ParameterInfo *p = (struct ParameterInfo *)args;

	clmd = find_cloner_moudle_medium(p->magic);
	if (!clmd)
		return -ENOSYS;

	if (!clmd->init) {
		printf("moudle(%x) not support init function\n", clmd->medium);
		return 0;
	}
	return clmd->init(cloner, (void*)p->data, clmd->data);
}

int clmg_info(struct cloner *cloner)
{
	struct cloner_moudle *clmd = NULL;

	list_for_each_entry(clmd, &clmg_list, node)
	{
		if (!clmd || !clmd->info)
			continue;
		clmd->info(cloner);
	}
	return 0;
}

int clmg_write(struct cloner *cloner)
{
	struct cloner_moudle *clmd;
	int ops;

	if (!cloner)
		return -EINVAL;

	ops = MOUDLE_TYPE(cloner->cmd->write.ops);

	clmd = find_cloner_moudle_ops(ops);
	if (!clmd || !clmd->write)
		return -ENOSYS;

	return clmd->write(cloner, MOUDLE_SUB_TYPE(cloner->cmd->write.ops), clmd->data);
}

int clmg_read(struct cloner *cloner)
{
	struct cloner_moudle *clmd;
	int ops;

	if (!cloner)
		return -EINVAL;

	ops = MOUDLE_TYPE(cloner->cmd->read.ops);

	clmd = find_cloner_moudle_ops(ops);
	if (!clmd || !clmd->read)
		return -ENOSYS;

	return clmd->read(cloner, MOUDLE_SUB_TYPE(cloner->cmd->read.ops), clmd->data);
}

int clmg_check(struct cloner *cloner)
{
	struct cloner_moudle *clmd;
	int ops;

	if (!cloner)
		return -EINVAL;

	ops = MOUDLE_TYPE(cloner->cmd->check.ops);

	clmd = find_cloner_moudle_ops(ops);
	if (!clmd || !clmd->check)
		return -ENOSYS;

	return clmd->check(cloner, MOUDLE_SUB_TYPE(cloner->cmd->check.ops), clmd->data);
}


int i2c_program(struct cloner *cloner)
{
	int i = 0;
	struct i2c_args *i2c_arg = (struct i2c_args *)cloner->write_req->buf;
	struct i2c i2c;
	i2c.scl = i2c_arg->clk;
	i2c.sda = i2c_arg->data;
	i2c_init(&i2c);

	for(i=0;i<i2c_arg->value_count;i++) {
		char reg = i2c_arg->value[i] >> 16;
		unsigned char value = i2c_arg->value[i] & 0xff;
		i2c_write(&i2c,i2c_arg->device,reg,1,&value,1);
	}
	return 0;
}


struct ParameterInfo	*global_args;
struct policy_param	*policy_args;
struct debug_param	*debug_args;

int cloner_init(struct cloner *cloner)
{
	int i = 0;
	struct ParameterInfo *p = global_args;
	struct ParameterInfo	*m = NULL;
	while(1)
	{
		if(((int)p%4==0) && ((char*)p>=(char*)global_args)
				&& ((char*)p<((char*)global_args+ARGS_LEN)) && (p->magic != 0))
		{
			printf("magic=");
			for(i=3;i>=0;i--)
				printf("%c",((char*)&p->magic)[i]);
			printf("\n");
			switch(p->magic)
			{
				case MAGIC_POLICY:
					policy_args = p->data;
					break;
				case MAGIC_DEBUG:
					debug_args = p->data;
					L.enable = debug_args->log_enabled;
					break;
				case MAGIC_SFC:
				case MAGIC_MMC:
				case MAGIC_NAND:
					m = p;
					break;
				default:
					printf("Unknown magic!!!\n");
					break;
			}
		}
		else
		{
			break;
		}
		p = (struct ParameterInfo *)((char *)p + p->size + sizeof(uint32_t) * 2);
	}
	return clmg_init(cloner, m);
}

void *realloc_buf(struct cloner *cloner, size_t realloc_size)
{
	if (unlikely(cloner->buf_size < realloc_size)) {
		cloner->buf_size = realloc_size;
		cloner->buf = realloc(cloner->buf,cloner->buf_size);
		cloner->write_req->buf = cloner->read_req->buf = cloner->buf;
	}
	return cloner->buf;
}

void handle_read(struct cloner *cloner)
{
#define OPS(x,y) ((x<<16)|(y&0xffff))
	switch(cloner->cmd->read.ops) {
#ifdef CONFIG_JZ_SCBOOT
	case OPS_GET_ENCK:                             //4. send enckey to pc burner
		cloner->ack = cpu_get_enckey(cloner->read_req->buf);
		break;
#endif
	default:
		cloner->ack = clmg_read(cloner);
		break;
	}

	if (debug_args->transfer_data_chk)
		cloner->crc = local_crc32(0xffffffff, cloner->read_req->buf, cloner->cmd->read.length);
	//printf("handle read cloner->crc %x\n", cloner->crc);

	/*always transfer data*/
	cloner->read_req->length = cloner->cmd->read.length;
	//usb_ep_queue(cloner->ep_in, cloner->read_req, 0);
#undef OPS
}

void handle_read_complete(struct usb_ep *ep,struct usb_request *req)
{
}

int handle_check(struct cloner *cloner)
{
	return clmg_check(cloner);
}

void handle_write(struct usb_ep *ep,struct usb_request *req)
{
	struct cloner *cloner = req->context;

	if(req->status == -ECONNRESET) {
		cloner->ack = -ECONNRESET;
		return;
	}

	if (req->actual != req->length) {
		printf("write transfer length is err,actual=%08x,length=%08x\n",req->actual,req->length);
		cloner->ack = -EIO;
		return;
	}

	if(cloner->cmd_type == VR_UPDATE_CFG) {
		cloner->ack = 0;
		return;
	}

	if (debug_args->transfer_data_chk) {
		uint32_t tmp_crc = local_crc32(0xffffffff,req->buf,req->actual);
		if (cloner->cmd->write.crc != tmp_crc) {
			printf("crc is errr! src crc=%08x crc=%08x\n",cloner->cmd->write.crc,tmp_crc);
			cloner->ack = -EINVAL;
			return;
		}
	}

#define OPS(x,y) ((x<<16)|(y&0xffff))
	switch(cloner->cmd->write.ops) {
		case OPS(I2C,RAW):
			cloner->ack = i2c_program(cloner);
			break;
#ifdef CONFIG_CMD_EFUSE
		case OPS(EFUSE,RAW):
			cloner->ack = efuse_program(cloner);
			break;
#endif
		case OPS(MEMORY,RAW):
			{
				unsigned char *dest_addr = (void *)(cloner->cmd->write.partition + cloner->cmd->write.offset);
				unsigned char *src_addr = cloner->write_req->buf;
				unsigned int len = cloner->cmd->write.length;
				//memset(dest_addr, 0, len);
				memcpy(dest_addr, src_addr, len);
				if(debug_args->write_back_chk)
				{
					printf("src:%p----dest:%p-----len:%d\n",src_addr, dest_addr, len);
					int i=0;
					for(; i < len; i++)
					{
						if(dest_addr[i] != src_addr[i])
						{
							printf("compare error: dest_addr:0x%p = 0x%02x,src_addr:0x%p = 0x%02x\n",
									dest_addr+i, dest_addr[i], src_addr+i, src_addr[i]);
							break;
						}
					}
				}
			}
			cloner->ack = 0;
			break;
		case OPS(REGISTER,RAW):
			{
				volatile unsigned int *tmp = (void *)cloner->cmd->write.partition;
				if((unsigned)tmp > 0xb0000000 && (unsigned)tmp < 0xb8000000) {
					*tmp = *((int*)cloner->write_req->buf);
					cloner->ack = 0;
				} else {
					printf("OPS(REGISTER,RAW): not supported address.");
					cloner->ack = -ENODEV;
				}
			}
			break;
#ifdef CONFIG_JZ_SCBOOT
		case OPS_BURN_NKU:     //3.uboot recv nku and burn
			cloner->ack = cpu_burn_nku(cloner->write_req->buf,cloner->cmd->write.length);
			break;
		case OPS_BURN_ENUK:     //5.recv encrtpy ukey and burn
			cloner->ack = cpu_burn_ukey(cloner->write_req->buf);
			break;
#endif
		default:
			cloner->ack = clmg_write(cloner);
	}
	memset(cloner->write_req->buf,0,cloner->write_req->length);
#undef OPS
}

#ifdef CONFIG_FPGA
extern int do_udc_reset(void);
#endif

extern void burner_set_reset_tag(void);
void handle_cmd(struct usb_ep *ep,struct usb_request *req)
{
	struct cloner *cloner = req->context;
	if(req->status == -ECONNRESET) {
		cloner->ack = -ECONNRESET;
		return;
	}

	if (req->actual != req->length) {
		printf("cmd transfer length is err req->actual = %d, req->length = %d\n",
				req->actual,req->length);
		cloner->ack = -EIO;
		return;
	}

	union cmd *cmd = req->buf;
	debug_cond(BURNNER_DEBUG,"handle_cmd type=%x\n",cloner->cmd_type);
	switch(cloner->cmd_type) {
		case VR_UPDATE_CFG:
			cloner->args_req->length = cmd->update.length;
			usb_ep_queue(cloner->ep_out, cloner->args_req, 0);
			break;
		case VR_WRITE:
			realloc_buf(cloner, cmd->write.length);
			cloner->write_req->length = cmd->write.length;
			usb_ep_queue(cloner->ep_out, cloner->write_req, 0);
			break;
		case VR_INIT:
			if(!cloner->inited) {
				cloner->ack = -EBUSY;
				cloner_init(cloner);
				cloner->inited = 1;
				cloner->ack = 0;
			}
			break;
		case VR_READ:
			handle_read(cloner);
			break;
		case VR_GET_CRC:
			if (!cloner->ack)
				usb_ep_queue(cloner->ep_in, cloner->read_req, 0);
			break;
		case VR_SYNC_TIME:
			cloner->ack = rtc_set(&cloner->cmd->rtc);
			break;
		case VR_CHECK:
			cloner->ack = handle_check(cloner);
			break;
		case VR_GET_CHIP_ID:
		case VR_GET_USER_ID:
		case VR_GET_ACK:
		case VR_GET_CPU_INFO:
		case VR_SET_DATA_ADDR:
		case VR_SET_DATA_LEN:
			break;
		case VR_REBOOT:
#ifdef CONFIG_FPGA
			mdelay(1000);
			do_udc_reset();
			mdelay(10000);
#endif
			do_reset(NULL,0,0,NULL);
			break;
		case VR_POWEROFF:
			burner_set_reset_tag();
			do_reset(NULL,0,0,NULL);
			break;
#ifdef CONFIG_JZ_SCBOOT
		case VR_SEC_SEDEN:
			if(cloner->cmd->security.security_en)
				is_security = true;
			else
				is_security = false;
			break;
		case VR_SEC_INIT:  //1.init security boot
			cloner->ack = init_seboot();
			break;
		case VR_SEC_BURN_RKCK://2.burn rckey
			cloner->ack = cpu_burn_rckey();
			break;
		case VR_SEC_BURN_SECBOOT_EN:	//6.burn secboot_enable
			cloner->ack = cpu_burn_secboot_enable();
			//cloner->ack = 0;
			break;
#endif
	}
}

int f_cloner_setup_handle(struct usb_function *f,
		const struct usb_ctrlrequest *ctlreq)
{
	struct cloner *cloner = f->config->cdev->req->context;
	struct usb_request *req = cloner->ep0req;

	if ((ctlreq->bRequestType & USB_TYPE_MASK) != USB_TYPE_VENDOR) {
		printf("Unkown RequestType 0x%x \n",ctlreq->bRequestType);
		cloner->ack = -ENOSYS;
		return -ENOSYS;
	}

	usb_ep_dequeue(cloner->ep0, cloner->ep0req);
	usb_ep_dequeue(cloner->ep_in, cloner->read_req);
	usb_ep_dequeue(cloner->ep_out, cloner->write_req);

	cloner->cmd_type = ctlreq->bRequest;
	req->length = ctlreq->wLength;
	req->complete = handle_cmd;

	switch (ctlreq->bRequest) {
		case VR_GET_CPU_INFO:
			strcpy(cloner->ep0req->buf,CONFIG_BURNER_CPU_INFO);
			break;
		case VR_GET_ACK:
			if (cloner->ack) printf("cloner->ack = %d\n",cloner->ack);
			memcpy(cloner->ep0req->buf,&cloner->ack,sizeof(int));
			break;
		case VR_GET_CRC:
			if (cloner->ack) printf("cloner->ack = %d, cloner->crc = %x\n",cloner->ack, cloner->crc);
			memcpy(cloner->ep0req->buf,&cloner->ack,sizeof(int));
			memcpy(cloner->ep0req->buf + sizeof(int),&cloner->crc,sizeof(int));
			break;
		case VR_INIT:
			break;
		case VR_GET_FLASH_INFO:
			cloner->ack = clmg_info(cloner);
			break;
		case VR_UPDATE_CFG:
		case VR_WRITE:
			cloner->ack = -EBUSY;
			break;
#ifdef CONFIG_CMD_EFUSE
		case VR_GET_CHIP_ID:
		case VR_GET_USER_ID:
			cloner->ep0req->length = ctlreq->wLength;
			cloner->ack = efuse_program(cloner);
			break;
#endif
		case VR_SET_DATA_ADDR:
		case VR_SET_DATA_LEN:
			cloner->full_size = ctlreq->wIndex | ctlreq->wValue << 16;
			cloner->full_size_remainder = cloner->full_size;
#ifdef CONFIG_JZ_SCBOOT
			if(cloner->full_size < 0x100000)
				is_bootfile = true;
			else
				is_bootfile = false;
#endif
			printf("cloner->full_size = %x\n", cloner->full_size);
			break;
#ifdef CONFIG_JZ_SCBOOT
		case VR_SEC_GET_CK_LEN:     //4. get ckey length
			*(unsigned int *)cloner->ep0req->buf = get_rsakeylen();
			break;
#endif
	}

	return usb_ep_queue(cloner->ep0, cloner->ep0req, 0);
}

int f_cloner_bind(struct usb_configuration *c,
		struct usb_function *f)
{
	struct usb_composite_dev *cdev = c->cdev;
	struct cloner *cloner = func_to_cloner(f);

	debug_cond(BURNNER_DEBUG,"f_cloner_bind\n");

	intf_desc.bInterfaceNumber = usb_interface_id(c, f);
	if(intf_desc.bInterfaceNumber < 0 )
		return intf_desc.bInterfaceNumber;

	cloner->ep0 = cdev->gadget->ep0;
	cloner->ep0req = cdev->req;
	cloner->gadget = cdev->gadget;
	cloner->ack = 0;
	cloner->cdev = cdev;

	cloner->cmd = (union cmd *)cloner->ep0req->buf;

	if (gadget_is_dualspeed(cdev->gadget)) {
		hs_bulk_in_desc.bEndpointAddress =
			fs_bulk_in_desc.bEndpointAddress;
		hs_bulk_out_desc.bEndpointAddress =
			fs_bulk_out_desc.bEndpointAddress;
	}

	cdev->req->context = cloner;

	cloner->ep_in = usb_ep_autoconfig(cdev->gadget, &fs_bulk_in_desc);
	cloner->ep_out = usb_ep_autoconfig(cdev->gadget, &fs_bulk_out_desc);

	cloner->write_req = usb_ep_alloc_request(cloner->ep_out,0);
	cloner->args_req = usb_ep_alloc_request(cloner->ep_out,0);
	cloner->read_req = usb_ep_alloc_request(cloner->ep_in,0);

	cloner->buf_size = ARGS_LEN;
	cloner->buf = calloc(1,ARGS_LEN);
	cloner->write_req->complete = handle_write;
	cloner->write_req->buf = cloner->buf;
	cloner->write_req->length = ARGS_LEN;
	cloner->write_req->context = cloner;

	cloner->args = calloc(1,ARGS_LEN);
	global_args = (struct ParameterInfo*)(cloner->args);
	cloner->args_req->complete = handle_write;
	cloner->args_req->buf = cloner->args;
	cloner->args_req->length = ARGS_LEN;
	cloner->args_req->context = cloner;

	cloner->read_req->complete = handle_read_complete;
	cloner->read_req->buf = cloner->buf;
	cloner->read_req->length = ARGS_LEN;
	cloner->read_req->context = cloner;

	return 0;
}

int f_cloner_set_alt(struct usb_function *f,
		unsigned interface, unsigned alt)
{
	struct cloner *cloner = func_to_cloner(f);
	const struct usb_endpoint_descriptor *epin_desc,*epout_desc;
	int status = 0;

	debug_cond(BURNNER_DEBUG,"set interface %d alt %d\n",interface,alt);
	epin_desc = ep_choose(cloner->gadget,&hs_bulk_in_desc,&fs_bulk_in_desc);
	epout_desc = ep_choose(cloner->gadget,&hs_bulk_out_desc,&fs_bulk_out_desc);

	status += usb_ep_enable(cloner->ep_in,epin_desc);
	status += usb_ep_enable(cloner->ep_out,epout_desc);

	if (status < 0) {
		printf("usb enable ep failed\n");
		goto failed;
	}

	cloner->ep_in->driver_data = cloner;
	cloner->ep_out->driver_data = cloner;
failed:
	return status;
}

void f_cloner_unbind(struct usb_configuration *c,struct usb_function *f)
{
}

void f_cloner_disable(struct usb_function *f)
{
	struct cloner *cloner = func_to_cloner(f);
	int status = 0;
	status += usb_ep_disable(cloner->ep_in);
	status += usb_ep_disable(cloner->ep_out);
	if (status < 0)
		printf("usb disable ep failed");
	return;
}

int cloner_function_bind_config(struct usb_configuration *c)
{
	int status = 0;
	struct cloner *cloner = calloc(sizeof(struct cloner),1);

	if (!cloner)
		return -ENOMEM;

	cloner->usb_function.name = "vendor burnner interface";
	cloner->usb_function.bind = f_cloner_bind;
	cloner->usb_function.hs_descriptors = hs_intf_descs;
	cloner->usb_function.descriptors = fs_intf_descs;
	cloner->usb_function.set_alt = f_cloner_set_alt;
	cloner->usb_function.setup = f_cloner_setup_handle;
	cloner->usb_function.strings= burn_intf_string_tab;
	cloner->usb_function.disable = f_cloner_disable;
	cloner->usb_function.unbind = f_cloner_unbind;
	cloner->inited = 0;

	if (cloner_moudle_init())
		return -EINVAL;

	INIT_LIST_HEAD(&cloner->usb_function.list);
	bitmap_zero(cloner->usb_function.endpoints,32);

	status =  usb_add_function(c,&cloner->usb_function);
	if (status)
		free(cloner);
	return status;
}

int jz_cloner_add(struct usb_configuration *c)
{
	int id;

	id = usb_string_id(c->cdev);
	if (id < 0)
		return id;
	burner_intf_string_defs[0].id = id;
	intf_desc.iInterface = id;

	debug_cond(BURNNER_DEBUG,"%s: cdev: 0x%p gadget:0x%p gadget->ep0: 0x%p\n", __func__,
			c->cdev, c->cdev->gadget, c->cdev->gadget->ep0);

	return cloner_function_bind_config(c);
}
