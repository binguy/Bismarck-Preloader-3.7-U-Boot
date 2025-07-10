#include <util.h>
#include <bus_pri/bpsa_reg_map.h>

void bpsa_change_bus_priority(void) {
	// 1. Enable BPSA
	BPSA_CTRLrv = 0x0;
	BPSA_CTRLrv = 0x1;

	//puts("II: BPSA Setting ");

	BPSA_DFT_PRI0rv = 0x7b2345a9;
	BPSA_DFT_PRI1rv = 0x000018c6;
	B0_PSAR0rv = 0x10101010;
	B0_PSAR1rv = 0x10101010;
	B1_PSAR0rv = 0x01010101;
	B1_PSAR1rv = 0x01010101;
	B2_PSAR0rv = 0x02020202;
	B2_PSAR1rv = 0x02020202;
	B3_PSAR0rv = 0x41041041;
	B3_PSAR1rv = 0x10410410;
	B4_PSAR0rv = 0x24924924;
	B4_PSAR1rv = 0x49249249;
	B5_PSAR0rv = 0x92492492;
	B5_PSAR1rv = 0x24924924;
	B6_PSAR0rv = 0xffffffff;
	B6_PSAR1rv = 0xffffffff;
	B7_PSAR0rv = 0xcccccccc;
	B7_PSAR1rv = 0xcccccccc;
	B8_PSAR0rv = 0x33333333;
	B8_PSAR1rv = 0x33333333;
	B9_PSAR0rv = 0xffffffff;
	B9_PSAR1rv = 0xffffffff;
	B10_PSAR0rv = 0x49249249;
	B10_PSAR1rv = 0x92492492;
	B11_PSAR0rv = 0x20202020;
	B11_PSAR1rv = 0x20202020;

	//3. These register take effect after DMCR is updated.
	u32_t dmcr_val = DMCRrv;
	DMCRrv = dmcr_val;
	//puts("done\n");
	return;
}
REG_INIT_FUNC(bpsa_change_bus_priority, 17);
