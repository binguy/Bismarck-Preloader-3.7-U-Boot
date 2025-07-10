#ifdef OTTO_PROJECT_FPGA
#include <cg/apro_fpga_cg.c>
#else
#include <soc.h>
#include <uart/uart.h>
#include <cg/cg.h>
#include <util.h>

#ifndef SECTION_CG
#define SECTION_CG
#endif
#ifndef SECTION_CG_INFO
#define SECTION_CG_INFO
#endif

static void reg_to_mhz(void);
cg_info_t cg_info_query SECTION_CG_INFO;
u32_t cg_status SECTION_CG_INFO;

SECTION_CG
u32_t cg_query_freq(u32_t dev_type) {
    if (0 == cg_info_query.dev_freq.ocp_mhz) {
        reg_to_mhz();
    }

	switch(dev_type){
	case CG_DEV_OCP:
		return cg_info_query.dev_freq.ocp_mhz;
	case CG_DEV_MEM:
		return cg_info_query.dev_freq.mem_mhz;
	case CG_DEV_LX:
		return cg_info_query.dev_freq.lx_mhz;
	case CG_DEV_SPIF:
		return cg_info_query.dev_freq.spif_mhz;
	default:
		return (u32_t)(-1);
	}
}

SECTION_CG
void cg_copy_info_to_sram(void) {
	inline_memcpy(&cg_info_query, &cg_info_proj, sizeof(cg_info_t));
}

#if 0
SECTION_CG
static u32_t _cg_calc_lx(s16_t targ_freq,
                         cg_register_set_t *rs) {
	u32_t divisor = (1000 / targ_freq) - 2;
	u32_t cg_status = 0;
	if ((divisor < 1) || (divisor > 17)) {
		cg_status |= CG_LX_WRONG;
	}
	rs->lx_clk_pll.f.lxpll_div = divisor;
	/* printf("DD: LX: %dMHz, divisor: %d\n", df->lx_mhz, divisor); */
	return cg_status;
}
#endif
SECTION_CG
static u32_t _cg_calc_mem(s16_t targ_freq,
                          cg_register_set_t *rs) {
	u32_t factor, divisor;
	u32_t cg_status = 0;

	if (targ_freq < 100) {
		/* 1 << '2' ==> 4 */
		divisor = 2;
	} else if (targ_freq < 200) {
		/* 1 << '1' ==> 2 */
		divisor = 1;
	} else {
		divisor = 0;
	}

	factor = ((targ_freq * 2 * (1 << divisor)) / 25) - 2;
	if ((factor < 14) || (factor > 30)) {
		cg_status = CG_MEM_WRONG;
	}
	rs->syspllctr.f.sdpll_div = factor;
	rs->mckg_div.f.mckg_divl = divisor;
	/* printf("DD: MEM: %dMHz, divisor: %d\n", df->mem_mhz, divisor); */
	return cg_status;
}

