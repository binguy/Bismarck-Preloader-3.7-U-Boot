#include <soc.h>

SECTION_RECYCLE
void text_of_snaf(void) {
	*((volatile unsigned int *)(0xb8003220)) = 0xc0de5e7;
	return;
}

REG_INIT_FUNC(text_of_snaf, 1);
