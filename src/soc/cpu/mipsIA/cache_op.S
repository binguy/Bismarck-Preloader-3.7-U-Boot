#define __ASM__
#include <soc.h>

#ifndef GFUNC
#define GFUNC(symbol) \
  .text; \
	.globl symbol; \
	.align 4; \
	.ent symbol; \
symbol:
#endif

#ifndef GSFUNC
#define GSFUNC(symbol) \
	.text; \
	.globl symbol; \
	.align 4; \
	.ent symbol; \
symbol:
#endif

#ifndef END
#define END(symbol) \
  .end symbol
#endif

#include <cpu/cpu.s>

.set push
.set noreorder

GFUNC(bypass_L2)
	mfc0	a0, CP0_CONFIG2
	li	a1, 1
	ins	a0, a1, 12, 1
	mtc0	a0, CP0_CONFIG2
	jr	ra
	nop
END(bypass_L2)

GFUNC(init_l23)
	mtc0  zero, CP0_TAGLO2
	mtc0  zero, CP0_DATALO2
	mtc0  zero, CP0_DATAHI2
	lui   t0, 0x8000
	li    t1, (0x80000000 + L2CACHE_SIZE - CACHELINE_SIZE)
	_reset_next_l2tag:
	cacheop(Index_Store_Tag_SD, t0)
	bne   t0, t1, _reset_next_l2tag
	addi  t0, CACHELINE_SIZE
	j ra
	nop
END(init_l23)

GFUNC(enable_l23)
	li    t1, GCR_ADDR
	lw    t0, 0x0008(t1) // Read GCR_BASE
	ins   t0, zero, 0, 8 // Disable CM2 CCA override
	sw    t0, 0x0008(t1) // Write GCR_BASE
	j ra
	nop
END(enable_l23)

GFUNC(disable_l23)
	DISABLE_L23
	j ra
	nop
END(disable_l23)

.set pop

GSFUNC(invalidate_icache_all)
	mtc0	zero, CP0_ITAGLO
	li	t0, CKSEG0
	li	t2, ICACHE_SIZE
	addu	t1, t0, t2
1:
	cacheop(Index_Invalidate_I, t0)
	addiu	t0, t0, CACHELINE_SIZE
	bne	t0, t1, 1b
	jr	ra
END(invalidate_icache_all)

GSFUNC(writeback_invalidate_dcache_all)
	mtc0	zero, CP0_DTAGLO
	li	t0, CKSEG0
	li	t2, DCACHE_SIZE
	addu	t1, t0, t2
1:
	cacheop(Index_Writeback_Inv_D, t0)
	addiu	t0, t0, CACHELINE_SIZE
	bne	t0, t1, 1b
	jr	ra
END(writeback_invalidate_dcache_all)

GSFUNC(writeback_invalidate_dcache_range)
	move	t0, a0
	li	t1, CACHELINE_SIZE
	move	t2, a1
wi_dc_r_loop:
	cacheop(Hit_Writeback_Inv_D, t0)
	addu	t0, t1
	bge	t2, t0, wi_dc_r_loop
	j	ra
END(writeback_invalidate_dcache_range)

GSFUNC(invalidate_icache_range)
	move	t0, a0
	li	t1, CACHELINE_SIZE
	move	t2, a1
i_ic_r_loop:
	cacheop(Index_Invalidate_I, t0)
	addu	t0, t1
	bge	t2, t0, i_ic_r_loop
	j	ra
END(invalidate_icache_range)

GSFUNC(writeback_invalidate_l2cache_all)
	mtc0    zero, CP0_TAGLO2
	li      t0, CKSEG0
	li      t2, L2CACHE_SIZE
	addu    t1, t0, t2
1:
	cacheop(Index_Writeback_Inv_SD, t0)
	addiu   t0, t0, CACHELINE_SIZE
	bne     t0, t1, 1b
	jr      ra
END(writeback_invalidate_l2cache_all)

// adapt from the generic one in mips4kec
#ifndef SECTION_CACHE_FUNC
#	define SECTION_CACHE_FUNC .text
#endif
#define Hit_Writeback_D 0x19
#define Fetch_Lock_I 0x1c
#define Fetch_Lock_D 0x1d
#define GFUNC_new(fname, ...) \
	SECTION_CACHE_FUNC;\
	.global fname;\
	.ent fname;\
	.set noreorder;\
	fname: \
	__VA_ARGS__ \
	.end fname;

//Force line size to 16B to fit all CPUs
.macro RANGED_CACHE_OPERATION_A0_START_A1_END, cache_op
	move  t0, a0
1:
	cacheop(\cache_op, t0)
	addi  t0, 16
	blt   t0, a1, 1b
	nop
	jr    ra
	nop
.endm

GFUNC_new(fetch_lock_icache_range,
	RANGED_CACHE_OPERATION_A0_START_A1_END Fetch_Lock_I;
)

GFUNC_new(writeback_dcache_range,
	RANGED_CACHE_OPERATION_A0_START_A1_END Hit_Writeback_D;
)

GFUNC_new(fetch_lock_dcache_range,
	RANGED_CACHE_OPERATION_A0_START_A1_END Fetch_Lock_D;
)
