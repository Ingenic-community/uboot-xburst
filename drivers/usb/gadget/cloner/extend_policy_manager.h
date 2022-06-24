#ifndef __EXPY_MG_H__
#define __EXPY_MG_H__
#include <cloner/cloner.h>
#include <linux/types.h>
#include <linux/list.h>

struct extend_policy {
	uint32_t magic;
	struct list_head node;
	int (*write)(void *buf, int length, void *data);
	int (*read)(void *buf, int length, void *data);
	void *data;
};

int epmg_register(struct extend_policy *expy);
#endif /*__EXPY_MG_H__*/
