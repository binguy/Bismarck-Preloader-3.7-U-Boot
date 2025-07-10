#ifndef SECTION_CACHE_FUNC
	#define SECTION_CACHE_FUNC .text
#endif

#define GCFUNC(fname, ...) \
	SECTION_CACHE_FUNC;\
	.global fname;\
	.ent fname;\
	.set noreorder;\
fname: \
	__VA_ARGS__ \
	.end fname;

#define DISABLE_CPC CPC_SWITCH 0
#define ENABLE_CPC  CPC_SWITCH 1
.macro CPC_SWITCH en
	li  k1, GCR_CPC_BASE
	li  k0, ((CPC_BASE_ADDR & 0x1fffffff) | \en)
	sw  k0, 0(k1)
.endm

.macro LOOP_IF_NOT_CORE0
	mfc0 k0, CP0_EBASE
	ext  k0, k0, 0, 2
_not_core0:
	bne  zero, k0, _not_core0
	nop
.endm

.macro CLOCK_OFF_CORE1
	ENABLE_CPC
	li  k1, CPC_CO_BASE_ADDR
	lui k0, 0x0001
	sw  k0, 0x10(k1)
	li  k0, 0x0001
	sw  k0, 0x00(k1)
	DISABLE_CPC
.endm

.macro CPU_CP0_INIT
cpu_cp0_init:
	lui   k0, 0x0040 //set BEV to direct exception to 0xbfc00380
	ori   k0, k0, 0x0004 //set ERL to block WP int.
	mtc0  k0, CP0_STATUS

	//Some CPUs have up to 8 sets. Extra mtc0 should be ok, tho.
	mtc0  zero, CP0_WATCHLO
	mtc0  zero, CP0_WATCHLO, 1
	mtc0  zero, CP0_WATCHLO, 2
	mtc0  zero, CP0_WATCHLO, 3
	mtc0  zero, CP0_WATCHLO, 4
	mtc0  zero, CP0_WATCHLO, 5
	mtc0  zero, CP0_WATCHLO, 6
	mtc0  zero, CP0_WATCHLO, 7

	li    k0, 0x7
	mtc0  k0, CP0_WATCHHI
	mtc0  k0, CP0_WATCHHI, 1
	mtc0  k0, CP0_WATCHHI, 2
	mtc0  k0, CP0_WATCHHI, 3
	mtc0  k0, CP0_WATCHHI, 4
	mtc0  k0, CP0_WATCHHI, 5
	mtc0  k0, CP0_WATCHHI, 6
	mtc0  k0, CP0_WATCHHI, 7

	mtc0  zero, CP0_CAUSE
	mtc0  zero, CP0_COUNT
	mtc0  zero, CP0_COMPARE
.endm

.macro MIPS_CACHE_V0_SZ_V1_LINE_SZ config1_cache_off
	mfc0  t0, CP0_MIPS_CONFIG1
	srl   a0, t0, \config1_cache_off //$ assoc.
	andi  a0, a0, 7
	addi  a0, a0, 1

	srl   a1, t0, (\config1_cache_off + 3) //$ line sz, pow(2)
	andi  a1, a1, 7
	addi  a1, a1, 1

	srl   a2, t0, (\config1_cache_off + 6) //$ set num., pow(2)
	andi  a2, a2, 7
	addi  a2, a2, 6

	li    t0, 1
	add   a2, a2, a1 //1 << (line sz + set num)
	sll   a2, t0, a2
	mul   v0, a2, a0 //$ size in byte, v0
	sll   v1, t0, a1 //$ line sz in byte, v1
.endm

.macro ENABLE_CACHEABLE_K0
enable_cacheable_k0:
	mfc0  t0, CP0_MIPS_CONFIG
	li    t1, ~(0x7)
	and   t0, t0, t1
	ori   t0, CP0_CONF_CACHABLE_NC_WB_WA //Non-Coherency, Write-Back, Allocate
	mtc0  t0, CP0_MIPS_CONFIG // write CP0_Config
	nop
.endm

