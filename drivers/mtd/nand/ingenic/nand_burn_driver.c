/*
 * jz mtd nand driver probe interface
 *
 *  Copyright (C) 2013 Ingenic Semiconductor Co., LTD.
 *  Author: cli <chen.li@ingenic.com>
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
#define DEBUG
#include <common.h>
#include <malloc.h>
#include <errno.h>
#include <nand.h>
#include <linux/list.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <asm/arch/nand.h>
#include <ingenic_nand_mgr/nand_param.h>

extern int mtd_nand_auto_init(nand_flash_param* nand_param);

#ifdef MTDIDS_DEFAULT
static const char *const mtdids_default = MTDIDS_DEFAULT;
#else
static const char *const mtdids_default = "nand0:nand";
#endif

static void* zalloc(size_t size) {
	void *p = 0;

	p = malloc(size);
	memset(p, 0, size);
	return p;
}
/*---------------------nand param idx-----------------------------------------------------------------*/
static int __nand_id(int chip) {
	return 0x2c68;
}
int jz_get_nand_id(int chip) __attribute__((weak, alias("__nand_id")));

static int nand_params_idx(nand_flash_param *nand_params, int max_params) {
	int param_index;
	int chip_id = jz_get_nand_id(0);
	debug("chip_id %x,max_params %d\n", chip_id, max_params);
	for (param_index = 0; param_index < max_params; param_index++) {
		debug("nand_params[%d].id %x\n",
				param_index,
				nand_params[param_index].id);
		if (nand_params[param_index].id == chip_id)
			break;
	}
	if (param_index == max_params) {
		printf("\033[31m [E]nand id %x not support \033[0m", chip_id);
		return -EINVAL;
	}

	return param_index;
}
/*-----------------------spl title-------------------------------------------------------------------*/
#if defined(CONFIG_JZ4775) || defined(CONFIG_4780)
static int mtd_nand_get_spl_title(nand_flash_param *nand_params, void **pad_buf, int *size) {
	char *pbuf;
	int i, pos;
	char pad, pad1, pad2;

	if (!nand_params)
		return -EINVAL;

	pbuf = *pad_buf = zalloc(256);
	if (*pad_buf == NULL)
		return -ENOMEM;
	*size = 256;

	debug("buswidth = %d\nnand_params->nandtype= %d\nnand_params->rowcycles=%d\nnand_params->pagesize = %d\n",
			nand_params->buswidth,
			nand_params->nandtype,
			nand_params->rowcycles,
			nand_params->pagesize);

	if (nand_params->buswidth == 16)
		pad = 0xaa;
	else
		pad = 0x55;
	for (i = 0, pos = 0; i < 64; i++)
		pbuf[pos++] = pad;

	if (nand_params->nandtype == 1)
		pad = 0xaa;
	else
		pad = 0x55;
	for (i= 0; i < 64; i++)
		pbuf[pos++] = pad;

	if (nand_params->rowcycles == 2)
		pad = 0x55;
	else
		pad = 0xaa;
	for (i = 0; i < 32; i++)
		pbuf[pos++] = pad;

	switch (nand_params->pagesize) {
	case 2048:
		pad2 = 0x55;
		pad1 = 0xaa;
		pad = 0x55;
		break;
	case 4096:
		pad2 = 0xaa;
		pad1 = 0x55;
		pad = 0x55;
		break;
	case 8192:
		pad2 = 0xaa;
		pad1 = 0xaa;
		pad = 0x55;
		break;
	case 16384:
		pad2 =0xaa;
		pad1 = 0xaa;
		pad = 0xaa;
		break;
	default:
	case 512:
		pad2 = 0x55;
		pad1 = 0x55;
		pad = 0x55;
		break;
	}

	for (i = 0; i < 32; i++)
		pbuf[pos++] = pad2;
	for (i = 0; i < 32; i++)
		pbuf[pos++] = pad1;
	for (i = 0; i < 32; i++)
		pbuf[pos++] = pad;
	return 0;
}
#else /*CONFIG_JZ4775 || CONFIG_JZ4780*/
static int mtd_nand_get_spl_title(nand_flash_param *nand_params, void **pad_buf, int *size) {
	char *pbuf1;
	int i, pos;
	char pad, pad1, pad2;
	int *pbuf2;

	if (!nand_params)
		return -EINVAL;

	pbuf1 = *pad_buf = zalloc(512);
	if (*pad_buf == NULL)
		return -ENOMEM;
	*size = 512;
	debug("buswidth = %d\nnand_params->nandtype= %d\nnand_params->rowcycles=%d\nnand_params->pagesize = %d\n",
			nand_params->buswidth,
			nand_params->nandtype,
			nand_params->rowcycles,
			nand_params->pagesize);

	if (nand_params->buswidth == 16)
		pad = 0xaa;		/*16bit buswidth*/
	else
		pad = 0x55;		/*8bit buswidth*/
	for (i = 0, pos = 0; i < 64; i++)
		pbuf1[pos++] = pad;

	if (nand_params->nandtype == 1)
		pad = 0xaa;		/*toggle nand*/
	else
		pad = 0x55;		/*common nand*/
	for (i= 0; i < 64; i++)
		pbuf1[pos++] = pad;

	pbuf2 = (int *)(&pbuf1[pos]);
	pbuf2[0] = 0x214c5053;		/*magic code 'SPL!'*/
	pbuf2[1] = nand_params->rowcycles;
	pbuf2[2] = nand_params->pagesize;
	return 0;
}
#endif

