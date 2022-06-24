/*
 * M200 GPIO definitions
 *
 * Copyright (c) 2013 Ingenic Semiconductor Co.,Ltd
 * Author: Sonil <ztyan@ingenic.cn>
 * Based on: newxboot/modules/gpio/jz4775_gpio.c|jz4780_gpio.c
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

static struct jz_gpio_func_def uart_gpio_func[] = {
	[0] = { .port = GPIO_PORT_C, .func = GPIO_FUNC_0, .pins = 0xf<<10},
#ifdef CONFIG_SYS_UART1_PA
	[1] = { .port = GPIO_PORT_A, .func = GPIO_FUNC_2, .pins = 3<<4},
#else
	[1] = { .port = GPIO_PORT_D, .func = GPIO_FUNC_1, .pins = 0xf<<2},
#endif
#ifdef CONFIG_SYS_UART2_PD
	[2] = { .port = GPIO_PORT_D, .func = GPIO_FUNC_0, .pins = 3<<4},
#elif defined(CONFIG_SYS_UART2_PA)
	[2] = { .port = GPIO_PORT_A, .func = GPIO_FUNC_2, .pins = 3<<2},
#else
	[2] = { .port = GPIO_PORT_C, .func = GPIO_FUNC_1 | GPIO_PULL, .pins = 1<<31},
#endif
};

static struct jz_gpio_func_def gpio_func[] = {
#if defined(CONFIG_JZ_MMC_MSC0_PA_4BIT)
	{ .port = GPIO_PORT_A, .func = GPIO_FUNC_1, .pins = 0x03f00000},
#endif
#if defined(CONFIG_JZ_SSI0_PA)
	{ .port = GPIO_PORT_A, .func = GPIO_FUNC_2, .pins = 0x3c000000},
#endif
#if defined(CONFIG_JZ_MMC_MSC0_PA_8BIT)
	{ .port = GPIO_PORT_A, .func = GPIO_FUNC_1, .pins = 0x03ff0000},
#endif
#if defined(CONFIG_JZ_MMC_MSC1_PC)
	{ .port = GPIO_PORT_C, .func = GPIO_FUNC_0, .pins = 0x0000003f},
#endif

#if defined(CONFIG_LCD_GPIO_FUNC0_16BIT) || defined(CONFIG_LCD_GPIO_FUNC0_24BIT)
	{ .port = GPIO_PORT_C, .func = GPIO_FUNC_0, .pins = 0x0fffffff, },
#endif
#ifdef  CONFIG_LCD_GPIO_FUNC2_SLCD
	{.port = GPIO_PORT_C, .func = GPIO_FUNC_2, .pins = 0x0e0ff3fc, }
#endif
#if defined(CONFIG_LCD_GPIO_FUNC1_SLCD)
	{.port = GPIO_PORT_B, .func = GPIO_FUNC_1, .pins = 0x001a0000, },
	{.port = GPIO_PORT_A, .func = GPIO_FUNC_1, .pins = 0x000000ff, },
#endif

#ifdef CONFIG_JZ_PWM_GPIO_E0
	{ .port = GPIO_PORT_E, .func = GPIO_FUNC_0, .pins = 1 << 0, },
#endif
#ifdef CONFIG_JZ_PWM_GPIO_E1
	{ .port = GPIO_PORT_E, .func = GPIO_FUNC_0, .pins = 1 << 1, },
#endif
#ifdef CONFIG_JZ_PWM_GPIO_E2
	{ .port = GPIO_PORT_E, .func = GPIO_FUNC_0, .pins = 1 << 2, },
#endif
#ifdef CONFIG_JZ_PWM_GPIO_E3
	{ .port = GPIO_PORT_E, .func = GPIO_FUNC_0, .pins = 1 << 3, },
#endif
#ifdef CONFIG_JZ_SFC_PA_6BIT
	{ .port = GPIO_PORT_A, .func = GPIO_FUNC_1 | GPIO_PULL, .pins = 0x3f <<26, },
#endif
#ifdef CONFIG_JZ_PMU_SLP_OUTPUT1
	{ .port = GPIO_PORT_C, .func = GPIO_OUTPUT1, .pins = 0x1 <<22, },
#endif
#ifdef CONFIG_NET_GMAC
	{ .port = GPIO_PORT_B, .func = GPIO_FUNC_1, .pins = 0x3ff << 6, },
#endif
};
