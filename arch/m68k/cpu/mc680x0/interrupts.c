// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright (C) 2004-2007, 2012 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

#include <common.h>
#include <asm/processor.h>
#include <asm/io.h>

#if defined(CONFIG_MC680x0)
int interrupt_init(void)
{
	enable_interrupts();

	return 0;
}

#endif /* CONFIG_MC680x0 */