/*--------------------------------------------partition----------------------------------------*/
struct mtd_expart_info {
	char *part_name;
	int offset;
	struct mtd_part_info *parent;
};

struct mtd_part_info {
	char *part_name;
	int offset;
	int size;
	struct mtd_expart_info expart_info[MUL_PARTS];
};

static struct mtd_part_info part_info[MAX_PART_NUM];
static char *current_part = NULL;

char *get_expart_name(int offset) {
	struct mtd_expart_info *expart_info = NULL;
	int ex_moffset = 0;
	int i;
	for (i = 0; i < MAX_PART_NUM; i++) {
		if (part_info[i].size != -1) {
			debug("part_info[i].offset = %d  part_info[i].size = %d offset =%d\n",
					part_info[i].offset, part_info[i].size, offset);
			if (part_info[i].offset + part_info[i].size - 1 > offset/1024/1024) {
				expart_info = part_info[i].expart_info;
				ex_moffset = (offset/1024/1024) - part_info[i].offset;
				break;
			}
		} else {
			expart_info = part_info[i].expart_info;
			ex_moffset = (offset/1024/1024) - part_info[i].offset;
			break;
		}
	}
	if (!expart_info)  return NULL;
	for (i = 0; i < MUL_PARTS; i++) {
		if (ex_moffset == expart_info[i].offset)
			return expart_info[i].part_name;
	}
	return NULL;
}

char *get_part_name(int offset)
{
	int i;
	for (i = 0; i < MAX_PART_NUM; i++) {
		if (part_info[i].size != -1) {
			debug("part_info[i].offset = %d  part_info[i].size = %d offset =%d\n",
					part_info[i].offset, part_info[i].size, offset);
			if (part_info[i].offset + part_info[i].size - 1 > offset/1024/1024)
				return part_info[i].part_name;
		} else {
			return part_info[i].part_name;
		}
	}
	return NULL;
}

int set_current_part(char *name) {
	current_part = name;
	return 0;
}

int need_change_part(char *name) {
	return !(name == current_part);
}

