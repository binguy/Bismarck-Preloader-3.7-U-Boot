#include <init_define.h>
#include <dram/memcntlr.h>
#include <autoconf.h>

unsigned int get_memory_ddr2_dram_odt_parameters(unsigned int *para_array);
unsigned int get_memory_dram_reduce_drv_parameters(unsigned int *para_array);
unsigned int get_memory_ddr3_dram_rtt_nom_parameters(unsigned int *para_array);
unsigned int get_memory_ddr3_dram_rtt_wr_parameters(unsigned int *para_array);

#define _REDUCE_CALIBRATION_

#undef CACHELINE_SIZE
#undef ICACHE_SIZE
#undef DCACHE_SIZE

void puthex(int h) {
	printf("%x", h);
	return;
}

#include <boot0412/mips_cache_ops.c>
#include <boot0412/memctl_cali_dram.c>
#include <boot0412/memctl_utils.c>

void dram_ldo_setup(void);

void delay_for_setting(void){
	volatile u32_t i=10000;
	while(i>0) i--;
}

void dram_setup(void) {
#ifdef OTTO_PROJECT_FPGA
	if (RFLD_MCR(dram_type)==1) {
		printf("II: FPGA(V6) DDR2 Init ...\n");

		REG32(0xb8001064)=0x22220000;
		REG32(0xb8001000)=0x10205c60;
		delay_for_setting();
		REG32(0xb8001004)=0x11220000;
		REG32(0xb8001008)=0x41301804;
		REG32(0xb800100c)=0x00000002;
		REG32(0xb8001010)=0x00502000;

		REG32(0xb800101c)=0x00120000;
		delay_for_setting();
		REG32(0xb800101c)=0x00130000;
		delay_for_setting();
		REG32(0xb800101c)=0x00110001;
		delay_for_setting();
		REG32(0xb800101c)=0x00110000;
		delay_for_setting();
		REG32(0xb800101c)=0x00100352;
		delay_for_setting();
		REG32(0xb800101c)=0x00100252;
		delay_for_setting();
		REG32(0xb8001500)=0x80000000;
		delay_for_setting();
		REG32(0xb8001500)=0x80000010;
		delay_for_setting();
	}
#else
	printf("II: LDO setup...\n");
	dram_ldo_setup();

	printf("II: DRAM init...\n");
	memctlc_init_dram();
#endif
	ISTAT_SET(cal, MEM_CAL_OK);

	return;
}

REG_INIT_FUNC(dram_setup, 30);