.macro DISABLE_L23
disable_l23:
	mfc0  t0, CP0_MIPS_CONFIG2
	ori   t0, t0, (1<<12)
	mtc0  t0, CP0_MIPS_CONFIG2
	nop
.endm

.macro INIT_L23
enable_l23:
	lui   v0, 0x0004
	lui   a0, 0x8000;
	add   a1, a0, v0;
	mtc0  zero, CP0_L23TAGLO;
	mtc0  zero, CP0_L23DATALO;
	mtc0  zero, CP0_L23DATAHI;
	RANGED_CACHE_OPERATION_A0_START_A1_END Index_Store_Tag_L23;
.endm

.macro CONTINUE_WITH_ICACHE
	lui   ra, %hi(_continue_with_icache)
	addiu ra, %lo(_continue_with_icache)
	jr    ra
	nop
_continue_with_icache:
.endm

.macro CPU_INIT
	CPU_CP0_INIT
.endm

.macro CACHE_INIT
	GET_CPU_TYPE
	bnez  v0, _cache_init_common
	nop

_cache_init_mips_only:

	DISABLE_L23
	ENABLE_CACHEABLE_K0

_cache_init_common:
	jal   invalidate_icache_all
	nop

	CONTINUE_WITH_ICACHE

	jal   invalidate_dcache_all
	nop
.endm

//v0 == 1: 5x81; v0 == 0: MIPs
.macro GET_CPU_TYPE
	mfc0  v0, CP0_PRID;
	srl   v0, v0, 14;
	andi  v0, v0, 0x1;
.endm

#define CCTL_INV_DCACHE_ALL (1)
#define CCTL_INV_ICACHE_ALL (1<<1)
#define CCTL_WB_DCACHE_ALL (1<<8)
#define CCTL_WB_INV_DCACHE_ALL (1<<9)
.macro RLX_5X81_CCTL, cctl_op_bitmap
	mfc0  t0, CP0_5x81_CCTL;
	li    t1, ~(\cctl_op_bitmap);
	and   t0, t0, t1;
	mtc0  t0, CP0_5x81_CCTL;
	ori   t0, t0, (\cctl_op_bitmap);
	mtc0  t0, CP0_5x81_CCTL;
.endm

//Force line size to 16B. It may hurt performance but fit all CPUs.
//note that `sync' is required due to E47 of MD00907-1D-interAptiv-ERS-01.14.pdf
.macro RANGED_CACHE_OPERATION_A0_START_A1_END, cache_op
	move  t0, a0
1:
	cacheop(\cache_op, t0)
	sync
	addi  t0, 16
	blt   t0, a1, 1b
	nop
.endm

GCFUNC(invalidate_icache_all,
	GET_CPU_TYPE;
	beqz  v0, _iia_mips; nop;
_iia_5x81:
	RLX_5X81_CCTL CCTL_INV_ICACHE_ALL;
	jr    ra; nop;
_iia_mips:
	MIPS_CACHE_V0_SZ_V1_LINE_SZ MIPS_CONFIG1_ICACHE_OFF;
	lui   a0, 0x8000;
	add   a1, a0, v0;
	RANGED_CACHE_OPERATION_A0_START_A1_END Index_Invalidate_I;
	jr    ra; nop;
)

GCFUNC(invalidate_icache_range,
	RANGED_CACHE_OPERATION_A0_START_A1_END Hit_Invalidate_I;
	jr    ra; nop;
)

GCFUNC(fetch_lock_icache_range,
	RANGED_CACHE_OPERATION_A0_START_A1_END Fetch_Lock_I;
	jr    ra; nop;
)

GCFUNC(writeback_invalidate_dcache_all,
	GET_CPU_TYPE;
	beqz  v0, _wida_mips; nop;
_wida_5x81:
	RLX_5X81_CCTL CCTL_WB_INV_DCACHE_ALL;
	jr    ra; nop;
_wida_mips:
	MIPS_CACHE_V0_SZ_V1_LINE_SZ MIPS_CONFIG1_DCACHE_OFF;
	lui   a0, 0x8000;
	add   a1, a0, v0;
	RANGED_CACHE_OPERATION_A0_START_A1_END Index_Writeback_Inv_D;
	jr    ra; nop;
)