static void dump_partition_info(void) {
	int i,j;

	printf("current_part %s------------\n", current_part);
	for (i = 0; i < MAX_PART_NUM ;i++) {
		printf("Main Part offset %d:%s\n", part_info[i].offset, part_info[i].part_name);
		for (j = 0; j < MUL_PARTS && !!part_info[i].expart_info[j].part_name; j++) {
			printf("\tExtern part  %d:%s\n",
					part_info[i].expart_info[j].offset,
					part_info[i].expart_info[j].part_name);
		}
	}
	printf("---------------------------\n");
}
#define X_ENV_LENGTH		1024
#define X_COMMAND_LENGTH	128
static int mtd_nand_burner_part(MTDPartitionInfo *pinfo, int eraseall) {
	MTDNandppt *nandppt = pinfo->ndppt;
	char mtdparts_env[X_ENV_LENGTH];
	char command[X_COMMAND_LENGTH];
	int part, ret;

	memset(mtdparts_env, 0, X_ENV_LENGTH);
	memset(command, 0, X_COMMAND_LENGTH);

	/*MTD part*/
	sprintf(mtdparts_env, "mtdparts=nand:");
	for (part = 0; part < MAX_PART_NUM && nandppt[part].size; part++) {
		debug("nandppt[%d].name %s, nandppt[%d].size = %d,  nandppt[%d].managermode = %d\n",
				part , nandppt[part].name, part, nandppt[part].size, part,
				nandppt[part].managermode);
		if (nandppt[part].size == -1) {
			sprintf(mtdparts_env,"%s-(%s)", mtdparts_env,
					nandppt[part].name);
			break;
		} else if (nandppt[part].size != 0) {
			sprintf(mtdparts_env,"%s%dM(%s),", mtdparts_env,
					nandppt[part].size,
					nandppt[part].name);
		} else
			break;
	}
	debug("env:mtdparts=%s\n", mtdparts_env);
	setenv("mtdids", mtdids_default);
	setenv("mtdparts", mtdparts_env);
	setenv("partition", NULL);

	/*UBI part*/
	memset(part_info, 0, (MAX_PART_NUM *sizeof(struct mtd_part_info)));
	set_current_part(NULL);

	for (part = 0; part < MAX_PART_NUM && nandppt[part].size; part++) {
		int expart = 0;

		debug("nandppt[%d].name %s, nandppt[%d].size = %d\n",
				part , nandppt[part].name, part, nandppt[part].size);
		part_info[part].part_name = zalloc(strlen(nandppt[part].name) + 1);
		if (!part_info[part].part_name) return -ENOMEM;
		sprintf(part_info[part].part_name, "%s", nandppt[part].name);
		part_info[part].offset =  nandppt[part].offset;
		part_info[part].size =  nandppt[part].size;

		if (nandppt[part].managermode != 1)	/*select ubi part*/
			continue;

		if (!!eraseall) {
			sprintf(command,"ubi part %s", part_info[part].part_name);
			debug("%s\n", command);
			ret = run_command(command , 0);
			memset(command, 0, X_COMMAND_LENGTH);
			if (ret) return ret;
			set_current_part(part_info[part].part_name);
		}

		for (expart = 0; expart < MUL_PARTS && nandppt[part].ui_ex_partition[expart].size; expart++) {
			struct mtd_expart_info  *p_expart_info = NULL;
			int64_t size;

			p_expart_info = &part_info[part].expart_info[expart];
			p_expart_info->part_name = zalloc(strlen(nandppt[part].ui_ex_partition[expart].name) + 1);
			if (!p_expart_info->part_name) return -ENOMEM;
			sprintf(p_expart_info->part_name, "%s", nandppt[part].ui_ex_partition[expart].name);
			p_expart_info->offset = nandppt[part].ui_ex_partition[expart].offset;
			p_expart_info->parent = &part_info[part];

			if (!!eraseall) {
				if (nandppt[part].ui_ex_partition[expart].size == -1) {
					sprintf(command, "ubi create %s",
							nandppt[part].ui_ex_partition[expart].name);
					ret = run_command(command, 0);
					memset(command, 0, X_COMMAND_LENGTH);
					break;
				} else if (nandppt[part].ui_ex_partition[expart].size) {
					size = nandppt[part].ui_ex_partition[expart].size * 1024ull * 1024ull;
					sprintf(command, "ubi create %s 0x%llx",
							nandppt[part].ui_ex_partition[expart].name,
							size);
					debug("%s\n", command);
					ret = run_command(command, 0);
					memset(command, 0, X_COMMAND_LENGTH);
					if (ret) return ret;
				} else
					break;
			}
		}

		if (nandppt[part].size == -1)
			break;
	}
	dump_partition_info();
	return ret;
}
/*----------------------------------nand param---------------------------------------------*/
static void *mtd_nand_params = NULL;
void* get_params_addr(void) {
	return mtd_nand_params;
}

