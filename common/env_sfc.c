/*
 * (C) Copyright 2008-2011 Freescale Semiconductor, Inc.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/* #define DEBUG */

#include <common.h>
#include <command.h>
#include <environment.h>
#include <linux/stddef.h>
#include <malloc.h>
#include <search.h>
#include <errno.h>
#include <asm/arch/spinor.h>

extern struct burner_params params;

DECLARE_GLOBAL_DATA_PTR;


__weak int sfc_get_env_addr(int copy, u32 *env_addr)
{
	s64 offset;

	offset = CONFIG_ENV_OFFSET;
#ifdef CONFIG_ENV_OFFSET_REDUND
	if (copy)
		offset = CONFIG_ENV_OFFSET_REDUND;
#endif


	*env_addr = offset;

	return 0;
}

int env_init(void)
{
	/* use default */
	gd->env_addr	= (ulong)&default_environment[0];
	gd->env_valid	= 1;

	return 0;
}

int saveenv(void)
{
	ALLOC_CACHE_ALIGN_BUFFER(char, buf, ENV_SIZE);
	env_t   env_new;
	env_t   *env_ptr;
	ssize_t len;
	char    *res;
	int   i, ret = 0, offset, conut = 0;
	int  copy = 0;
	int erase_size = params.spi_nor_info.erase_size;
	int erase_offset;
	char *r_buf = NULL;


	res = (char *)&env_new.data;
	env_ptr = (env_t *)buf;

	len = hexport_r(&env_htab, '\0', 0, &res, ENV_SIZE, 0, NULL);
	if (len < 0) {
		error("Cannot export environment: errno = %d\n", errno);
		return 1;
	}

#ifdef CONFIG_ENV_OFFSET_REDUND
	env_new->flags	= ++env_flags; /* increase the serial */

	if (gd->env_valid == 1)
		copy = 1;
#endif

	sfc_get_env_addr(copy, &offset);
	env_new.crc = crc32(0, env_new.data, ENV_SIZE);

	if (offset % erase_size) {
		erase_offset = ALIGN(offset, erase_size) - erase_size;
	} else {
		erase_offset = offset;
	}

	r_buf = malloc(erase_size);
	sfc_nor_read(erase_offset, erase_size, r_buf);
	memcpy(r_buf + (CONFIG_ENV_OFFSET - erase_offset), (char *)&env_new, CONFIG_ENV_SIZE);
	sfc_nor_erase(erase_offset, erase_size);
	sfc_nor_write(erase_offset, erase_size, r_buf);
	free(r_buf);

	sfc_nor_read(offset, CONFIG_ENV_SIZE , (char *)buf);

	env_ptr->crc = crc32(0, env_ptr->data, ENV_SIZE);

	for (i = 0; i < 0x0b0; i++){
		if (env_ptr->data[i] != env_new.data[i]){
			printf("env_ptr->data[%d] == 0x%x, env_new.data[%d] == 0x%x\n", i, env_ptr->data[i], i, env_new.data[i]);
			conut = 1;
		}
	}

	if (!conut)
		printf("save ok!!\n");

	return ret;

}
void env_relocate_spec(void)
{
	int copy = 0;
	u32 offset;
	ALLOC_CACHE_ALIGN_BUFFER(char, buf, ENV_SIZE);

	sfc_get_env_addr(copy, &offset);

	sfc_nor_read(offset, CONFIG_ENV_SIZE, (char *)buf);

	env_import(buf, 1);
}



