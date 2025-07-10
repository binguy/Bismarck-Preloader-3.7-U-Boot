#ifndef __MEM_PLL_H__
#define __MEM_PLL_H__

SKC35_REG_DEC(
	MEM_PLL_CTRL0, 0xb8000234,
	RFIELD(31, 27, crt_post_pi_sel3);
	RFIELD(26, 22, crt_post_pi_sel2);
	RFIELD(21, 17, crt_post_pi_sel1);
	RFIELD(16, 12, crt_post_pi_sel0);
	RF_RSV(11, 7);
	RFIELD(6, 0, crt_en_post_pi);
	);

SKC35_REG_DEC(
	MEM_PLL_CTRL1, 0xb8000238,
	RFIELD(31, 30, crt_cco_band);
	RF_RSV(29, 10);
	RFIELD(9, 5, crt_post_pi_sel5);
	RFIELD(4, 0, crt_post_pi_sel4);
	);

SKC35_REG_DEC(
	MEM_PLL_CTRL2, 0xb800023c,
	RF_RSV(31, 31);
	RFIELD(30, 30, crt_ckref_sel);
	RFIELD(29, 28, crt_v10_ldo_vsel);
	RFIELD(27, 27, crt_post_pi_rs);
	RFIELD(26, 26, crt_pll_sel_cpmode);
	RFIELD(25, 24, crt_post_pi_rl);
	RFIELD(23, 22, crt_post_pi_bias);
	RFIELD(21, 20, crt_ldo_vsel);
	RFIELD(19, 19, crt_pll_debug_enable);
	RFIELD(18, 16, crt_lpf_sr);
	RFIELD(15, 14, crt_pdiv);
	RFIELD(13, 13, crt_lpf_cp);
	RFIELD(12, 12, crt_cco_kvco);
	RFIELD(11, 8, crt_icp);
	RFIELD(7, 5, crt_loop_pi_isel);
	RF_RSV(4, 0);
	);

SKC35_REG_DEC(
	MEM_PLL_CTRL3, 0xb8000240,
	RFIELD(31, 24, crt_n_code);
	RFIELD(23, 16, crt_pll_dum);
	RFIELD(15, 11, crt_post_pi_sel);
	RF_RSV(10, 6);
	RFIELD(5, 0, crt_oesync_opsel);
	);

SKC35_REG_DEC(
	MEM_PLL_CTRL4, 0xb8000244,
	RF_RSV(31, 22);
	RFIELD(21, 16, crt_ph_sel);
	RF_RSV(15, 12);
	RFIELD(11, 6, crt_dpi_mck_clk_en);
	RFIELD(5, 0, crt_clk_oe);
	);

#define CG_MEM_PLL_OE_DIS() RMOD(MEM_PLL_CTRL4, crt_clk_oe, 0);
#define CG_MEM_PLL_OE_EN() RMOD(MEM_PLL_CTRL4, crt_clk_oe, 0x3f);

#endif  //__MEM_PLL_H__
