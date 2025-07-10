#include <soc.h>

#define OC_BTM_START 0xb8005100
#define LX_BTM_START 0xb8005200
#define LX_BTM_END 0xb80052bf

__attribute__ ((unused))
static void disable_bus_timeout_mon(void) {
	u32_t lx_btm_addr = LX_BTM_START;

	/* OC0 BTM disable; OC1 one can only be set from OC1. */
	REG32(OC_BTM_START) &= (~(1<<31));

	do {
		REG32(lx_btm_addr) &= (~(1<<31));
		lx_btm_addr += 0x10;
	} while(lx_btm_addr < LX_BTM_END);

	return;
}

__attribute__ ((unused))
static void enable_bus_timeout_mon(void) {
	u32_t lx_btm_addr = LX_BTM_START;

	/* OC0 BTM enable; OC1 one can only be set from OC1. */
	REG32(OC_BTM_START) |= (1<<31);

	do {
		REG32(lx_btm_addr) |= (1<<31);
		lx_btm_addr += 0x10;
	} while(lx_btm_addr < LX_BTM_END);

	return;
}

REG_INIT_FUNC(disable_bus_timeout_mon, 34);
