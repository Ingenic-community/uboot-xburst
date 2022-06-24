/*
 * Copyright 2014 Broadcom Corporation.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
struct disk_storge_msg {
	unsigned int    start;	/* # of first block in partition	*/
	unsigned int	size;	/* number of blocks in partition	*/
	ulong	blksz;		/* block size in bytes			*/
	char	*name;	/* partition name			*/
};

int fb_mmc_flash_write(const struct disk_storge_msg *storge_msg, unsigned int session_id,
			void *download_buffer, unsigned int download_bytes);
