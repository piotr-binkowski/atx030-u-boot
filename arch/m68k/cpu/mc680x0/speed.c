// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2003
 * Josef Baumgartner <josef.baumgartner@telex.de>
 *
 * Copyright (C) 2004-2007, 2012 Freescale Semiconductor, Inc.
 * Hayden Fraser (Hayden.Fraser@freescale.com)
 */

#include <common.h>
#include <asm/processor.h>

DECLARE_GLOBAL_DATA_PTR;

/* get_clocks() fills in gd->cpu_clk and gd->bus_clk */
int get_clocks (void)
{
	gd->cpu_clk = CONFIG_SYS_CLK;
	gd->bus_clk = gd->cpu_clk;
	return 0;
}
