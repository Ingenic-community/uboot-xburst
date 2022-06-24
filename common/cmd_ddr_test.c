#include <common.h>
#include <command.h>
#include <malloc.h>
#include <asm/errno.h>
#include <asm/string.h>
#include <asm/arch/clk.h>
#include <asm/arch/base.h>

#define WRITE_FLAG 0
#define READ_FLAG 1
#define MEMCPY_FLAG 2
#define CACHE_FLAG 0
#define UNCACHE_FLAG 1
#define REG32(addr) *(volatile unsigned int*)(addr)
static void ddr_count_start()
{
#ifdef CONFIG_X1000
	unsigned int val;
	val = REG32(0xb00000d0);
	val |= (1<<6);
	REG32(0xb00000d0) = val;
	printf("ddr_drcg = 0x%x\n", REG32(0xb00000d0));
#endif
	REG32((DDRC_BASE + 0xd4)) = 0;
	REG32((DDRC_BASE + 0xd8)) = 0;
	REG32((DDRC_BASE + 0xdc)) = 0;
	REG32((DDRC_BASE + 0xe4)) = 3;
}

static void ddr_count_stop()
{
	unsigned int i,j,k;

	i = 0;
	j = 0;
	k = 0;

	REG32((DDRC_BASE + 0xe4)) = 2;
	i = REG32((DDRC_BASE + 0xd4));
	j = REG32((DDRC_BASE + 0xd8));
	k = REG32((DDRC_BASE + 0xdc));
	printf("total_cycle = %d,valid_cycle = %d\n", i, j);
	printf("rate      = %%%d\n", j * 100 / i);
	printf("idle_rate = %%%d\n\n", k * 100 / i);
}

static void ddr_read(unsigned int *addr, unsigned int size, unsigned int times)
{
	int i, n;
	char *tmp_addr;

	for(i = 0; i < times; i++) {
		tmp_addr = addr;
		for(n = 0; n < size; n += 64) {
			__asm__ __volatile__(
				"lw $0, 0(%0)\n\t"
				"lw $0, 4(%0)\n\t"
				"lw $0, 8(%0)\n\t"
				"lw $0, 12(%0)\n\t"
				"lw $0, 16(%0)\n\t"
				"lw $0, 20(%0)\n\t"
				"lw $0, 24(%0)\n\t"
				"lw $0, 28(%0)\n\t"
				"lw $0, 32(%0)\n\t"
				"lw $0, 36(%0)\n\t"
				"lw $0, 40(%0)\n\t"
				"lw $0, 44(%0)\n\t"
				"lw $0, 48(%0)\n\t"
				"lw $0, 52(%0)\n\t"
				"lw $0, 56(%0)\n\t"
				"lw $0, 60(%0)\n\t"
				:
				:"r"(tmp_addr)
				);
			tmp_addr += 64;
		}
	}
}
static void ddr_write(unsigned int *addr, unsigned int size, unsigned int times)
{
	int i;

	for(i = 0;i < times; i++) {
		memset(addr, 0xff, size);
	}
}
static void ddr_memcpy(char * src_addr, char * dst_addr, unsigned int size, unsigned int times)
{
	int i;

	for(i = 0; i < times; i++) {
		memcpy(dst_addr, src_addr, size);
	}
}

static int err_print(unsigned int num)
{
	printf("%d argument\n",  num);
	printf("ddr_wr w/r/p [size] [k/m] [times] [cache/uncache]\n");
	return CMD_RET_USAGE;
}
static int ddr_wr_test(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned int wr_flag = READ_FLAG;
	unsigned int cache_flag = CACHE_FLAG;
	unsigned int size = 4;
	unsigned int unit = 1024 * 1024;
	unsigned int times = 100;
	unsigned int *src_addr, *dst_addr;
	unsigned int start, end;
	int i;

	if (argc < 2)
		return err_print(0);

	printf("current ddr test:\n");
	printf("ddr_wr ");
	for(i = 1; i < argc; i++) {
		switch(i) {
		case 1:
			if(!strcmp (argv[i], "w")) {
				wr_flag = WRITE_FLAG;
				printf("w ");
			} else if(!strcmp (argv[i], "r")) {
				wr_flag = READ_FLAG;
				printf("r ");
			} else if(!strcmp (argv[i], "p")) {
				wr_flag = MEMCPY_FLAG;
				printf("p ");
			} else {
				return err_print(i);
			}
			break;
		case 2:
			size = simple_strtoul(argv[i], NULL, 10);
			break;
		case 3:
			if(!strcmp (argv[i], "K") || !strcmp (argv[i], "k"))
				unit = 1024;
			else if(!strcmp (argv[i], "M") || !strcmp (argv[i], "m"))
				unit = 1024 * 1024;
			else
				return err_print(i);
			break;
		case 4:
			times = simple_strtoul(argv[i], NULL, 10);
			break;
		case 5:
			if(!strcmp (argv[i], "cache"))
				cache_flag = CACHE_FLAG;
			else if(!strcmp (argv[i], "uncache"))
				cache_flag = UNCACHE_FLAG;
			else
				return err_print(i);
			break;
		default:
			printf("./ddr_wr w/r/p [size] [k/m] [times] [cache/uncache]");
			return CMD_RET_USAGE;
		}
	}
	if(unit == 1024)
		printf("%d K times = %d ", size, times);
	else
		printf("%d M times = %d ", size, times);

	size *= unit;
	src_addr = (unsigned int *)malloc(size);
	if(src_addr == NULL) {
		printf("src: malloc faile\n");
		return CMD_RET_USAGE;
	}
	if(wr_flag == MEMCPY_FLAG) {
		dst_addr = (unsigned int *)malloc(size);
		if(dst_addr == NULL) {
			printf("dst:malloc faile\n");
			return CMD_RET_USAGE;
		}
	}
	if(cache_flag == UNCACHE_FLAG) {
		src_addr = (unsigned int *)(((unsigned int)src_addr) | 0xa0000000);
		dst_addr = (unsigned int *)(((unsigned int)dst_addr) | 0xa0000000);
		printf("uncache\n");
		printf("src_addr = %x\n", (unsigned int)src_addr);
		printf("dst_addr = %x\n", (unsigned int)dst_addr);
	} else {
		printf("cache\n");
		printf("src_addr = %x\n", (unsigned int)src_addr);
	}


	start = get_timer(0);
	printf("current %d us\n",start);
	ddr_count_start();
	if(wr_flag == WRITE_FLAG)
		ddr_write(src_addr, size, times);
	else if(wr_flag == READ_FLAG)
		ddr_read(src_addr, size, times);
	else if(wr_flag == MEMCPY_FLAG)
		ddr_memcpy(src_addr, dst_addr, size, times);
	ddr_count_stop();
	end = get_timer(0);
	printf("current %d us\n",end);
	printf("spend %d us\n",end - start);

	free(src_addr);
	if(wr_flag == MEMCPY_FLAG)
		free(dst_addr);
	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(ddr_wr, 5, 2, ddr_wr_test,
	"Ingenic ddr read write test",
	"ddr_wr w/r/p [size] [k/m] [times] [cache/uncache]\n"
);
