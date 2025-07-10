#ifndef CPU_H
#define CPU_H

//For MIPS IA
#define GCR_ADDR    (0xBFBF8000)

#define CKUSEG 0x00000000
#define CKSEG0 0x80000000
#define CKSEG1 0xa0000000
#define CKSEG2 0xc0000000
#define CKSEG3 0xe0000000

#define CP0_INDEX       $0
#define CP0_MVPCNTL     $0,1
#define CP0_MVPCONF0    $0,2
#define CP0_MVPCONF1    $0,3

#define CP0_RANDOM      $1
#define CP0_VPECNTL     $1,1
#define CP0_VPECONF0    $1,2
#define CP0_VPECONF1    $1,3
#define CP0_YQMASK      $1,4
#define CP0_VPESCHEDULE $1,5
#define CP0_VPESCHEFBACK $1,6
#define CP0_VPEOPT      $1,7

#define CP0_ENTRYLO     $2
#define CP0_ENTRYLO0    $2
#define CP0_TCSTATUS    $2,1
#define CP0_TCBIND      $2,2
#define CP0_TCRESTART   $2,3
#define CP0_TCHALT      $2,4
#define CP0_TCCONTEXT   $2,5
#define CP0_TCSCHEDULE  $2,6
#define CP0_TCSCHEFBACK $2,7

#define CP0_ENTRYLO1    $3
#define CP0_TCOPT       $3,7

#define CP0_CONTEXT     $4
#define CP0_CONTEXTCONF $4,1
#define CP0_USERLOCAL   $4,2

#define CP0_PAGEMASK    $5
#define CP0_SEGCTL0     $5,2
#define CP0_SEGCTL1     $5,3
#define CP0_SEGCTL2     $5,4

#define CP0_WIRED       $6
#define CP0_SRSCONF0    $6,1
#define CP0_SRSCONF1    $6,2
#define CP0_SRSCONF2    $6,3
#define CP0_SRSCONF3    $6,4
#define CP0_SRSCONF4    $6,5

#define CP0_HWRENA      $7

#define CP0_BADVADDR    $8
#define CP0_COUNT       $9
#define CP0_ENTRYHI     $10
#define CP0_COMPARE     $11

#define CP0_STATUS      $12
#define CP0_INTCTL      $12,1
#define CP0_SRSCTL      $12,2
#define CP0_SRSMAP      $12,3

#define CP0_CAUSE       $13
#define CP0_EPC         $14

#define CP0_PRID        $15
#define CP0_EBASE       $15,1
#define CP0_CDMMBASE    $15,2

#define CP0_5x81_DREG   $16
#define CP0_MIPS_CONFIG $16
#define CP0_MIPS_CONFIG1 $16,1
#define CP0_MIPS_CONFIG2 $16,2
#define CP0_MIPS_CONFIG3 $16,3
#define CP0_MIPS_CONFIG4 $16,4
#define CP0_MIPS_CONFIG5 $16,5
#define CP0_MIPS_CONFIG7 $16,7

#define CP0_5x81_DEPC   $17
#define CP0_LLADDR      $17

#define CP0_WATCHLO     $18
#define CP0_WATCHLO0    $18
#define CP0_WATCHLO1    $18,1
#define CP0_WATCHLO2    $18,2
#define CP0_WATCHLO3    $18,3

#define CP0_WATCHHI     $19
#define CP0_WATCHHI0    $19
#define CP0_WATCHHI1    $19,1
#define CP0_WATCHHI2    $19,2
#define CP0_WATCHHI3    $19,3

#define CP0_5x81_CCTL   $20

#define CP0_5x81_CONFIG $23
#define CP0_DEBUG       $23
#define CP0_TRACECNTL   $23,1
#define CP0_TRACECNTL2  $23,2
#define CP0_USERTRACEDATA1 $23,3
#define CP0_TRACEIBPC   $23,4
#define CP0_TRACEDBPC   $23,5

#define CP0_DEPC        $24
#define CP0_TRACECNTL3  $24,2
#define CP0_USERTRACEDATA2 $24,3

