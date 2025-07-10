#ifndef __CMU_H__
#define __CMU_H__

#define CMU_MODE_DISABLE 0
#define CMU_MODE_FIX 1
#define CMU_MODE_DYNAMIC 2

SKC35_REG_DEC(
	CMUGCR_OC0, 0xb8000380,
	RFIELD(31, 31, busy);
	RF_RSV(30, 7);
	RFIELD(6, 4, freq_div);
	RF_DNC(3, 2);
	RFIELD(1, 0, cmu_mode);
	);

SKC35_REG_DEC(
	CMUSDCR_OC0, 0xb8000384,
	RF_RSV(30, 3);
	RFIELD(2, 0, dly_base);
	);

SKC35_REG_DEC(
	CMUCR_OC0, 0xb8000388,
	RFIELD(31, 31, auto_bz);
	RF_RSV(30, 29);
	RFIELD(28, 28, int_cxn_type);
	RF_RSV(27, 20);
	RFIELD(19, 19, se_spif_wk);
	RFIELD(18, 18, se_spif_slp);
	RFIELD(17, 17, se_spif);
	RFIELD(16, 16, se_spif_hs);
	RFIELD(15, 15, se_sram_rom_wk);
	RFIELD(14, 14, se_sram_rom_slp);
	RFIELD(13, 13, se_sram_rom);
	RFIELD(12, 12, se_sram_rom_hs);
	RFIELD(11, 11, se_dram_wk);
	RFIELD(10, 10, se_dram_slp);
	RFIELD(9, 9, se_dram);
	RFIELD(8, 8, se_dram_hs);
	RF_RSV(7, 4);
	RFIELD(3, 0, dly_mul);
	);

SKC35_REG_DEC(
	CMUSCR_OC0, 0xb800038c,
	RFIELD(31, 0, bus_slp_cnt);
	);

SKC35_REG_DEC(
	LBSBCR, 0xb80003a0,
	RF_RSV(31, 23);
	RFIELD(22, 22, axi_frq_slower);
	RFIELD(21, 21, oc3_frq_slower);
	RFIELD(20, 20, pbo_egw_lx_frq_slower);
	RFIELD(19, 19, pbo_usr_lx_frq_slower);
	RFIELD(18, 18, pbo_usw_lx_frq_slower);
	RFIELD(17, 17, pbo_dsr_lx_frq_slower);
	RFIELD(16, 16, pbo_dsw_lx_frq_slower);
	RF_RSV(15, 9);
	RFIELD(8, 8, lxp_frq_slower);
	RF_RSV(7, 4);
	RFIELD(3, 3, lx3_frq_slower);
	RFIELD(2, 2, lx2_frq_slower);
	RFIELD(1, 1, lx1_frq_slower);
	RFIELD(0, 0, lx0_frq_slower);
	);

#endif //__CMU_H__
