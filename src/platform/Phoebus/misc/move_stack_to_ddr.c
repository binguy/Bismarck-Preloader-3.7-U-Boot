#include <init_define.h>
#include <dram/memcntlr_util.h>

UTIL_FAR void _change_sp_840(int new_sp, int stack_size_b);

static void change_sp_to_dram(void) {
	if (ISTAT_GET(cal) == MEM_CAL_OK) {
		u32_t old_sp;
		asm volatile("move %0, $29": "=r"(old_sp)::);
		_change_sp_840(NEW_STACK_AT_DRAM, (OTTO_PLR_STACK_DEF-old_sp));
		printf(
			"II: Change Stack from 0x%x to 0x%x\n",
			OTTO_PLR_STACK_DEF,
			NEW_STACK_AT_DRAM);
	} else {
		puts("EE: DRAM init. fail, stack pointer is not changed\n");
	}

	return;
}

REG_INIT_FUNC(change_sp_to_dram, 36);