#define CP0_PERFORMANCE $25
#define CP0_PERFCTL0    $25
#define CP0_PERFCNT0    $25,1
#define CP0_PERFCTL1    $25,2
#define CP0_PERFCNT1    $25,3

#define CP0_ERRCTL      $26
#define CP0_CACHEERR    $27

#define CP0_5x81_DATALO $28
#define CP0_TAGLO       $28
#define CP0_ITAGLO      $28
#define CP0_IDATALO     $28,1
#define CP0_DTAGLO      $28,2
#define CP0_DDATALO     $28,3
#define CP0_L23TAGLO    $28,4
#define CP0_L23DATALO   $28,5

#define CP0_5x81_DATAHI $29
#define CP0_IDATAHI     $29,1
#define CP0_DTAGHI      $29,2
#define CP0_L23DATAHI   $29,5

#define CP0_ERROREPC    $30

#define CP0_DESAVE      $31
#define CP0_KSCRATCH0   $31,2
#define CP0_KSCRATCH1   $31,3
#define CP0_KSCRATCH2   $31,4

#define CP0_STATUS_IE   (1<<0)
#define CP0_STATUS_EXL  (1<<1)
#define CP0_STATUS_ERL  (1<<2)

#define CP0_CONF_CACHABLE_NC_WT_nWA   0
#define CP0_CONF_UNCACHED             2
#define CP0_CONF_CACHABLE_NC_WB_WA    3
#define CP0_CONF_UNCACHED_ACCELERATED 7
#define CP0_CONF_CACHE_MASK           0x7

/* MIPS, 5281, 5181 */
#define Hit_Invalidate_D 0x11
#define Hit_Writeback_Inv_D 0x15
#define Hit_Writeback_D 0x19

/* MIPS, 5281 */
#define Hit_Invalidate_I 0x10

/* MIPS */
#define Index_Invalidate_I 0x00
#define Index_Writeback_Inv_D 0x01
#define Index_Writeback_Inv_L23 0x03
#define Index_Load_Tag_I 0x04
#define Index_Load_Tag_D 0x05
#define Index_Store_Tag_I 0x08
#define Index_Store_Tag_D 0x09
#define Index_Store_Tag_L23 0x0b
#define Hit_Invalidate_L23 0x13
#define Fill_I 0x14
#define Hit_Writeback_Inv_L23 0x17
#define Hit_Writeback_L23 0x1b
#define Fetch_Lock_I 0x1c
#define Fetch_Lock_D 0x1d

#define MIPS_CONFIG1_ICACHE_OFF (16)
#define MIPS_CONFIG1_DCACHE_OFF (7)

#define zero $0
#define AT   $1
#define v0   $2  /* return value */
#define v1   $3
#define a0   $4  /* argument registers */
#define a1   $5
#define a2   $6
#define a3   $7
#define t0   $8  /* caller saved */
#define t1   $9
#define t2   $10
#define t3   $11
#define t4   $12
#define t5   $13
#define t6   $14
#define t7   $15
#define s0   $16 /* callee saved */
#define s1   $17
#define s2   $18
#define s3   $19
#define s4   $20
#define s5   $21
#define s6   $22
#define s7   $23
#define t8   $24 /* caller saved */
#define t9   $25
#define k0   $26
#define k1   $27
#define gp   $28
#define sp   $29
#define s8   $30
#define ra   $31

#ifndef ASM_NL
#define ASM_NL ";\n\t"
#endif

#define DCACHE_LINE_SIZE	(CACHELINE_SIZE)
#define ICACHE_LINE_SIZE	(CACHELINE_SIZE)

#define cacheop(cmd, addr_reg) cache cmd, 0(addr_reg)

