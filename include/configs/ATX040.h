/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuation settings for the Freescale MCF5208EVBe.
 *
 * Copyright (C) 2004-2008 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

#ifndef _ATX040_H
#define _ATX040_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_SYS_LOAD_ADDR	0x00100000

#define CONFIG_SYS_CLK		25000000 /* CPU Core Clock */
/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */
/* Definitions for initial stack pointer and data area (in DPRAM) */
#define CONFIG_SYS_INIT_RAM_ADDR	0x00100000
#define CONFIG_SYS_INIT_RAM_SIZE	0x4000	/* Size of used area in internal SRAM */
#define CONFIG_SYS_GBL_DATA_OFFSET	((CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE) - 0x10)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_BASE		(CONFIG_SYS_SDRAM_BASE + 31 * 1024 * 1024 + 0x400)
#define CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor */

#define CONFIG_SYS_BOOTPARAMS_LEN	64*1024
#define CONFIG_SYS_MALLOC_LEN		(256 << 10)	/* Reserve 256 kB for malloc() */

#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_NUM_IRQS		64

#define CONFIG_DEBUG_UART_WB_UART

#define CONFIG_WB_UART
#define CONFIG_SYS_WB_UART_BASE		0xC0000000

#define CONFIG_WB_TIM
#define CONFIG_SYS_WB_TIM_BASE		0xC8000000

#ifdef CONFIG_MMC
# define CONFIG_MMC_SPI
#endif

#define CONFIG_ENV_SIZE			0x1000

#define CONFIG_SYS_MEMTEST_START	0x01000000
#define CONFIG_SYS_MEMTEST_END		0x01800000

#define CONFIG_EXTRA_ENV_SETTINGS \
	"kernel_addr=800000\0" \
	"initrd_addr=1000000\0" \
	"uboot_off=80000\0" \
	"uboot_size=40000\0" \
	"sd_args=root=/dev/mmcblk0p2 rw rootwait console=ttyS0\0" \
	"sdboot=ext4load mmc 0:1 $kernel_addr linux.u && setenv bootargs \"$sd_args\" && boot68 $kernel_addr\0" \
	"net_args=root=/dev/ram0 console=ttyS0\0" \
	"netboot=dhcp $kernel_addr /linux.u && dhcp $initrd_addr /rootfs.u && setenv bootargs \"$net_args\" && boot68 $kernel_addr $initrd_addr\0" \
	"update=mw.b $kernel_addr ff $uboot_size && " \
	"loadb $kernel_addr && " \
	"sf probe 0 && " \
	"mtd erase nor0 $uboot_off $uboot_size && " \
	"mtd write nor0 $kernel_addr $uboot_off $uboot_size\0"

/* Cache Configuration */
#define CONFIG_SYS_CACHELINE_SIZE	16

#define ICACHE_STATUS			(CONFIG_SYS_INIT_RAM_ADDR + \
					 CONFIG_SYS_INIT_RAM_SIZE - 8)
#define DCACHE_STATUS			(CONFIG_SYS_INIT_RAM_ADDR + \
					 CONFIG_SYS_INIT_RAM_SIZE - 4)

#endif /* _ATX040_H */