SECTION_CG
static u32_t _cg_calc_ocp(s16_t targ_freq,
                          cg_register_set_t *rs) {
	u32_t div_lmt, i;
	u32_t cg_status = 0, conf_div = 0;
	s16_t pll_out, pll_out_max;
	s16_t diff_targ_conf = 0x7fff, conf_pll_out = 400, curr_freq = 400;

	if ((_soc.cid == 0x6422)||(_soc.cid == 0x6405)) {
		div_lmt = 9;
		pll_out_max = 525;
	} else {
		div_lmt = 7;
		pll_out_max = 600;
	}

	for (pll_out = 275; pll_out <= 800; pll_out += 25) {
		for (i = 0; i <= div_lmt; i++) {
			curr_freq = pll_out / (1 << i);
			if (curr_freq <= targ_freq) {
				if ((targ_freq - curr_freq) < diff_targ_conf) {
					diff_targ_conf = (targ_freq - curr_freq);
					conf_pll_out = pll_out;
					conf_div = i;
				}
			}
		}
	}

	rs->syspllctr.f.ocp0_pll_div = (conf_pll_out / 25) - 2;
	if ((rs->syspllctr.f.ocp0_pll_div < 14) ||
	    (rs->syspllctr.f.ocp0_pll_div > ((pll_out_max/25) - 2))) {
		cg_status = CG_OCP_WRONG;
	}

	rs->pll2.v = PLL2dv;
	if ((_soc.cid == 0x6422)||(_soc.cid == 0x6405)) {
		if (conf_div > 2) {
			rs->pll2.f.en_ocp_div = 2;
		} else {
			rs->pll2.f.en_ocp_div = conf_div;
		}
		conf_div -= rs->pll2.f.en_ocp_div;
	} else if (_soc.cid == 0x0639) {
		rs->pll2.f.en_ocp_div = 2;
	}

	rs->cmugcr.f.oc0_freq_div = conf_div;
	if (conf_div != 0) {
		rs->cmugcr.f.cmu_mode = 1;	/* set to CMU fixed mode. */
	}else {
		rs->cmugcr.f.cmu_mode = 0;	/* set to CMU disabled mode. */
	}

	/* printf("DD: OCP: targ: %dMHz, pll_out: %d, plldiv: %d, cmudiv: %d\n", */
	/*        targ_freq, */
	/*        conf_pll_out, */
	/*        _soc.cid == 0x6422? (1 << rs->pll2.f.en_ocp_div): 1, */
	/*        1 << conf_div); */
	return cg_status;
}

SECTION_CG
void cg_xlat_n_assign(void) {
	cg_dev_freq_t *df     = &cg_info_query.dev_freq;
	cg_register_set_t *rs = &cg_info_query.register_set;

	cg_status = 0;

	cg_status |= _cg_calc_ocp(df->ocp_mhz, rs);
	cg_status |= _cg_calc_mem(df->mem_mhz, rs);
//	cg_status |= _cg_calc_lx(df->lx_mhz, rs);
	return;
}

SECTION_CG
static u32_t cg_udelay(u32_t us, u32_t mhz) {
	u32_t loop_cnt = us*mhz/2;
	while (loop_cnt--) {
		;
	}
	return loop_cnt;
}

SECTION_CG
static void reg_to_mhz(void) {
	cg_info_query.dev_freq.ocp_mhz = (RFLD_SYSPLLCTR(ocp0_pll_div) + 2) * 25;
	cg_info_query.dev_freq.ocp_mhz /= (1 << RFLD_CMUGCR(oc0_freq_div));
	if ((_soc.cid == 0x6422)||(_soc.cid == 0x6405)) {
		cg_info_query.dev_freq.ocp_mhz /= (1 << RFLD_PLL2(en_ocp_div));
	}

	cg_info_query.dev_freq.mem_mhz = ((RFLD_SYSPLLCTR(sdpll_div) + 2) * 25) >> 1;
	cg_info_query.dev_freq.mem_mhz >>= RFLD_MCKG_DIV(mckg_divl);
	cg_info_query.dev_freq.lx_mhz  = 1000 / (RFLD_LX_CLK_PLL(lxpll_div) + 2);
    cg_info_query.dev_freq.spif_mhz = cg_info_query.dev_freq.lx_mhz / ((get_spi_ctrl_divisor() + 1)*2);
}

