#ifndef __RTL9603CVD_CG_HEADER__
#define __RTL9603CVD_CG_HEADER__

#include <register_map.h>
#include "ocp_pll.h"
#include "mem_pll.h"
#include "cmu.h"

/*-----------------------------------------------------
	from System Clock control
	-----------------------------------------------------*/
typedef union {
	struct {
		RFIELD(31, 31, cf_ocp_cpu2bus_clear);
		RFIELD(30, 30, cf_oc2_access_spif);
		RFIELD(29, 29, core1_voltprobe_gnd_ctrl);
		RFIELD(28, 28, core1_voltprobe_pwr_ctrl);
		RFIELD(27, 27, core0_voltprobe_gnd_ctrl);
		RFIELD(26, 26, core0_voltprobe_pwr_ctrl);
		RFIELD(25, 22, ________________________reserved_25_22);
		RFIELD(21, 21, cf_reg_brg_sync_acked);
		RFIELD(20, 20, cf_disable_iaup_clk_tune);
		RFIELD(19, 19, en_sleep_delay);
		RFIELD(18, 18, rt_power_ctrl_out);
		RFIELD(17, 17, cg_rt_iso);
		RFIELD(16, 16, cf_rt_power_ctrl_in);
		RFIELD(15, 15, en_sleep_half);
		RFIELD(14, 14, cg_ocp_alow_iso);
		RFIELD(13, 13, reg_ssc_rstb);
		RFIELD(12, 12, disable_jtag_mipsia);
		RFIELD(11, 11, disable_jtag_5281);
		RFIELD(10, 8, cg_gnt_dly);
		RFIELD(7, 7, parallel);
		unsigned int one_ejtag_sel:1; //0
		unsigned int sys_cpu2_en:1; //0
		unsigned int cf_cksel_lx:1; //1
		unsigned int cf_ckse_ocp1:1; //1
		unsigned int cf_ckse_ocp0:1; //1
		unsigned int rdy_for_pathch:1; //0
		unsigned int soc_init_rdy:1; //0
	} f;
	unsigned int v;
} SYS_STS_T;
#define SYS_STSrv (*((regval)0xb8000044))
#define RMOD_SYS_STS(...) rset(SYS_STS, SYS_STSrv, __VA_ARGS__)
#define RFLD_SYS_STS(fld) (*((const volatile SYS_STS_T *)0xb8000044)).f.fld


SKC35_REG_DEC(
	SCATS, 0xb80040f8,
	RF_RSV(31, 9);
	RFIELD(8, 8, sram_to_lx);
	RFIELD(7, 7, sram_to_oc1);
	RFIELD(6, 6, sram_to_oc0);
	RFIELD(5, 5, oc1_to_rom);
	RFIELD(4, 4, oc0_to_rom);
	RFIELD(3, 2, oc1_to_sram);
	RFIELD(1, 0, oc0_to_sram);
	);

extern const cg_dev_freq_t rtl9603cvd_cg_proj_freq;


extern void rtl9603cvd_cg_get_parameters(void);
extern void rtl9603cvd_cg_init(void);
#endif
