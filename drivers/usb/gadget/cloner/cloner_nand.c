#include <linux/mtd/nand.h>
#include <linux/mtd/mtd.h>
#include <nand.h>
#include <ingenic_nand_mgr/nand_param.h>

#ifdef CONFIG_JZ_NAND_MGR
int nand_program(struct cloner *cloner)
{
	u32 startaddr = cloner->cmd->write.partition + (cloner->cmd->write.offset);
	u32 length = cloner->cmd->write.length;
	void *databuf = (void *)cloner->write_req->buf;

	BURNNER_PRI("=========++++++++++++>   NAND PROGRAM:startaddr = %d P offset = %d P length = %d \n",startaddr,cloner->cmd->write.offset,length);
	do_nand_request(startaddr, databuf, length,cloner->cmd->write.offset);

	return 0;
}
#endif

#ifdef CONFIG_MTD_NAND_JZ
extern char *get_expart_name(int offset);
extern char *get_part_name(int offset);
extern int set_current_part(char *name);
extern int need_change_part(char *name);

extern int mtd_nand_probe_burner(PartitionInfo *pinfo, nand_flash_param *nand_params,
		int nr_nand_args, int eraseall, void ** title_fill, int *size);

int nand_mtd_ubi_program(struct cloner *cloner)
{
	u32 full_size = cloner->full_size;
	u32 length = cloner->cmd->write.length;
	char command[128];
	char *volume;
	void *databuf = (void *)cloner->write_req->buf;
	int ret = 0;
	int offset = cloner->cmd->write.partition;
	char *part_name = NULL;

	if (cloner->full_size_remainder == 0)
		return 0;

	if (cloner->full_size) {
		part_name = get_part_name(offset);
		if (need_change_part(part_name)) {
			memset(command, 0, 128);
			sprintf(command, "ubi part %s", part_name);
			BURNNER_PRI("%s\n", command);
			ret = run_command(command, 0);
			if (ret) {
				BURNNER_PRI("error...\n");
				return ret;
			}
			BURNNER_PRI("ok....\n");
			set_current_part(part_name);
			cloner->last_offset = 0;
			cloner->last_offset_avail = 0;
		}
	}

	BURNNER_PRI("cloner->last_offset_avail = %d,  cloner->last_offset = %x cloner->cmd->write.offset = %x\n",
			cloner->last_offset_avail,
			cloner->last_offset,
			cloner->cmd->write.offset);
	if (cloner->last_offset_avail &&
			cloner->last_offset ==
			cloner->cmd->write.offset)
		return 0;

	memset(command, 0, 128);
	volume = get_expart_name(offset);
	if (full_size && full_size <= length) {
		length = full_size;
		sprintf(command, "ubi write 0x%x %s 0x%x", (unsigned)databuf, volume, length);
	} else if (full_size) {
		sprintf(command, "ubi write.part 0x%x %s 0x%x 0x%x",
				(unsigned)databuf, volume, length, full_size);
	} else {
		sprintf(command, "ubi write.part 0x%x %s 0x%x",
				(unsigned)databuf, volume, length);
	}
	cloner->full_size_remainder -= length;
	if (cloner->full_size_remainder < 0) cloner->full_size_remainder = 0;
	BURNNER_PRI("%s (full_size_remainder reserved = %d)\n", command, cloner->full_size_remainder);
	ret = run_command(command, 0);
	if (ret) {
		BURNNER_PRI("...error\n");
	} else {
		BURNNER_PRI("...ok\n");
		cloner->last_offset_avail = 1;
		cloner->last_offset = cloner->cmd->write.offset;
	}
	cloner->full_size = 0;
	return ret;
}

extern void* get_params_addr(void);
extern int get_nand_pagesize(void);
extern int get_nand_blocksize(void);

