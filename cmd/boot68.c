#include <common.h>
#include <command.h>
#include <image.h>

#define BI_LAST		0
#define BI_MACHTYPE	1
#define BI_CPUTYPE	2
#define BI_FPUTYPE	3
#define BI_MMUTYPE	4
#define BI_MEMCHUNK	5
#define BI_RAMDISK	6
#define BI_COMMAND	7
#define MACHTYPE	15
#define CPUTYPE		2
#define MMUTYPE		2
#define FPUTYPE		2

DECLARE_GLOBAL_DATA_PTR;

static u32 get_bi_addr(u32 addr)
{
	return addr + (4096 - (addr % 4096));
}

static u32 add_record(u32 bi_addr, u16 tag, u32 data)
{
	*(u16*)(bi_addr) = tag;
	*(u16*)(bi_addr + 2) = 8;
	*(u32*)(bi_addr + 4) = data;
	return 8;
}

static u32 add_meminfo(u32 bi_addr, u16 tag, u32 addr, u32 size)
{
	*(u16*)(bi_addr) = tag;
	*(u16*)(bi_addr + 2) = 12;
	*(u32*)(bi_addr + 4) = addr;
	*(u32*)(bi_addr + 8) = size;
	return 12;
}

static u32 add_bootargs(u32 bi_addr)
{
	u32 size = 4;
	char *bootargs = env_get("bootargs");

	size += strlen(bootargs);

	*(u16*)(bi_addr) = BI_COMMAND;
	*(u16*)(bi_addr + 2) = size;

	memcpy((u8*)(bi_addr + 4), bootargs, strlen(bootargs));	

	return size;
}

int do_boot68(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u32 mem_addr, mem_size;
	u32 bi_addr;
	int ret;
	int states = BOOTM_STATE_START | BOOTM_STATE_FINDOS | BOOTM_STATE_FINDOTHER | BOOTM_STATE_LOADOS;

	argc--; argv++;

	ret = do_bootm_states(cmdtp, flag, argc, argv, states, &images, 1);

	if(ret)
		return ret;

	mem_addr = gd->bd->bi_memstart;
	mem_size = gd->bd->bi_memsize;
	bi_addr = get_bi_addr(images.os.load_end);

	bi_addr += add_record(bi_addr, BI_MACHTYPE, MACHTYPE);
	bi_addr += add_record(bi_addr, BI_CPUTYPE, CPUTYPE);
	bi_addr += add_record(bi_addr, BI_FPUTYPE, FPUTYPE);
	bi_addr += add_record(bi_addr, BI_MMUTYPE, MMUTYPE);
	bi_addr += add_meminfo(bi_addr, BI_MEMCHUNK, mem_addr, mem_size);
	bi_addr += add_meminfo(bi_addr, BI_RAMDISK, images.rd_start, images.rd_end - images.rd_start);

	bi_addr += add_bootargs(bi_addr);
	bi_addr += add_record(bi_addr, BI_LAST, 0);

	bootm_disable_interrupts();
	icache_disable();
	dcache_disable();

	goto *(void*)images.ep;

	return 0;
}

#ifdef CONFIG_SYS_LONGHELP
static char boot68_help_text[] =
	"[kernel_addr initrd_addr]\n"
	"    - boot Linux m68k binary stored in memory";
#endif

U_BOOT_CMD(
	boot68, CONFIG_SYS_MAXARGS, 1, do_boot68,
	"boot Linux m68k binary from memory", boot68_help_text
);
