/*
 * M200 common routines
 *
 * Copyright (c) 2013 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
 * Based on: arch/mips/cpu/xburst/jz4775/jz4775.c
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

/* #define DEBUG */
#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <asm/mipsregs.h>
#include <asm/arch/clk.h>
#include <asm/arch/cpm.h>
#include <asm/arch-m200/tcu.h>
#include <spl.h>

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

extern void pll_init(void);
extern void sdram_init(void);
extern void validate_cache(void);

#ifdef CONFIG_BURNER
void mmc_clk_nopull(void)
{
	struct jz_gpio_func_def *g;
	int n, i;
	n = gd->arch.gi->nr_gpio_func;

	for (i = 0; i < n; i++) {
		g = &gd->arch.gi->gpio[i];
		if (g->port == GPIO_PORT_A &&
				g->func == (GPIO_FUNC_1|GPIO_PULL) &&
				g->pins && (1 << 18))
			gpio_set_func(GPIO_PORT_A, GPIO_FUNC_1, (1 << 18));

		else if (g->port == GPIO_PORT_E &&
				g->func == (GPIO_FUNC_1|GPIO_PULL) &&
				g->pins && (1 << 28))
			gpio_set_func(GPIO_PORT_E, GPIO_FUNC_1, (1 << 28));

		else if (g->port == GPIO_PORT_D &&
				g->func == (GPIO_FUNC_2|GPIO_PULL) &&
				g->pins && (1 << 24))
			gpio_set_func(GPIO_PORT_D, GPIO_FUNC_2, (1 << 24));

		else if (g->port == GPIO_PORT_E &&
				g->func == (GPIO_FUNC_2|GPIO_PULL) &&
				g->pins && (1 << 28))
			gpio_set_func(GPIO_PORT_E, GPIO_FUNC_2, (1 << 28));

		else if (g->port == GPIO_PORT_B &&
				g->func == (GPIO_FUNC_3|GPIO_PULL) &&
				g->pins && (1 << 28))
			gpio_set_func(GPIO_PORT_E, GPIO_FUNC_3, (1 << 28));

		else if (g->port == GPIO_PORT_E &&
				g->func == (GPIO_FUNC_3|GPIO_PULL) &&
				g->pins && (1 << 28))
			gpio_set_func(GPIO_PORT_E, GPIO_FUNC_3, (1 << 28));
	}
}
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
			(image_entry_noargs_t) (0xbfc03cf8);
		cpm_outl(0, CPM_SLPC);
		image_entry();
	}
}
#endif

void board_init_f(ulong dummy)
{
	/***wdt stop**/
	writel(readl(TCU_TSSR) | TCU_TSSR_WDTSS, TCU_TSSR);

	/* Set global data pointer */
	gd = &gdata;

	/* Setup global info */
#ifndef CONFIG_CMD_BURN
	gd->arch.gi = &ginfo;
#else
	burner_param_info();
	/* gd->arch.gi = (struct global_info *)CONFIG_SPL_GINFO_BASE; */
#endif

#ifdef CONFIG_SOFT_BURNER
	jz_burner_boot();
#endif

	gpio_init();

#ifdef CONFIG_BURNER
	mmc_clk_nopull();
#endif

#ifndef CONFIG_FPGA
	/* Init uart first */
	enable_uart_clk();
#endif

#ifdef CONFIG_SPL_SERIAL_SUPPORT
	preloader_console_init();
#endif
	printf("ERROR EPC %x\n", read_c0_errorepc());

#ifndef CONFIG_FPGA
	debug("Timer init\n");
	timer_init();

#ifdef CONFIG_SPL_REGULATOR_SUPPORT
	debug("regulator set\n");
	spl_regulator_set();
#endif

	debug("CLK stop\n");
	clk_prepare();

	debug("PLL init\n");
	pll_init();

	debug("CLK init\n");
	clk_init();
#endif

	debug("SDRAM init\n");
	sdram_init();
	debug("SDRAM init ok\n");

#ifdef CONFIG_DDR_TEST
	ddr_basic_tests();
#endif

#ifndef CONFIG_BURNER
	/* Clear the BSS */
	memset(__bss_start, 0, (char *)&__bss_end - __bss_start);
	debug("board_init_r\n");
	board_init_r(NULL, 0);
#endif
}

extern void flush_cache_all(void);

void __noreturn jump_to_image_no_args(struct spl_image_info *spl_image)
{
	typedef void __noreturn (*image_entry_noargs_t)(void);

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
