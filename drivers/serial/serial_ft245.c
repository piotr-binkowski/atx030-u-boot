// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2004-2007 Freescale Semiconductor, Inc.
 * TsiChung Liew, Tsi-Chung.Liew@freescale.com.
 *
 * Modified to add device model (DM) support
 * (C) Copyright 2015  Angelo Dureghello <angelo@sysam.it>
 */

/*
 * Minimal serial functions needed to use one of the uart ports
 * as serial console interface.
 */

#include <common.h>
#include <serial.h>
#include <linux/compiler.h>

DECLARE_GLOBAL_DATA_PTR;

#define FT245_TXE	0
#define FT245_RXF	1
#define FT245_DATA	2

static int ft245_serial_init(void)
{
	return 0;
}

static void ft245_serial_putc(const char c)
{
	if (c == '\n')
		serial_putc('\r');

	while(readb(CONFIG_SYS_FT245_BASE + FT245_TXE));
	writeb(c, CONFIG_SYS_FT245_BASE + FT245_DATA);
}

#ifdef CONFIG_DEBUG_UART_FT245

#include <debug_uart.h>

static inline void _debug_uart_init(void)
{

}

static inline void _debug_uart_putc(int c)
{
	while(readb(CONFIG_SYS_FT245_BASE + FT245_TXE));
	writeb(c, CONFIG_SYS_FT245_BASE + FT245_DATA);
}

DEBUG_UART_FUNCS

#endif

static int ft245_serial_getc(void)
{
	while(readb(CONFIG_SYS_FT245_BASE + FT245_RXF));
	return readb(CONFIG_SYS_FT245_BASE + FT245_DATA);
}

static void ft245_serial_setbrg(void)
{
}

static int ft245_serial_tstc(void)
{
	return !readb(CONFIG_SYS_FT245_BASE + FT245_RXF);
}

static struct serial_device ft245_serial_drv = {
	.name	= "ft245_serial",
	.start	= ft245_serial_init,
	.stop	= NULL,
	.setbrg	= ft245_serial_setbrg,
	.putc	= ft245_serial_putc,
	.puts	= default_serial_puts,
	.getc	= ft245_serial_getc,
	.tstc	= ft245_serial_tstc,
};

void ft245_serial_initialize(void)
{
	serial_register(&ft245_serial_drv);
}

__weak struct serial_device *default_serial_console(void)
{
	return &ft245_serial_drv;
}