GCFUNC(writeback_invalidate_dcache_range,
	RANGED_CACHE_OPERATION_A0_START_A1_END Hit_Writeback_Inv_D;
	jr    ra; nop;
)

//GFUNC(writeback_dcache_all,
//	N/A: MIPS only supports Hit_Writeback_D, and no Index_Writeback_D
//	jr    ra;
//	nop;
//)

GCFUNC(writeback_dcache_range,
	RANGED_CACHE_OPERATION_A0_START_A1_END Hit_Writeback_D;
	jr    ra; nop;
)

GCFUNC(invalidate_dcache_all,
	GET_CPU_TYPE;
	beqz  v0, _ida_mips; nop;
_ida_5x81:
	RLX_5X81_CCTL CCTL_INV_DCACHE_ALL;
	jr    ra; nop;
_ida_mips:
	MIPS_CACHE_V0_SZ_V1_LINE_SZ MIPS_CONFIG1_DCACHE_OFF;
	lui   a0, 0x8000;
	add   a1, a0, v0;
	mtc0  zero, CP0_TAGLO;
	mtc0  zero, CP0_DTAGLO;
	RANGED_CACHE_OPERATION_A0_START_A1_END Index_Store_Tag_D;
	jr    ra; nop;
)

GCFUNC(invalidate_dcache_range,
	RANGED_CACHE_OPERATION_A0_START_A1_END Hit_Invalidate_D;
	jr    ra; nop;
)

GCFUNC(fetch_lock_dcache_range,
	RANGED_CACHE_OPERATION_A0_START_A1_END Fetch_Lock_D;
	jr    ra; nop;
)

GCFUNC(writeback_invalidate_l23_range,
	GET_CPU_TYPE;
	beqz  v0, _wil23r_mips; nop;
_wil23r_5x81:
	//5281's Hit_Writeback_Inv_D and Hit_Invalidate_I cover both L1 and L2
	jr    ra; nop;
_wil23r_mips:
	RANGED_CACHE_OPERATION_A0_START_A1_END Hit_Writeback_Inv_L23;
	jr    ra; nop;
)

//This func. assumes 256KB L2$ with line size 16B. For experiment ONLY.
GCFUNC(writeback_invalidate_l23_all,
	GET_CPU_TYPE;
	beqz  v0, _wil23a_mips; nop;
_wil23a_5x81:
	//Not impl. yet.
	jr    ra; nop;
_wil23a_mips:
	lui   v0, 0x0004;
	lui   a0, 0x8000;
	add   a1, a0, v0;
	RANGED_CACHE_OPERATION_A0_START_A1_END Index_Writeback_Inv_L23;
	jr    ra; nop;
)

GCFUNC(writeback_l23_range,
	GET_CPU_TYPE;
	beqz  v0, _wl23r_mips; nop;
_wl23r_5x81:
	//5281's Hit_Writeback_D already covers L1 and L2
	jr    ra; nop;
_wl23r_mips:
	RANGED_CACHE_OPERATION_A0_START_A1_END Hit_Writeback_L23;
	jr    ra; nop;
)

GCFUNC(writeback_l23_all,
	//	N/A: MIPS only supports Hit_Writeback_X, and no Index_Writeback_X
	//	jr    ra;
	//	nop;
	jr    ra; nop;
)

GCFUNC(invalidate_l23_range,
	GET_CPU_TYPE;
	beqz  v0, _il23r_mips; nop;
_il23r_5x81:
	//5281's Hit_Invalidate_D and Hit_Invalidate_I already cover L1 and L2
	jr    ra; nop;
_il23r_mips:
	RANGED_CACHE_OPERATION_A0_START_A1_END Hit_Writeback_L23;
	jr    ra; nop;
)

GCFUNC(invalidate_l23_all,
	//Not impl. yet.
	jr    ra; nop;
)