int get_nand_pagesize(void) {
	struct mtd_info *mtd = &nand_info[0];

	if (mtd->writesize)
		return mtd->writesize;
	else
		return -ENODEV;
}

int get_nand_blocksize(void) {
	struct mtd_info *mtd = &nand_info[0];

	if (mtd->erasesize)
		return mtd->erasesize;
	else
		return -ENODEV;
}

void dump_params(void)
{
	struct mtd_nand_params *params = (struct mtd_nand_params *)mtd_nand_params;
	nand_flash_param *nand_params = &params->nand_params;
	MTDPartitionInfo *pinfo = &params->pinfo;
	MTDNandppt* ndppt = pinfo->ndppt;
	int i, j;

	printf("nand_params.name %s\n", nand_params->name);
	printf("nand_params.totalblocks %d\n", nand_params->totalblocks);
	printf("nand_params.buswid %d\n", nand_params->buswidth);
	printf("nand_params.pagesize %d\n", nand_params->pagesize);
	printf("nand_params.eccbit %d\n", nand_params->eccbit);
	printf("nand_params.oobsize %d\n", nand_params->oobsize);

	for (i = 0; i < MAX_PART_NUM; i++) {
		printf("Main part %s  off:size %dM:%dM\n", ndppt[i].name, ndppt[i].offset, ndppt[i].size);
		for (j = 0; j < MUL_PARTS; j++) {
			ui_plat_ex_partition *expart = ndppt[i].ui_ex_partition;
			printf("\texpart %s off:size %dM:%dM\n", expart[j].name, expart[j].offset, expart[j].size);
		}
	}
}
/*---------------------------nand probe--------------------------------------------------*/
int mtd_nand_probe_burner(MTDPartitionInfo *pinfo, nand_flash_param *nand_params,
		int nr_nand_args, int eraseall, void **title_fill, int *size)
{
	int wppin = pinfo->gpio_wp;
	nand_flash_param *nand_param = NULL;
	int ret;

	/*mtd: disable write protect*/
	if (wppin != -1)
		gpio_direction_output(wppin, 0);

	/*mtd: get nand param from params table*/
	ret = nand_params_idx(nand_params, nr_nand_args);
	if (ret < 0)
		return ret;
	nand_param = &nand_params[ret];

	/*mtd: prepare spl title*/
	ret = mtd_nand_get_spl_title(nand_param, title_fill, size);
	if (ret)
		return ret;

	/*mtd: nand probe*/
	ret = mtd_nand_auto_init(nand_param);
	if (ret) return ret;

	/*mtd: params prepare to burn*/
	mtd_nand_params = zalloc(CONFIG_SPL_PAD_TO);
	if (!mtd_nand_params) return -ENOMEM;
	memcpy(mtd_nand_params, nand_param, sizeof(nand_flash_param));
	memcpy(mtd_nand_params+sizeof(nand_flash_param), pinfo, sizeof(MTDPartitionInfo));
	dump_params();

	/*mtd: nand erase all*/
	if (eraseall == 1)
		ret = run_command("nand scrub.chip -y", 0);
	else if (eraseall == 2)
		ret = run_command("nand erase.chip", 0);

	/*mtd: partition*/
	return mtd_nand_burner_part(pinfo, eraseall);
}
