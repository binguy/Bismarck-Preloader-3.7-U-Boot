#include <soc.h>

	.section .unswapable_text
	.set noreorder
	.globl _change_sp_840
	.ent _change_sp_840
// a0: new SP
// a1: current size in byte
_change_sp_840:
	move t0, a0
	add  t1, a0, a1

_stack_copy:
	lw   t2, 0(sp)
	addi sp, sp, 4
	sw   t2, 0(t0)
	blt  t0, t1, _stack_copy
	addi t0, t0, 4

	jr   ra
	move sp, a0
	.end _change_sp_840
