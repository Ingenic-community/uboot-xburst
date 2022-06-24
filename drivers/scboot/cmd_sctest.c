/*
 * Ingenic sc test command
 *
 * Copyright (c) 2013 pzqi <aric.pzqi@ingenic.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>
#include <asm/errno.h>


/*sc_test */


static int do_sctest(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	if(argc < 2) {
		return CMD_RET_USAGE;
	}

	if(strcmp(argv[1], "init") == 0) {
		printf("calling sc_test init function !\n");
		if(init_seboot() < 0) {
			printf("init seboot fialed.\n");
		}

	} else if(strcmp(argv[1], "scboot") == 0) {
		if(test_scboot() < 0) {
			printf("scboot failed !!!!\n");
		}
		printf("calling sc_test scboot funtion !\n");
	} else if(strcmp(argv[1], "rsa") == 0) {
		if(test_rsa() < 0) {
			printf("at list one test case failed in test rsa !\n");
		}
	} else if(strcmp(argv[1], "burn_nku") == 0) {
		test_burnnku();
	} else if(strcmp(argv[1], "burn_ukey") == 0) {
		test_burnukey();
	} else if(strcmp(argv[1], "burn_rckey") == 0){
		cpu_burn_rckey();
	} else if(strcmp(argv[1], "get_rn") == 0){
		cpu_get_rn();
	} else if(strcmp(argv[1], "get_enckey") == 0) {
		test_get_enckey();
	}else if (strcmp(argv[1], "sec_boot_en") == 0){
		printf("burn sec boot enable bit:\n");
		cpu_burn_secboot_enable();
	}else if (strcmp(argv[1], "load_nku") == 0){
		printf("load test nku to mem!\n");
		test_load_nku();
	}else if (strcmp(argv[1], "aes_byckey") == 0){
		printf("test aes by ckey\n");
		test_aes_by_ckey();
	}else if (strcmp(argv[1], "chipid") == 0){
		printf("test aes by ckey\n");
		test_read_chip_id();
	}else if (strcmp(argv[1], "all") == 0){
		printf("test all in one\n");
		init_seboot();
		cpu_burn_rckey();
		test_burnnku();
		test_get_enckey();
		test_burnukey();

	} else {

		printf("cmd error!!\n");
	}

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(sctest, 2, 1, do_sctest,
	"Ingenic security test program",
	"sctest init -- load firmware to pdma and se-rom.\n"
	"sctest scboot -- test scboot function.\n"
	"sctest xxx	-- test to be add!!\n"
);
