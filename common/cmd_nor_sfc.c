#include <common.h>
#include <command.h>
#include <asm/io.h>
#include <asm/arch/sfc.h>

extern void sfc_nor_load(unsigned int src_addr, unsigned int count,unsigned int dst_addr);

static int do_sfcnor(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned int src_addr,count,dst_addr,erase_en = 0,ret;
	if(argc < 4){
		return CMD_RET_USAGE;
	}
	if(!strcmp(argv[1],"read")){
		src_addr = simple_strtoul(argv[2],NULL,16);
		count = simple_strtoul(argv[3],NULL,16);
		dst_addr = simple_strtoul(argv[4],NULL,16);
		printf("sfcnor read Image from 0x%x to  0x%x size is 0x%x ...\n",src_addr,dst_addr,count);

#ifdef CONFIG_JZ_SECURE_SUPPORT
		printf("Security boot rsa...\n");
		ret = secure_boot_rsa_nku(src_addr);
		if(ret) {
			printf("ERROR:  check NKU failed!!\n");
			return CMD_RET_FAILURE;
		}
#endif
		sfc_nor_read(src_addr,count, dst_addr);
		printf("sfcnor read ok!\n");
		return 0;
	}else if(!strcmp(argv[1],"write")){

		src_addr = simple_strtoul(argv[2],NULL,16);
		count = simple_strtoul(argv[3],NULL,16);
		dst_addr = simple_strtoul(argv[4],NULL,16);
		erase_en = simple_strtoul(argv[5],NULL,16);
		printf("sfcnor write Image from 0x%x to  0x%x size is 0x%x erase_en = %d...\n",dst_addr,src_addr,count,erase_en);
		sfc_nor_write(src_addr,count,dst_addr);
		printf("sfcnor write ok!\n");
		return 0;

	}else if(!strcmp(argv[1],"erase")){

		src_addr = simple_strtoul(argv[2],NULL,16);
		count = simple_strtoul(argv[3],NULL,16);
		printf("sfcnor erase Image  0x%x  size is 0x%x ...\n",src_addr,count);
		sfc_nor_erase(src_addr,count);
		printf("sfcnor erase ok!\n");
		return 0;

	}else
		return CMD_RET_USAGE;
usage:
	return CMD_RET_USAGE;
}

U_BOOT_CMD(
	sfcnor, 6,	0,	do_sfcnor,
	"sfc nor",
	"sfcnor read   [src:nor flash addr] [bytes:0x..] [dst:ddr address]\n"
	"sfcnor write  [dst:nor flash addr] [bytes:0x..] [src:ddr address] [force erase:1, no erase:0]\n"
	"sfcnor erase  [src:nor flash addr] [bytes:0x..]\n "
);
