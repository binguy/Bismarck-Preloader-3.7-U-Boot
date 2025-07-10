#ifndef __OCP_PLL_H__
#define __OCP_PLL_H__

SKC35_REG_DEC(
	SYSPLLCTR, 0xb8000200,
	/* union { */
	/* 	/\* Original naming, cksel_sram500, is so confusing. */
	/* 	   Rename it to proper meaning for software. *\/ */
	/* 	RFIELD(31, 31, cksel_sram500); */
	/* 	RFIELD(31, 31, sram_no_div2); */
	/* }; */
	RFIELD(31, 31, sram_no_div2);
	RF_RSV(30, 22);
	RFIELD(21, 16, ocp0_pll_div);
	RF_RSV(15, 14);
	RFIELD(13, 8, ocp1_pll_div);
	RF_RSV(7, 5);
	RFIELD(4, 0, sdpll_div);
	);

SKC35_REG_DEC(
	PLLCCR, 0xb8000204,
	RFIELD(31, 31, pllddr_en);
	RFIELD(30, 30, pll_ddr_rstb_in);
	RFIELD(29, 28, ssc_test_mode);
	RFIELD(27, 20, ssc_offset);
	RFIELD(19, 14, ssc_step);
	RFIELD(13, 7, ssc_period);
	RFIELD(6, 6, ssc_en);
	RFIELD(5, 5, pllocp1_en);
	RFIELD(4, 4, reg_sel_500m);
	RFIELD(3, 3, frac_en);
	RFIELD(2, 2, pllddr_fupdn);
	RFIELD(1, 1, pllocp0_en);
	RFIELD(0, 0, pll500m_en);
	);

SKC35_REG_DEC(
	PLL1, 0xb800020c,
	RF_DNC(31, 19);
	RFIELD(18, 18, reg_en_div2_cpu0);
	RF_DNC(17, 0);
	);

#endif  //__OCP_PLL_H__
