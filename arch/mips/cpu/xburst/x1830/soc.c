/*
 * X1830 common routines
 *
 * Copyright (c) 2017 Ingenic Semiconductor Co.,Ltd
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

#define DEBUG
#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <asm/arch/clk.h>
#include <asm/arch/cpm.h>
#include <spl.h>
#include <asm/mipsregs.h>

#ifdef CONFIG_SPL_BUILD
/* Pointer to as well as the global data structure for SPL */
DECLARE_GLOBAL_DATA_PTR;
gd_t gdata __attribute__ ((section(".data")));

#ifndef CONFIG_BURNER
struct global_info ginfo __attribute__ ((section(".data"))) = {
	.extal		= CONFIG_SYS_EXTAL,
	.cpufreq	= CONFIG_SYS_CPU_FREQ,
	.ddrfreq	= CONFIG_SYS_MEM_FREQ,
	.uart_idx	= CONFIG_SYS_UART_INDEX,
	.baud_rate	= CONFIG_BAUDRATE,
};
#endif

extern void pll_init(void);
extern void sdram_init(void);
extern void validate_cache(void);

#ifdef CONFIG_DDR_AUTO_REFRESH_TEST
extern void ddr_test_refresh(unsigned int start_addr, unsigned int end_addr);
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
	gpio_init();
	//gpio_direction_output(GPIO_PB(18), 1);

	/* Init uart first */
	enable_uart_clk();

#ifdef CONFIG_SPL_SERIAL_SUPPORT
	preloader_console_init();
#endif
	printf("ERROR EPC 0x%x\n", (unsigned int)read_c0_errorepc());

	debug("Timer init\n");
	timer_init();

#ifdef CONFIG_SPL_REGULATOR_SUPPORT
	debug("regulator set\n");
	spl_regulator_set();
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
	debug("SDRAM init ok\n");

	{
#undef assert
#define assert(con) if (!(con)) { printf("error %s %d",__func__, __LINE__); while(1); }
		unsigned ddrcdr = cpm_inl(CPM_DDRCDR);
		int ddr = clk_get_rate(DDR);
		if (CONFIG_DDR_SEL_PLL == APLL) {
			assert(((ddrcdr >> 30) & 0x3) == 0x1);
		} else {
			assert(((ddrcdr >> 30) & 0x3) == 0x2);
		}
		debug("ddr %d ==> %d\n", gd->arch.gi->ddrfreq, ddr);
	}

#ifdef CONFIG_DDR_AUTO_REFRESH_TEST
	ddr_test_refresh(0xa0000000, 0xa8000000);
#endif

#ifdef CONFIG_DDR_TEST
	ddr_basic_tests();
#endif

#ifndef CONFIG_BURNER
	/* Clear the BSS */
	memset(__bss_start, 0, (char *)&__bss_end - __bss_start);
	debug("board_init_r\n");
	board_init_r(NULL, 0);
#else
	debug("run firmware finished\n");
	return ;
#endif
}

extern void flush_cache_all(void);

void __noreturn jump_to_image_no_args(struct spl_image_info *spl_image)
{
	typedef void __noreturn (*image_entry_noargs_t)(void);

	image_entry_noargs_t image_entry =
			(image_entry_noargs_t) spl_image->entry_point;

	flush_cache_all();

	debug("image entry point: 0x%x\n", spl_image->entry_point);
#ifdef DEBUG
        {
                int i;
                unsigned int *attr = (unsigned int *)spl_image->entry_point;
                for (i = 0; i < 30; i++)
                        debug("0x%x\n", attr[i]);
        }
#endif
	image_entry();
}

#else /* CONFIG_SPL_BUILD */

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
extern void flush_cache_all(void);

unsigned long do_go_exec(ulong (*entry)(int, char * const []), int argc,
                char * const argv[])
{
        printf("Flush cache all before jump. \n");
        flush_cache_all();

        return entry (argc, argv);
}
#endif /*!CONFIG_SPL_BUILD*/
