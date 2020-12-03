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
	puts("ATX030\n");
	return 0;
};

int dram_init(void)
{
	gd->ram_size = 128 * 1024 * 1024;

	return 0;
};

#ifdef CONFIG_CMD_I2C

#define ATX030_I2C_BASE	0xE0020000
#define I2C_DATA_REG	0
#define I2C_OE_REG	1
#define SDA_BIT		0x01
#define SCL_BIT		0x02

char atx030_i2c_sda_get(void)
{
	volatile uint8_t *reg = (uint8_t*)(ATX030_I2C_BASE + I2C_DATA_REG);
	return *reg & SDA_BIT;
}

void atx030_i2c_sda_set(char val)
{
	volatile uint8_t *reg = (uint8_t*)(ATX030_I2C_BASE + I2C_OE_REG);
	if(val)
		*reg &= ~SDA_BIT;
	else
		*reg |= SDA_BIT;
}

void atx030_i2c_scl_set(char val)
{
	volatile uint8_t *reg = (uint8_t*)(ATX030_I2C_BASE + I2C_OE_REG);
	if(val)
		*reg &= ~SCL_BIT;
	else
		*reg |= SCL_BIT;
}

#endif /* CONFIG_CMD_I2C */

#ifdef CONFIG_MMC
int board_mmc_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_MMC_SPI
	struct mmc *mmc;
	mmc = mmc_spi_init(0, 0, 25000000, 0);
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
	rc = enc28j60_initialize(1, 0, 25000000, 0);
#endif
	return rc;
}
#endif

#ifdef CONFIG_FLASH_CFI_LEGACY
ulong board_flash_get_legacy(ulong base, int banknum, flash_info_t *info)
{
	if (banknum == 0) {	/* non-CFI boot flash */
		info->portwidth = FLASH_CFI_8BIT;
		info->chipwidth = FLASH_CFI_BY8;
		info->interface = FLASH_CFI_X8;
		return 1;
	} else {
		return 0;
	}
}
#endif /* CONFIG_FLASH_CFI_LEGACY */