#define set_zero_64(dst_reg)	  \
	sw	zero, (0*4 - 64)(dst_reg); \
	sw	zero, (1*4 - 64)(dst_reg); \
	sw	zero, (2*4 - 64)(dst_reg); \
	sw	zero, (3*4 - 64)(dst_reg); \
	sw	zero, (4*4 - 64)(dst_reg); \
	sw	zero, (5*4 - 64)(dst_reg); \
	sw	zero, (6*4 - 64)(dst_reg); \
	sw	zero, (7*4 - 64)(dst_reg); \
	sw	zero, (8*4 - 64)(dst_reg); \
	sw	zero, (9*4 - 64)(dst_reg); \
	sw	zero, (10*4 - 64)(dst_reg); \
	sw	zero, (11*4 - 64)(dst_reg); \
	sw	zero, (12*4 - 64)(dst_reg); \
	sw	zero, (13*4 - 64)(dst_reg); \
	sw	zero, (14*4 - 64)(dst_reg); \
	sw	zero, (15*4 - 64)(dst_reg);

/* So far RS is safe on MIPS so no actual disable code */
#define RETURN_STACK_SAVE(_bpctl_backup)
#define RETURN_STACK_RESTORE(_bpctl_backup)

#define __asm_mfc0(mfc_reg, mfc_sel) ({	\
			unsigned int __ret;	\
			__asm__ __volatile__ ( \
				"mfc0 %0," TO_STR(mfc_reg) "," TO_STR(mfc_sel) ";\n\t" \
				: "=r" (__ret)); \
			__ret;})

#define __asm_mtc0(value, mtc_reg, mtc_sel) ({	  \
			unsigned int __value=(value); \
			__asm__ __volatile__ ( \
			                      "mtc0 %0, " TO_STR(mtc_reg) "," TO_STR(mtc_sel) ";\n\t" \
			                      : : "r" (__value)); })

#define asm_mfc0(mfc_reg) __asm_mfc0(mfc_reg, 0)
#define asm_mtc0(value, mtc_reg) __asm_mtc0(value, mtc_reg, 0)
//#define asm_mtc0_1(value, mtc_reg) __asm_mtc0(value, mtc_reg, 1)
//#define asm_mfc0_2(mfc_reg) __asm_mfc0(mfc_reg, 2)
//#define asm_mtc0_2(value, mtc_reg) __asm_mtc0(value, mtc_reg, 2)
//#define asm_mfc0_3(mfc_reg) __asm_mfc0(mfc_reg, 3)

#define CPU_GET_CP0_CYCLE_COUNT()	  \
	({ int __res; \
		__asm__ __volatile__("mfc0 %0, $9;" \
		                     : "=r" (__res)); \
		__res; \
	})

#define CPU_GET_STACK_PTR()	  \
	({ int __res; \
		__asm__ __volatile__("move %0, $29" \
		                     : "=r"(__res)); \
		__res; \
	})

/* _MIPS_ARCH_xxxxx comes with toolkit: "$gcc -dM -E - < /dev/null" */
#if defined(_MIPS_ARCH_5281)
#	define OTTO_CPU_RLXMIPS 1
# define OTTO_CPU_MIPS32R2 0
#elif defined(_MIPS_ARCH_MIPS32R2)
#	define OTTO_CPU_RLXMIPS 0
# define OTTO_CPU_MIPS32R2 1
#else
#	error "EE: unknown CPU ARCH\n"
#endif

#ifndef __ASSEMBLER__
extern char* otto_cpu_name(void);
extern int otto_cache_line_size(int cache_type);
extern void otto_cache_dump_info(void);
extern void invalidate_icache_all(void);
extern void invalidate_icache_range(unsigned int start, unsigned int end);
extern void fetch_lock_icache_range(unsigned int start, unsigned int end);
extern void writeback_invalidate_dcache_all(void);
extern void writeback_invalidate_dcache_range(unsigned int start, unsigned int end);
extern void writeback_dcache_all(void);
extern void writeback_dcache_range(unsigned int start, unsigned int end);
extern void invalidate_dcache_all(void);
extern void invalidate_dcache_range(unsigned int start, unsigned int end);
extern void fetch_lock_dcache_range(unsigned int start, unsigned int end);
extern void writeback_invalidate_l23_all(void);
extern void writeback_invalidate_l23_range(unsigned int start, unsigned int end);
extern void writeback_l23_all(void);
extern void writeback_l23_range(unsigned int start, unsigned int end);
extern void invalidate_l23_all(void);
extern void invalidate_l23_range(unsigned int start, unsigned int end);
#endif

#endif //CPU_H
