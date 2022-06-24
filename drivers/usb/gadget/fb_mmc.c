/*
 * Ingenic JZ Fastboot Command Explain Function Driver
 *
 *  Copyright (C) 2013 Ingenic Semiconductor Co., LTD.
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
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <common.h>
#include <errno.h>
#include <fastboot.h>
#include <fb_mmc.h>
#include <image-sparse.h>
#include <part.h>
#include <sparse_format.h>
#include <mmc.h>
#include <div64.h>

#define _DEBUG	0

static int fb_mmc_sparse_write(struct sparse_storage *storage,
			       unsigned int offset,
			       unsigned int size,
			       char *data)
{
	char command[128];
	memset(command , 0, 128);
	sprintf(command,"mmc write %p %x %x",data,offset,size);
	run_command(command,"0");
	debug("mmc flash OK!\n");
	return size;
}

static int  write_raw_image(struct disk_storge_msg *storge_msg, void *buffer,
		unsigned int download_bytes)
{
	unsigned int  blkcnt;
	char command[128];

	/* determine number of blocks to write */
	blkcnt = ((download_bytes + (storge_msg->blksz - 1)) & ~(storge_msg->blksz - 1));
	blkcnt = lldiv(blkcnt, storge_msg->blksz);

	if (blkcnt > storge_msg->size) {
		printf("too large for partition: '%s'\n", storge_msg->name);
		return -1;
	}

	debug("Flashing Raw Image\n");

	memset(command , 0, 128);
	sprintf(command,"mmc write %p %x %x",buffer, storge_msg->start, blkcnt);
	run_command(command,"0");
	debug("mmc flash OK!\n");
	return 0;
}

int fb_mmc_flash_write(const struct disk_storge_msg *storge_msg, unsigned int session_id,
			void *download_buffer, unsigned int download_bytes)
{
	if (is_sparse_image(download_buffer)) {
		sparse_storage_t sparse;

		sparse.block_sz = storge_msg->blksz;
		sparse.start = storge_msg->start;
		sparse.size = storge_msg->size;
		sparse.name = storge_msg->name;
		sparse.write = fb_mmc_sparse_write;

		if(store_sparse_image(&sparse, session_id, download_buffer))
			return -1;
	} else {
		if(write_raw_image(storge_msg, download_buffer,
				download_bytes)) {
			printf("Flashing Raw %s Image failed!\n", storge_msg->name);
			return -1;
		}
	}

	return 0;
}
