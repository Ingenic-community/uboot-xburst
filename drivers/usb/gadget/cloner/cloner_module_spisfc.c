#include <cloner/cloner.h>
#include "burn_printf.h"

static char *readbuf = NULL;
extern struct ParameterInfo	*global_args;
struct spi_param *spi_args;

#ifdef CONFIG_MTD_SFCNOR
#include "cloner_sfcnor.c"
#endif
#ifdef CONFIG_MTD_SPINAND
#include "cloner_spinand.c"
#endif
#ifdef CONFIG_MTD_SFCNAND
#include "cloner_sfcnand.c"
#endif

int buf_compare(unsigned char *org_data,unsigned char *read_data,unsigned int len,unsigned int offset)
{
	unsigned int i,val = 0;
	unsigned int *buf1 = (unsigned int *)org_data;
	unsigned int *buf2 = (unsigned int *)read_data;
	for(i = 0; i < len / 4; i++)
	{
		if(buf1[i] != buf2[i]){
			printf("XXXXXXXXXX  compare error: org_data[%d] = 0x%08x read_data[%d] = 0x%08x addr= 0x%08x  len = %d\n",
					i, buf1[i], i, buf2[i], offset + i * 4, len);
			val = -1;
		}
	}
	return val;
}

int clmd_spisfc_info(struct cloner *cloner)
{
	int id_code = 0;

	if(global_args->magic == MAGIC_POLICY){
		policy_args = global_args->data;
	} else {
		printf("ERR: cloner send policy data error!\n");
		memset(global_args->data, 0, sizeof(*policy_args));
	}
#ifdef CONFIG_MTD_SFCNOR
	if(policy_args->use_sfc_nor){
		id_code = get_norflash_id();
	}
#endif
	if(id_code < 0) {
		printf("ERR : (get flash_info) try id err, %d\n", id_code);
		id_code = 0;
	}

	memcpy(cloner->ep0req->buf, &id_code, sizeof(unsigned int));
	return id_code;
}

int clmd_spisfc_init(struct cloner *cloner, void *args, void *ops_data)
{
	spi_args = (struct spi_param *)args;
	if(!spi_args)
	{
		printf("Not found sfc parameters (%s)\n",__func__);
		return -EINVAL;
	}
	int ret = 0;

	if(!policy_args)
	{
		printf("Not fount policy parameters (%s)\n",__func__);
		return -EINVAL;
	}

#ifdef CONFIG_MTD_SFCNOR
	if(policy_args->use_sfc_nor){
		ret = norflash_get_params_from_burner((unsigned char *)spi_args + sizeof(struct spi_param));
		if (spi_args->spi_erase == SPI_ERASE_PART) {
			sfc_erase();
		}
	}

#endif
#ifdef CONFIG_MTD_SFCNAND
	if(policy_args->use_sfc_nand){
		ret = mtd_sfcnand_probe_burner(&(spi_args->spi_erase),spi_args->sfc_quad_mode,spi_args->flash_info);
		if (!ret)
			get_burner_nandinfo(spi_args->flash_info);
	}
#endif
#ifdef CONFIG_MTD_SPINAND
	if(policy_args->use_spi_nand){
		get_burner_nandinfo(spi_args->flash_info, &nand_param_from_burner);
		mtd_spinand_probe_burner(&(spi_args->spi_erase),&nand_param_from_burner);
	}
#endif
	return ret;
}

int clmd_spisfc_write(struct cloner *cloner, int sub_type, void *ops_data)
{
	int ret = 0;
	switch(sub_type)
	{
#ifdef CONFIG_MTD_SFCNOR
		case SFC_NOR:
			ret = sfc_program(cloner);
			break;
#endif
#if defined(CONFIG_MTD_SPINAND) || defined(CONFIG_MTD_SFCNAND)
		case SPI_NAND:
		case SFC_NAND:
			ret = spinand_program(cloner);
			break;
#endif

#ifdef CONFIG_JZ_SPINAND_SN
		case SFC_NAND_SN_WRITE:
			ret = spinand_sn_program(cloner);
			break;
#endif

#ifdef CONFIG_JZ_SPINAND_MAC
		case SFC_NAND_MAC_WRITE:
			ret = spinand_mac_program(cloner);
			break;
#endif
		default:
			printf("Not found sfc sub_type!\n");
			return -EINVAL;
	}
	return ret;
}

static int32_t clmd_spisfc_read(struct cloner *cloner, int sub_type, void *ops_data) {

	int32_t ret = 0;

	switch(sub_type)
	{
#ifdef CONFIG_JZ_SPINAND_SN
		case SFC_NAND_SN_READ:
			ret = spinand_sn_read(cloner);
			break;
#endif

#ifdef CONFIG_JZ_SPINAND_MAC
		case SFC_NAND_MAC_READ:
			ret = spinand_mac_read(cloner);
			break;
#endif
		default:
			printf("Not found sfc sub_type!\n");
			return -EINVAL;
	}

	return ret;
}
int cloner_spisfc_init(void)
{
	struct cloner_moudle *clmd = malloc(sizeof(struct cloner_moudle));
	int ret;

	if (!clmd)
		return -ENOMEM;
	clmd->medium = MAGIC_SFC;
	clmd->ops = SPISFC;
	clmd->write = clmd_spisfc_write;
	clmd->init = clmd_spisfc_init;
	clmd->info = clmd_spisfc_info;
	clmd->read = clmd_spisfc_read;
	clmd->check = NULL;
	clmd->data = NULL;
	printf("cloner spisfc register\n");
	return register_cloner_moudle(clmd);
}
CLONER_MOUDLE_INIT(cloner_spisfc_init);
