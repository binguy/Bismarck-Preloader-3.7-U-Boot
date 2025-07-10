#include <init_define.h>
#include <uart/uart.h>
#include <dram/memcntlr_reg.h>
#include <dram/memcntlr_util.h>

#ifdef OTTO_PROJECT_FPGA
static void delay_for_setting(void) {
	volatile u32_t i=10000;
	while (i>0) i--;
}

static void ddr123_static_setup(void) {
	const int dram_type = RFLD_MCR(dram_type) + 1;
	printf("II: FPGA(V6) DDR%d Init ...\n", dram_type);

	if (dram_type == 1) {
		REG32(0xb8001000)=0x00000060;
		REG32(0xb8001004)=0x11220000;
		REG32(0xb8001008)=0x03000800;
		REG32(0xb800100c)=0x00000000;
		REG32(0xb8001010)=0x00301000;
		REG32(0xb800101c)=0x00100061;
		delay_for_setting();
		REG32(0xb8001500)=0xc0000000;
		REG32(0xb8001500)=0xc0000010;
	} else if(dram_type == 2) {
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
	} else if(dram_type == 3) {
		REG32(0xb8001064) = 0x22220000;
		REG32(0xb8001004) = 0x21220000;
		REG32(0xb8001008) = 0x44433804;
		REG32(0xb800100c) = 0x02000301;
		REG32(0xb8001010) = 0x00401000;

		REG32(0xb800101c) = 0x00120000;
		delay_for_setting();
		REG32(0xb800101c) = 0x00130000;
		delay_for_setting();
		REG32(0xb800101c) = 0x00110043;
		delay_for_setting();

#if 0
		puts(" Enable DLL mode for Samsung DDR3 ...\n");
		REG32(0xb800101c) = 0x00110042;
		delay_for_setting();
#else
		puts(" Disable DLL mode for Winbond ...\n");
#endif

		REG32(0xb800101c) = 0x00100320;
		delay_for_setting();

		REG32(0xb8001080) = 0x80000000;
		delay_for_setting();

		puts(" Enable dynamic DDR PHY FIFO buffer pointer reset\n");
		REG32(0xb8001500) = 0x80000000;
		REG32(0xb8001500) = 0x80000030;

		REG32(0xb80010b0) = 0x00FFFFFF;
		REG32(0xb80010b4) = 0x00111111;
		REG32(0xb80010C0) = 0x11111110;
		REG32(0xb80010C4) = 0x00000011;

		delay_for_setting();
		REG32(0xb800101c) = 0x00100320;

		// Enable Memory controller "outstanding ECO"
		RMOD_DCR(eco_rd_buf_mech_iA,1);
		RMOD_DCR(eco_rd_buf_mech_5281,1);
		DMCRrv = DMCRrv;
	}
}
#else
void memctlc_init_dram(void); // at ./ext/xxxx/autok/memctl.c

static void dram_calibration_entry(void) {
	puts("II: DRAM calibration starts...\n");
	memctlc_init_dram();
}
#endif

void dram_setup(void) {
#ifdef OTTO_PROJECT_FPGA
	ddr123_static_setup();
#else
	dram_calibration_entry();
#endif

	ISTAT_SET(cal, MEM_CAL_OK);
}
REG_INIT_FUNC(dram_setup, 28);
