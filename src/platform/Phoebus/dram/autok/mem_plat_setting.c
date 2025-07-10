#include <soc.h>
#include "mem_plat_setting.h"
#include <register_map.h>
#include <dram/memcntlr_reg.h>
#include "./memctl.h"
#include "./bspchip.h"



void memcntlr_plat_preset(void) {

    printf("AK: MEMPLL[31/63/95/127/159/191/223]=%x, %x, %x, %x, %x, %x, %x\n",
        REG32(MEMPLL31_0), REG32(MEMPLL63_32), REG32(MEMPLL95_64), REG32(MEMPLL127_96),
        REG32(MEMPLL159_128), REG32(MEMPLL191_160), REG32(MEMPLL223_192));
	return;
}

void memcntlr_plat_postset(void) {
	//check register
	printf("AK: DCR=0x%x, DTR[0:2]=0x%x, 0x%x, 0x%x\n",REG32(DCR),REG32(DTR0),REG32(DTR1),REG32(DTR2));

	return;
}
