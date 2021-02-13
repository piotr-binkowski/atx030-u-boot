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

#define WB_UART_DATA	0x0
#define WB_UART_STATUS	0x4

#define RX_EMPTY	0x1
#define TX_FULL		0x8

static int wb_uart_serial_init(void)
{
	return 0;
}

static void wb_uart_serial_putc(const char c)
{
	if (c == '\n')
		serial_putc('\r');

	while(readb(CONFIG_SYS_WB_UART_BASE + WB_UART_STATUS) & TX_FULL);
	writeb(c, CONFIG_SYS_WB_UART_BASE + WB_UART_DATA);
}

#ifdef CONFIG_DEBUG_UART_WB_UART

#include <debug_uart.h>

static inline void _debug_uart_init(void)
{

}

static inline void _debug_uart_putc(int c)
{
	while(readb(CONFIG_SYS_WB_UART_BASE + WB_UART_STATUS) & TX_FULL);
	writeb(c, CONFIG_SYS_WB_UART_BASE + WB_UART_DATA);
}

DEBUG_UART_FUNCS

#endif

static int wb_uart_serial_getc(void)
{
	while(readb(CONFIG_SYS_WB_UART_BASE + WB_UART_STATUS) & RX_EMPTY);
	return readb(CONFIG_SYS_WB_UART_BASE + WB_UART_DATA);
}

static void wb_uart_serial_setbrg(void)
{

}

static int wb_uart_serial_tstc(void)
{
	return !(readb(CONFIG_SYS_WB_UART_BASE + WB_UART_STATUS) & RX_EMPTY);
}

static struct serial_device wb_uart_serial_drv = {
	.name	= "wb_uart_serial",
	.start	= wb_uart_serial_init,
	.stop	= NULL,
	.setbrg	= wb_uart_serial_setbrg,
	.putc	= wb_uart_serial_putc,
	.puts	= default_serial_puts,
	.getc	= wb_uart_serial_getc,
	.tstc	= wb_uart_serial_tstc,
};

void wb_uart_serial_initialize(void)
{
	serial_register(&wb_uart_serial_drv);
}

__weak struct serial_device *default_serial_console(void)
{
	return &wb_uart_serial_drv;
}
