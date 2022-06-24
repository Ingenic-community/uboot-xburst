/*
 * MMC SPL check tool.
 *
 * Copyright (C) 2013 Ingenic Semiconductor Co.,Ltd
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

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <config.h>

#ifdef CONFIG_X2000
#define SPL_SIZE (24 * 1024)
#endif

#define BLOCK_SIZE 512
#define SPL_MAX_BLOCK (SPL_SIZE / BLOCK_SIZE)

#define BUFFER_SIZE 4
#define SPL_LENGTH_POSITION	4	/* 4th */

int main(int argc, char *argv[])
{
	int fd, count;
	int bytes_read;
	char buffer[BUFFER_SIZE];
	unsigned int check = 0;
	volatile int t = 0;

	if (argc != 2) {
		printf("Usage: %s fromfile tofile\n\a",argv[0]);
		return 1;
	}

	fd = open(argv[1], O_RDWR);
	if (fd < 0) {
		printf("Open %s Error\n", argv[1]);
		return 1;
	}

	count = 0;

	while ((bytes_read = read(fd, buffer, BUFFER_SIZE)) > 0) {
			count += bytes_read;
	}
	printf("mmc spl count = %08x Bytes\n", count);

	count = (count + BLOCK_SIZE - 1) / BLOCK_SIZE;
	if(count > SPL_MAX_BLOCK)
		count = SPL_MAX_BLOCK;
	printf("mmc spl count = %08x Blocks\n", count);

	/*set spl len*/
	lseek( fd, SPL_LENGTH_POSITION, SEEK_SET);
	if ((t = write(fd, &count, 4)) != 4) {
		printf("Check: Write %s Error\n",argv[1]);
		return 1;
	}

	close(fd);

	return 0;
}
