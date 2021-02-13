// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright (C) 2004-2008, 2012 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

#include <asm/io.h>
#include <config.h>
#include <common.h>
#include <spi.h>
#include <netdev.h>

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	puts("Board: ");
	puts("ATX040\n");
	return 0;
};

int dram_init(void)
{
	gd->ram_size = 32 * 1024 * 1024;

	return 0;
};

#ifdef CONFIG_MMC
int board_mmc_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_MMC_SPI
	struct mmc *mmc;
	mmc = mmc_spi_init(1, 0, 25000000, 0);
	if(mmc)
		mmc_init(mmc);
#endif
	return rc;
}
#endif

#ifdef CONFIG_CMD_NET
int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_ENC28J60
	rc = enc28j60_initialize(2, 0, 25000000, 0);
#endif
	return rc;
}
#endif
