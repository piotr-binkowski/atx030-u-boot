// SPDX-License-Identifier: GPL-2.0+
/*
 */

#include <common.h>
#include <command.h>
#include "cpu.h"

DECLARE_GLOBAL_DATA_PTR;

#ifdef	CONFIG_MC680x0
int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	puts("resetting...\n");
	disable_interrupts();
	for(;;)
		__asm__ __volatile__("reset");

	return 0;
};

#if defined(CONFIG_DISPLAY_CPUINFO)
int print_cpuinfo(void)
{
	char buf[32];

	printf("CPU:   Motorola MC68030 @ %s MHz\n", strmhz(buf, gd->cpu_clk));
	return 0;
};
#endif /* CONFIG_DISPLAY_CPUINFO */

#endif /* CONFIG_MC680x0 */

