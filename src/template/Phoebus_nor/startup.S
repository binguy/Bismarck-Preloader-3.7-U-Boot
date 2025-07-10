#include <soc.h>
#include <cpu/cpu.s>

#define SRAM_0KB 0x0
#define SRAM_4KB 0x5
#define SRAM_8KB 0x6
#define SRAM_16KB 0x7
#define SRAM_32KB 0x8
#define SRAM_64KB 0x9
#define SRAM_128KB 0xa
#define SRAM_256KB 0xb

.macro BUSY_LOOP
1:
	b       1b
	nop
.endm

// this loop is used to create a time window so ICE can break to debug and recover.
.macro DEBUG_LOOP delay
	.set noreorder
	li      t0, \delay
1:
	bnez    t0, 1b
	addi    t0, -1
.endm

	.section .exc_reset, "ax", @progbits
	.align 4
	.global plr_S_entry
	.ent plr_S_entry
plr_S_entry:
	DEBUG_LOOP 0x6000
	j       ucbasic_init
	nop

	.global always_return_zero
always_return_zero:
	.global bp_barrier
bp_barrier:
	jr      ra
	move    v0, zero
	.end plr_S_entry

	.section .exc_utlb, "ax", @progbits
	.align  4
utlb_handler:
	BUSY_LOOP

	.section .exc_rest, "ax", @progbits
	.align  4
exc_handler:
	BUSY_LOOP

	.section .flash_text, "ax", @progbits
	.align  4
	.set    noreorder
	.ent    ucbasic_init
ucbasic_init:
	CPU_INIT

	CACHE_INIT

	li      a0, SRAM_SIZE_SEG0
	li      a1, SRAM_SIZE_SEG1
	li      a2, SRAM_SIZE_SEG2
	li      a3, SRAM_SIZE_SEG3
	jal     sram_cntlr_init
	lui     sp, 0xdead

_init_sram_content:
	la      a0, OTTO_SRAM_START
	la      a1, OTTO_ENTRY
	lui     a2, %hi(end_of_sram - OTTO_SRAM_START)
	addi    a2, %lo(end_of_sram - OTTO_SRAM_START)
	jal     memcpy
	lui     sp, 0xdead

	la      a0, OTTO_SRAM_START
	la      a1, end_of_sram
	jal     writeback_dcache_range
	lui     sp, 0xdead

_init_bss:
	la      t0, start_of_bss
	la      t1, end_of_bss - 4
1:
	sw      zero, 0(t0)
	bne     t0, t1, 1b
	addi    t0, t0, 4

_init_stack_guard:
	la      t0, sp_bottom
	addi    t1, t0, 32 - 4
	li      t2, 0xcafecafe
1:
	sw      t2, 0(t0)
	bne     t0, t1, 1b
	addi    t0, t0, 4

_start_traversal_init_table:
	la      a0, traversal_init_table
	lui     sp, %hi(OTTO_PLR_STACK_DEF)
	jr      a0
	ori     sp, %lo(OTTO_PLR_STACK_DEF)
	.end    ucbasic_init
