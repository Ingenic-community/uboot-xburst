#include <linux/list.h>
#include <malloc.h>
#include <cloner/cloner.h>
#include "extend_policy_manager.h"

static LIST_HEAD(epmg_list);
static bool inited = false;

struct extend_policy *find_extend_policy(uint32_t magic)
{
	struct extend_policy *pos;
	list_for_each_entry(pos, &epmg_list, node)
		if (pos->magic == magic)
			return pos;
	return NULL;
}

int epmg_write(struct cloner *cloner, int sub_type, void *ops_data)
{
	struct extend_policy* expy = NULL;
	uint32_t magic = 0;

	if (!cloner)
		return -EINVAL;

	magic = (cloner->cmd->write.partition & 0xffffffff);
	expy = find_extend_policy(magic);
	if (!expy)
		return -ENOSYS;
	return expy->write(cloner->write_req->buf, cloner->cmd->write.length, expy->data);
}

int epmg_read(struct cloner *cloner, int sub_type, void *ops_data)
{
	struct extend_policy* expy = NULL;
	uint32_t magic = 0;

	if (!cloner)
		return -EINVAL;

	magic = (cloner->cmd->write.partition & 0xffffffff);
	expy = find_extend_policy(magic);
	if (!expy)
		return -ENOSYS;

	return expy->read(cloner->read_req->buf, cloner->cmd->read.length, expy->data);
}

int epmg_register(struct extend_policy *expy)
{
	if (!expy || !expy->read || !expy->write)
		return -EINVAL;

	if (!inited)
		return -ENOSYS;
	list_add_tail(&expy->node, &epmg_list);
	return 0;
}

int extend_policy_mg_init(void)
{
	struct cloner_moudle *clmd = malloc(sizeof(struct cloner_moudle));
	int ret;

	if (!clmd)
		return -ENOMEM;
	clmd->medium = MAGIC_EXPY; 
	clmd->ops = EXT_POL;
	clmd->read = epmg_read;
	clmd->write = epmg_write;
	clmd->init = NULL;
	clmd->check = NULL;
	clmd->info = NULL;
	clmd->data = NULL;
	printf("extend policy manager register\n");
	ret = register_cloner_moudle(clmd);
	if (!ret)
		inited = true;
	return ret;
}
CLONER_MOUDLE_INIT(extend_policy_mg_init);
