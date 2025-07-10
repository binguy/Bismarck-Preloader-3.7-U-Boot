#ifndef _CROSS_ENV_H_
#define _CROSS_ENV_H_

#include <soc.h>

#if defined(__LUNA_KERNEL__)
#include <linux/kernel.h>
#include <linux/module.h>
#include <luna_cfg.h>
#include <asm/regdef.h>
#include <asm/cacheflush.h>
#define puts(...)          printk(__VA_ARGS__)
#define printf(...)        printk(__VA_ARGS__)
#define inline_memcpy(...) memcpy(__VA_ARGS__)

#ifdef noinline
#undef noinline
#endif
#define IDCACHE_FLUSH() do { \
		dma_cache_inv(NORSF_CFLASH_BASE, 64*1024*1024); \
	} while (0)
#define DCACHE_LINE_SZ_B CACHELINE_SIZE

#elif defined(CONFIG_UNDER_UBOOT)
#include <cpu.h>
#include <common.h>
#include <asm/mipsregs.h>
#include <asm/otto_cg_dev_freq.h>
#include <malloc.h>

void invalidate_icache_all(void);
void writeback_invalidate_dcache_all(void);

#define inline_memcpy(...) memcpy(__VA_ARGS__)
#define IDCACHE_FLUSH() do { \
		invalidate_icache_all(); \
		writeback_invalidate_dcache_all(); \
	} while (0)
#ifndef GET_CPU_MHZ
	#define GET_CPU_MHZ()    cg_query_freq(CG_DEV_OCP)
#endif
#define DCACHE_LINE_SZ_B CONFIG_SYS_CACHELINE_SIZE

#define _TO_STR(_s) __STR(_s)
#define TO_STR(s)   STR(s)

#else
#include <cpu/cpu.h>
#include <cg/cg_dev_freq.h>
#define IDCACHE_FLUSH() do {	  \
		_bios.dcache_writeback_invalidate_all(); \
		_bios.icache_invalidate_all(); \
	} while (0)
#ifndef GET_CPU_MHZ
	#define GET_CPU_MHZ()    cg_query_freq(CG_DEV_OCP)
#endif
#define DCACHE_LINE_SZ_B DCACHE_LINE_SIZE
#endif

#endif
