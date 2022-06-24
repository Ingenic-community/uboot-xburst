/*
 * JZ4775 common routines
 *
 * Copyright (c) 2013 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
 * Based on: arch/mips/cpu/xburst/jz4780/jz4780.c
 *           Written by Paul Burton <paul.burton@imgtec.com>
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

/*#define DEBUG*/
#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <asm/mipsregs.h>
#include <asm/arch/clk.h>
#include <asm/arch/cpm.h>
#include <spl.h>
#include <regulator.h>
#ifdef CONFIG_HW_WATCHDOG
#include <watchdog.h>
#endif
#ifdef CONFIG_AUDIO_CAL_DIV
#include <generated/audio_div_values.h>
#endif
#ifdef CONFIG_SPL_BUILD

/* Pointer to as well as the global data structure for SPL */
DECLARE_GLOBAL_DATA_PTR;
gd_t gdata __attribute__ ((section(".data")));

#ifndef CONFIG_BURNER
#include <generated/ddr_reg_values.h>
struct global_info ginfo __attribute__ ((section(".data"))) = {
	.extal		= CONFIG_SYS_EXTAL,
	.cpufreq	= CONFIG_SYS_CPU_FREQ,
	.ddrfreq	= CONFIG_SYS_MEM_FREQ,
	.uart_idx	= CONFIG_SYS_UART_INDEX,
	.baud_rate	= CONFIG_BAUDRATE,

	.ddr_change_param = {
		DDRC_CFG_VALUE,
		DDRC_MMAP0_VALUE,
		DDRC_MMAP1_VALUE,
	    	DDRC_TIMING4_VALUE,
		DDRC_AUTOSR_EN_VALUE,
		.ddr_remap_array = REMMAP_ARRAY
	}
};
#endif

extern void gpio_init(void);
extern void pll_init(void);
extern void sdram_init(void);
#ifdef CONFIG_CHECK_SOCID
extern int check_socid();
#endif

#ifdef CONFIG_SOFT_BURNER
static void jz_burner_boot(void)
{
	unsigned int val = 'b' << 24 | 'u' << 16 | 'r' << 8 | 'n';
	unsigned int reg;

	reg = cpm_inl(CPM_SLPC);

	if(reg == val) {
		typedef void __noreturn (*image_entry_noargs_t)(void);

		image_entry_noargs_t image_entry =
			(image_entry_noargs_t) (0xbfc0320c);
		cpm_outl(0, CPM_SLPC);
		image_entry();
	}
}
#endif
void board_init_f(ulong dummy)
{
	/* Set global data pointer */
	gd = &gdata;

	/* Setup global info */
#ifndef CONFIG_BURNER
	gd->arch.gi = &ginfo;
#else
	burner_param_info();
#endif
#ifdef CONFIG_SOFT_BURNER
	jz_burner_boot();
#endif
#ifdef CONFIG_CHECK_SOCID
	if(check_socid() < 0)
		return;
#endif
	gpio_init();

	/* Init uart first */
	enable_uart_clk();

#ifdef CONFIG_SPL_SERIAL_SUPPORT
	preloader_console_init();
#endif
	printf("ERROR EPC %x\n", read_c0_errorepc());

	debug("Timer init\n");
	timer_init();

#ifndef CONFIG_FPGA
#ifdef CONFIG_SPL_CORE_VOLTAGE
	debug("Set core voltage:%dmv\n", CONFIG_SPL_CORE_VOLTAGE);
	spl_regulator_set_voltage(REGULATOR_CORE, CONFIG_SPL_CORE_VOLTAGE);
#endif
#ifdef CONFIG_SPL_MEM_VOLTAGE
	debug("Set mem voltage:%dmv\n", CONFIG_SPL_MEM_VOLTAGE);
	spl_regulator_set_voltage(REGULATOR_MEM, CONFIG_SPL_MEM_VOLTAGE);
#endif
#endif
#ifndef CONFIG_BURNER
	debug("CLK stop\n");
	clk_prepare();
#endif
	debug("PLL init\n");
	pll_init();

	debug("CLK init\n");
	clk_init();

#ifdef CONFIG_HW_WATCHDOG
	debug("WATCHDOG init\n");
	hw_watchdog_init();
#endif
	debug("SDRAM init\n");
	sdram_init();

#ifdef CONFIG_DDR_TEST
	ddr_basic_tests();
#endif

#ifndef CONFIG_BURNER
	/* Clear the BSS */
	memset(__bss_start, 0, (char *)&__bss_end - __bss_start);
#ifdef CONFIG_AUDIO_CAL_DIV
	{
		struct audio_div_values {
			unsigned int savem:8;
			unsigned int saven:24;
		};
		struct audio_div_values divvalues [] = AUDIO_DIV_VALUES;
		memcpy((void*)0xf4000000,divvalues,sizeof(divvalues));
	}
#endif
	debug("board_init_r\n");
	board_init_r(NULL, 0);
#else
	debug("run firmware finished\n");
	return ;
#endif
}

extern void flush_cache_all(void);

#ifdef CONFIG_JZ_SECURE_SUPPORT
extern int secure_scboot (void *, void *);
static int secure_load_uboot(struct spl_image_info *spl_image)
{
	int ret = secure_scboot ((void *)spl_image->load_addr,
				 (void *)spl_image->entry_point);
	return ret;
}
#endif

void __noreturn jump_to_image_no_args(struct spl_image_info *spl_image)
{
	typedef void __noreturn (*image_entry_noargs_t)(void);

#ifdef CONFIG_JZ_SECURE_SUPPORT
	flush_cache_all();
	int ret = secure_load_uboot(spl_image);

	if (ret)
	  printf("Error spl secure load uboot.\n");
#endif

	image_entry_noargs_t image_entry =
			(image_entry_noargs_t) spl_image->entry_point;

	flush_cache_all();

	debug("image entry point: 0x%X\n", spl_image->entry_point);
	image_entry();
}

#endif /* CONFIG_SPL_BUILD */

/*
 * U-Boot common functions
 */

void enable_interrupts(void)
{
}

int disable_interrupts(void)
{
	return 0;
}
