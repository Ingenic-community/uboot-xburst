/*
  * WDT DRIVER
  *
  * Copyright (c) 2015 Ingenic Semiconductor Co.,Ltd
  * Author: gao wei <wei.gao@ingenic.com>
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
#include <common.h>
#include <watchdog.h>
#include <asm/io.h>
#include <asm/arch/base.h>
#include <asm/arch/wdt.h>
#include <asm/arch/tcu.h>
#include <asm/arch/cpm.h>
#include <asm/arch/rtc.h>

#define wdt_write(value, reg) writel(value, WDT_BASE + reg)
#define wdt_read(reg) readl(WDT_BASE + reg)

#define tcu_writel(value, reg) writel(value, reg)
#define tcu_readl(reg) readl(reg)

#define rtc_writel(value, reg) writel(value, RTC_BASE + reg)
#define rtc_readl(reg) readl(RTC_BASE + reg)

struct {
        int div;
        unsigned int value;
} wdt_div_table[] = {
        {1   , TCSR_PRESCALE_1   },
        {4   , TCSR_PRESCALE_4   },
        {16  , TCSR_PRESCALE_16  },
        {64  , TCSR_PRESCALE_64  },
        {256 , TCSR_PRESCALE_256 },
        {1024, TCSR_PRESCALE_1024},
};

static int wdt_settimeout(unsigned int timeout) /* timeout: ms */
{
        unsigned int i;
        unsigned int reg;
        unsigned int base_freq;
        int wdt_div_num = -1;
        unsigned int tdr, clk_in;
		int div_num = sizeof(wdt_div_table) / sizeof(wdt_div_table[0]);

#if defined(CONFIG_WDT_FREQ_BY_RTC)
        clk_in = TCSR_RTC_EN;

#ifdef CONFIG_RTC_SELEXC_BY_RTC
		{
			unsigned int val;
			base_freq = RTC_FREQ;
			val = cpm_readl(CPM_OPCR);
			val |= (1 << 2);
			cpm_writel(val, CPM_OPCR);

			val = rtc_readl(RTC_RTCCR);
			val &= ~(1 << 1);
			rtc_writel(val, RTC_RTCCR);
		}
#else
		{
			unsigned int val;
			base_freq = CONFIG_SYS_EXTAL/512;
			val = cpm_readl(CPM_OPCR);
			val &= ~(1 << 2);
			cpm_writel(val, CPM_OPCR);

			val = rtc_readl(RTC_RTCCR);
			val |= 1 << 1;
			rtc_writel(val, RTC_RTCCR);
		}
#endif

#elif defined(CONFIG_WDT_FREQ_BY_EXCLK)
        base_freq = CONFIG_SYS_EXTAL;
        clk_in = TCSR_EXT_EN;
#elif defined(CONFIG_WDT_FREQ_BY_PCLK)
        goto err;
#else
        goto err;
#endif
		for (i = 0; i < div_num; i++) {
			tdr = base_freq / wdt_div_table[i].div * (timeout / 1000);
			if(tdr < 65535) {
				wdt_div_num = i;
				break;
			}
        }

		if(i == div_num) {
			tdr = 65535;
			wdt_div_num = i - 1;
		}

		tcu_writel(1 << 16, TCU_TSCR);
        wdt_write(tdr, WDT_TDR);                                                // set contrast count
        wdt_write(clk_in | wdt_div_table[wdt_div_num].value, WDT_TCSR);         // set clk config
        wdt_write(0x0, WDT_TCNT);                                               // clean counter

#ifdef DEBUG
        debug("WDT CLK IN is  %dHZ\n" , base_freq);
        debug("REAL FREQ is   %dHZ\n" , base_freq / wdt_div_table[i].div);
        debug("The timeout is %dms\n" , tdr * 1000 * wdt_div_table[i].div / base_freq);
        debug("WDT_TDR:       0x%x\n" , wdt_read(WDT_TDR));
        debug("WDT_TCSR:      0x%x\n" , wdt_read(WDT_TCSR));
#endif
        wdt_write(0, WDT_TCER);
        wdt_write(1, WDT_TCER);
        return 0;
err:
        printf("Unable to provide the timeout, please check it!");
        return -1;
}

void hw_watchdog_disable(void)
{
        wdt_write(wdt_read(WDT_TCER) & ~TCER_TCEN, WDT_TCER);
		tcu_writel(1 << 16, TCU_TSSR);
}

void hw_watchdog_reset(void)
{
        debug("watchdog reset\n");
        wdt_write(0x0, WDT_TCNT);

}
void hw_watchdog_init(void)
{
        wdt_settimeout(CONFIG_WDT_TIMEOUT_BY_MS);
}