static bool spl_is_complete_or_no_spl(u32 startaddr, u32 length)
{
	/*no spl*/
	if (startaddr >= CONFIG_SPL_PAD_TO)
		return true;

	/*spl is complete*/
	if (startaddr == 0 && length >= CONFIG_SPL_PAD_TO)
		return true;

	return false;
}

static int nand_mtd_raw_erase(u32 startaddr, u32 length)
{
	int erase_size = get_nand_blocksize();
	int page_size = get_nand_pagesize();
	int erase_len = length;
	u32 erase_addr = startaddr;
	u32 temp = 0;
	char command[128];
	int ret;

	if (erase_size < 0 || page_size < 0)
		return -1;

	/*has been erased before*/
	if (temp = (erase_addr%erase_size)) {
		if ((temp + length) > erase_size) {
			erase_addr = erase_addr - temp + erase_size;
			erase_len -= (erase_size - temp);
		} else
			return 0;
	}

	if (!startaddr)
		erase_len = (erase_len - CONFIG_SPL_PAD_TO) +
			(page_size * 128 * 8);
	erase_len += (erase_size - erase_len%erase_size);

	memset(command, 0 , 128);
	sprintf(command, "nand erase 0x%x 0x%x", erase_addr, erase_len);
	BURNNER_PRI("%s\n", command);
	ret = run_command(command, 0);
	if (ret)
		return ret;
	return 0;
}

static int nand_mtd_raw_program_spl(struct cloner *cloner, void *databuf) {
	int page_size = get_nand_pagesize();
	void *parambuf = NULL;
	char command[128];
	int ret = 0;

	if (page_size < 0) return -1;

	memcpy(databuf, cloner->spl_title, cloner->spl_title_sz);
	memset(command, 0 , 128);
	sprintf(command, "writespl 0x%x 0x%x", (unsigned)databuf, 6);
	BURNNER_PRI("%s\n", command);
	ret = run_command(command, 0);
	if (ret) return -1;

	parambuf = get_params_addr();
	memset(command, 0 , 128);
	sprintf(command, "writespl %p 0x%x 0x%x", parambuf, 6, 2);
	BURNNER_PRI("%s\n", command);
	ret = run_command(command, 0);
	if (ret) return -1;;

	cloner->skip_spl_size = page_size * 128 * 8 - CONFIG_SPL_PAD_TO;
	BURNNER_PRI("...ok\n");
	return 0;
}

int nand_mtd_raw_program(struct cloner *cloner)
{
	u32 length = cloner->cmd->write.length;
	void *databuf = (void *)cloner->write_req->buf;
	u32 startaddr = cloner->cmd->write.partition + (cloner->cmd->write.offset);
	char command[128];
	int ret = 0;

	if (!spl_is_complete_or_no_spl(startaddr, length))
		goto out;

	startaddr += cloner->skip_spl_size;

	if (!nand_args->nand_erase) {
		ret = nand_mtd_raw_erase(startaddr, length);
		if (ret) goto out;
	}

	if (startaddr == 0) {
		ret = nand_mtd_raw_program_spl(cloner, databuf);
		if (ret) goto out;
		length -= CONFIG_SPL_PAD_TO;
		databuf += CONFIG_SPL_PAD_TO;
		cloner->full_size_remainder -= CONFIG_SPL_PAD_TO;
		startaddr += cloner->skip_spl_size + CONFIG_SPL_PAD_TO;
	}

	cloner->full_size_remainder -= length;
	if (cloner->full_size_remainder <= 0) {
		cloner->skip_spl_size = 0;
		cloner->full_size_remainder = 0;
	}

	if (length) {
		memset(command, 0 , 128);
		sprintf(command, "nand write.skip 0x%x 0x%x 0x%x", (unsigned)databuf, startaddr, length);
		BURNNER_PRI("%s\n", command);
		ret = run_command(command, 0);
		if (ret) goto out;
	}
	cloner->full_size = 0;
	BURNNER_PRI("...ok\n");
	return 0;
out:
	BURNNER_PRI("...error\n");
	return ret;
}
#endif
