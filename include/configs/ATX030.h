/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuation settings for the Freescale MCF5208EVBe.
 *
 * Copyright (C) 2004-2008 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

#ifndef _ATX030_H
#define _ATX030_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_SYS_LOAD_ADDR	0x01000000

#define CONFIG_SYS_CLK		40000000 /* CPU Core Clock */
/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */
/* Definitions for initial stack pointer and data area (in DPRAM) */
#define CONFIG_SYS_INIT_RAM_ADDR	0x01000000
#define CONFIG_SYS_INIT_RAM_SIZE	0x4000	/* Size of used area in internal SRAM */
#define CONFIG_SYS_GBL_DATA_OFFSET	((CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE) - 0x10)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_BASE		(CONFIG_SYS_SDRAM_BASE + 127 * 1024 * 1024 + 0x400)
#define CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor */

#define CONFIG_SYS_BOOTPARAMS_LEN	64*1024
#define CONFIG_SYS_MALLOC_LEN		(256 << 10)	/* Reserve 256 kB for malloc() */

#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_NUM_IRQS		64

#define CONFIG_DEBUG_UART_FT245

#define CONFIG_FT245
#define CONFIG_SYS_FT245_BASE		0xE0000000
#define CONFIG_SYS_FT245_IRQ		26

#define CONFIG_ATX030_TIM
#define CONFIG_SYS_ATX030_TIM_IRQ	25

#define CONFIG_SYS_FLASH_CFI
#define CONFIG_FLASH_CFI_LEGACY
#define CONFIG_SYS_FLASH_LEGACY_512Kx8
#define CONFIG_SYS_FLASH_BASE		0xF0000000
#define CONFIG_SYS_FLASH_SIZE		0x80000
#define CONFIG_SYS_MAX_FLASH_BANKS	1
#define CONFIG_SYS_MAX_FLASH_SECT	128
#define CONFIG_SYS_FLASH_BANKS_LIST	{ CONFIG_SYS_FLASH_BASE, }

#ifdef CONFIG_MMC
# define CONFIG_MMC_SPI
#endif

#define CONFIG_ENV_SIZE			0x1000

#ifdef CONFIG_ENV_IS_IN_FLASH
# define CONFIG_ENV_SECT_SIZE		0x1000
# define CONFIG_ENV_OFFSET		(CONFIG_SYS_FLASH_SIZE - CONFIG_ENV_SECT_SIZE)
# define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + CONFIG_ENV_OFFSET)
#endif

#ifdef CONFIG_CMD_I2C

# define CONFIG_SYS_I2C
# define CONFIG_SYS_I2C_SOFT
# define CONFIG_SYS_I2C_SPEED           100000
# define CONFIG_SYS_I2C_SLAVE           0x7F
# define CONFIG_SOFT_I2C_READ_REPEATED_START

# ifndef __ASSEMBLY__
char atx030_i2c_sda_get(void);
void atx030_i2c_sda_set(char);
void atx030_i2c_scl_set(char);
# endif

# define I2C_INIT do { } while (0)
# define I2C_READ atx030_i2c_sda_get()
# define I2C_SDA(bit) atx030_i2c_sda_set(bit)
# define I2C_SCL(bit) atx030_i2c_scl_set(bit)
# define I2C_DELAY do { } while (0)
# define I2C_ACTIVE do { } while (0)
# define I2C_TRISTATE do { } while (0)
#endif

#define CONFIG_SYS_MEMTEST_START	0x01000000
#define CONFIG_SYS_MEMTEST_END		0x02000000

#define CONFIG_EXTRA_ENV_SETTINGS \
	"kernel_addr=1000000\0" \
	"initrd_addr=2000000\0" \
	"uboot_size=40000\0" \
	"update=mw.b $kernel_addr ff $uboot_size && "\
	"loadb $kernel_addr && " \
	"mtd erase nor0 0 $uboot_size && " \
	"mtd write nor0 $kernel_addr 0 $uboot_size\0" \
	"sd_args=root=/dev/mmcblk0p2 rw rootwait console=ttyS0\0" \
	"sdboot=ext4load mmc 0:1 $kernel_addr linux.u && setenv bootargs \"$sd_args\" && boot68 $kernel_addr\0" \
	"net_args=root=/dev/ram0 console=ttyS0\0" \
	"netboot=dhcp $kernel_addr /linux.u && dhcp $initrd_addr /rootfs.u && setenv bootargs \"$net_args\" && boot68 $kernel_addr $initrd_addr\0" \
	"bootcmd=run sdboot\0"

/* Cache Configuration */
#define CONFIG_SYS_CACHELINE_SIZE	16

#define ICACHE_STATUS			(CONFIG_SYS_INIT_RAM_ADDR + \
					 CONFIG_SYS_INIT_RAM_SIZE - 8)
#define DCACHE_STATUS			(CONFIG_SYS_INIT_RAM_ADDR + \
					 CONFIG_SYS_INIT_RAM_SIZE - 4)

#endif /* _ATX030_H */