SECTION_CG
void cg_init(void) {
	cg_dev_freq_t *df     = &cg_info_query.dev_freq;
	cg_register_set_t *rs = &cg_info_query.register_set;

	/* 1: Enable DRAM clock de-glitch. */
	RMOD_DRAM_CLK_CHG(dram_clk_dg_en, 1);
	cg_udelay(1, 400);

	/* 2: Switch OCP to LX clock. */
	RMOD_SYS_STATUS(cf_ckse_ocp0, 0);
	cg_udelay(5, 200);

	/* 3: Change OCP and MEM divisors. */
	PLL2rv = rs->pll2.v;
	SYSPLLCTRrv = rs->syspllctr.v;
	MCKG_DIVrv = rs->mckg_div.v;
	cg_udelay(5, 200);

	/* 4. Switch OCP to original clock. */
	RMOD_SYS_STATUS(cf_ckse_ocp0, 1);
	cg_udelay(5, 200);

	/* 5. Disable DRAM clock de-glitch. */
	RMOD_DRAM_CLK_CHG(dram_clk_dg_en, 0);

#if 0
	/* 6. Switch LX to 100MHz (RTL86865S) or 1/8 OCP clock. (RTL9601B) */
	RMOD_SYS_STATUS(cf_cksel_lx, 0);
	cg_udelay(5, df->ocp_mhz);

	/* 7. Change LX and SPIF (PLL) divisors.*/
	LX_CLK_PLLrv = rs->lx_clk_pll.v;
	cg_udelay(5, df->ocp_mhz);

	/* 8. Switch LX to original clock and done. */
	RMOD_SYS_STATUS(cf_cksel_lx, 1);
#endif


	/* 10. Check for LX/OCP to DRAM slow bits. */
	rs->cmugcr.f.lx0_se_dram = ((s32_t)df->lx_mhz - (s32_t)df->mem_mhz - 1) >> 31;
	rs->cmugcr.f.lx1_se_dram = rs->cmugcr.f.lx0_se_dram;
	rs->cmugcr.f.lx2_se_dram = rs->cmugcr.f.lx0_se_dram;

	rs->cmugcr.f.oc0_se_dram = ((s32_t)df->ocp_mhz - (s32_t)df->mem_mhz - 1) >> 31;
	rs->cmuo0cr.f.oc0_se_dram_wk  = rs->cmugcr.f.oc0_se_dram;
	rs->cmuo0cr.f.oc0_se_dram_slp = rs->cmugcr.f.oc0_se_dram;

	rs->cmuo0cr.f.oc0_auto_bz = 1;
	rs->cmuo0cr.f.oc0_spif_hs = 1;
	rs->cmuo0cr.f.oc0_dram_hs = 1;

	CMUO0CRrv = rs->cmuo0cr.v;
	CMUGCRrv = rs->cmugcr.v;

	uart_init(uart_baud_rate, df->lx_mhz);

	return;
}

SECTION_CG
static void _cg_status_decode(void) {
	const s8_t *bus_name[] = {"OCP", "MEM", "LX"};
	const u32_t bus_mask[] = {CG_OCP_WRONG, CG_MEM_WRONG, CG_LX_WRONG};
	u32_t i;

	for (i=0; i<(sizeof(bus_mask)/sizeof(bus_mask[0])); i++) {
		if (cg_status & bus_mask[i]) {
			printf("WW: %s PLL works beyond spec.\n", bus_name[i]);
		}
	}
	return;
}

SECTION_CG
void cg_result_decode(void) {
	reg_to_mhz();

	_cg_status_decode();

	printf("II: OCP %dMHz(%d/%d/%d), MEM %dMHz(%d/(2*%d)), LX %dMHz, sclk %dMHz(%d/%d)\n",
	       /* OCP */
	       cg_info_query.dev_freq.ocp_mhz,
	       (RFLD_SYSPLLCTR(ocp0_pll_div) + 2) * 25, (1 << RFLD_PLL2(en_ocp_div)), 1 << RFLD_CMUGCR(oc0_freq_div),
	       /* MEM */
	       cg_info_query.dev_freq.mem_mhz,
	       (RFLD_SYSPLLCTR(sdpll_div) + 2) * 25,
	       1 << RFLD_MCKG_DIV(mckg_divl),
	       /* LX */
	       cg_info_query.dev_freq.lx_mhz,
	       /* SPI NAND */
	       cg_info_query.dev_freq.spif_mhz, cg_info_query.dev_freq.lx_mhz, ((get_spi_ctrl_divisor() + 1)*2));
	return;
}

REG_INIT_FUNC(cg_copy_info_to_sram, 15);
REG_INIT_FUNC(cg_xlat_n_assign, 17);
REG_INIT_FUNC(cg_init, 19);
REG_INIT_FUNC(cg_result_decode, 20);
#endif
