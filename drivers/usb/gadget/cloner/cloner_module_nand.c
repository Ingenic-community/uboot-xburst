#include <cloner/cloner.h>
#include "burn_printf.h"
#include "cloner_nand.c"

struct nand_param *nand_args;

static int clmd_nand_init(struct cloner *cloner, void *args, void *mdata)
{
	int ret = 0;
	nand_args = (struct nand_param *)args;
	if(!nand_args)
	{
		printf("Not found nand parameters\n");
		return -EINVAL;
	}
	if(!policy_args)
	{
		printf("Not fount policy parameters (%s)\n",__func__);
		return -EINVAL;
	}
#ifdef CONFIG_JZ_NAND_MGR
	if(policy_args->use_nand_mgr) {
		ret = nand_probe_burner(&(nand_args->PartInfo),
				&(nand_args->nand_params[0]),
				nand_args->nr_nand_args,
				nand_args->nand_erase,policy_args->offsets,nand_args->nand_erase_count);
	}
#endif
#ifdef CONFIG_MTD_NAND_JZ
	if(policy_args->use_nand_mtd) {
		ret = mtd_nand_probe_burner(&nand_args->MTDPartInfo,
				&nand_args->nand_params,
				nand_args->nr_nand_args,
				nand_args->nand_erase,
				&cloner->spl_title,
				&cloner->spl_title_sz);
	}
#endif

	return ret;
}

int clmd_nand_write(struct cloner *cloner,int sub_type, void* ops_data)
{
	int ret = 0;
	switch(sub_type) {
		case RAW:
			break;
#ifdef CONFIG_JZ_NAND_MGR
		case IMAGE:
			ret = nand_program(cloner);
			break;
#endif
#ifdef CONFIG_MTD_NAND_JZ
		case MTD_RAW:
			ret = nand_mtd_raw_program(cloner);
			break;
		case MTD_UBI:
			ret = nand_mtd_ubi_program(cloner);
			break;
#endif
		default:
			printf("Not found nand sub_type.\n");
			return -EINVAL;
	}
	return ret;
}

int cloner_nand_init(void)
{
	struct cloner_moudle *clmd = malloc(sizeof(struct cloner_moudle));
	int ret;

	if (!clmd)
		return -ENOMEM;
	clmd->medium = MAGIC_NAND;
	clmd->ops = NAND;
	clmd->init = clmd_nand_init;
	clmd->write = clmd_nand_write;
	clmd->read = NULL;
	clmd->info = NULL;
	clmd->check = NULL;
	clmd->data = NULL;
	printf("cloner nand register\n");
	return register_cloner_moudle(clmd);
}
CLONER_MOUDLE_INIT(cloner_nand_init);
