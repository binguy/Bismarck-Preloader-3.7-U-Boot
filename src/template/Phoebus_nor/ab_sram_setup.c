#include <soc.h>

typedef unsigned int uint32_t;

/* This function should not use *SP* and invoked before C-env properly init. */
void sram_cntlr_init(uint32_t seg1_sz, uint32_t seg2_sz,
                     uint32_t seg3_sz, uint32_t seg4_sz) {
	REG32(0xb8001304) = seg1_sz;
	REG32(0xb8004004) = seg1_sz;
	REG32(0xb8001314) = seg2_sz;
	REG32(0xb8004014) = seg2_sz;
	REG32(0xb8001324) = seg3_sz;
	REG32(0xb8004024) = seg3_sz;
	REG32(0xb8001334) = seg4_sz;
	REG32(0xb8004034) = seg4_sz;

	if (seg1_sz) seg1_sz = (1 << (seg1_sz + 7));
	if (seg2_sz) seg2_sz = (1 << (seg2_sz + 7));
	if (seg3_sz) seg3_sz = (1 << (seg3_sz + 7));
	if (seg4_sz) seg4_sz = (1 << (seg4_sz + 7));
	REG32(0xb8004008) = 0;
	REG32(0xb8004018) = seg1_sz;
	REG32(0xb8004028) = seg1_sz + seg2_sz;
	REG32(0xb8004038) = seg1_sz + seg2_sz + seg3_sz;

	if (seg1_sz) seg1_sz = 1;
	REG32(0xb8001300) = 0x1f000000 | seg1_sz;
	REG32(0xb8004000) = *((uint32_t *)0xb8001300);

	if (seg2_sz) seg2_sz = 1;
	REG32(0xb8001310) = 0x1f000000 | *((uint32_t *)0xb8004018) | seg2_sz;
	REG32(0xb8004010) = *((uint32_t *)0xb8001310);

	if (seg3_sz) seg3_sz = 1;
	REG32(0xb8001320) = 0x1f000000 | *((uint32_t *)0xb8004028) | seg3_sz;
	REG32(0xb8004020) = *((uint32_t *)0xb8001320);

	if (seg4_sz) seg4_sz = 1;
	REG32(0xb8001330) = 0x1f000000 | *((uint32_t *)0xb8004038) | seg4_sz;
	REG32(0xb8004030) = *((uint32_t *)0xb8001330);

	return;
}
