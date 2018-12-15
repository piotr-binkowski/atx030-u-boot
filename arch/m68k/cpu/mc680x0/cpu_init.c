// SPDX-License-Identifier: GPL-2.0+
/*
 */

#include <common.h>
#include <asm/io.h>

#if defined(CONFIG_MC680x0)
void cpu_init_f(void)
{
	icache_enable();
	dcache_enable();
}

/* initialize higher level parts of CPU like timers */
int cpu_init_r(void)
{
	return 0;
}

void uart_port_conf(int port)
{

}
#endif /* CONFIG_SBC030 */

