#ifndef __SYS_CLK_GEN2_H__
#define __SYS_CLK_GEN2_H__

typedef union {
	struct {
		unsigned int cf_ocp_cpu2bus_clear:1; //0
		unsigned int cf_oc2_access_spif:1; //0
		unsigned int core1_voltprobe_gnd_ctrl:1; //0
		unsigned int core1_voltprobe_pwr_ctrl:1; //0
		unsigned int core0_voltprobe_gnd_ctrl:1; //0
		unsigned int core0_voltprobe_pwr_ctrl:1; //0
		unsigned int reserved_25_13:13; //0
		unsigned int disable_jtag_mipsia:1; //0
		unsigned int disable_jtag_5281:1; //0
		unsigned int cf_gnt_dly:3; //0
		unsigned int parallel_tdi:1; //0
		unsigned int one_ejtag_sel:1; //0
		unsigned int sys_cpu2_en:1; //0
		unsigned int cf_cksel_lx:1; //0
		unsigned int cf_ckse_ocp1:1; //0
		unsigned int cf_ckse_ocp0:1; //0
		unsigned int rdy_for_pathch:1; //0
		unsigned int soc_init_rdy:1; //0
	} f;
	unsigned int v;
} SYS_STATUS_GEN2_T;
#define SYS_STATUS_GEN2rv (*((regval)0xb8000044))
#define RMOD_SYS_STATUS_GEN2(...) rset(SYS_STATUS_GEN2, SYS_STATUS_GEN2rv, __VA_ARGS__)
#define RIZS_SYS_STATUS_GEN2(...) rset(SYS_STATUS_GEN2, 0, __VA_ARGS__)
#define RFLD_SYS_STATUS_GEN2(fld) (*((const volatile SYS_STATUS_GEN2_T *)0xb8000044)).f.fld

typedef union {
	struct {
		unsigned int cksel_sram_500:1;
		unsigned int reserved_30_22:9;
		unsigned int ck_cpu_freq_sel0:6;
		unsigned int reserved_15_14:2;
		unsigned int ck_cpu_freq_sel1:6;
		unsigned int tbc_7_5:3;
		unsigned int sdpll_div:5;
	} f;
	unsigned int v;
} SYS_OCP_PLL_CTRL0_GEN2_T;
#define SYS_OCP_PLL_CTRL0_GEN2rv (*((regval)0xb8000200))
#define RMOD_SYS_OCP_PLL_CTRL0_GEN2(...) rset(SYS_OCP_PLL_CTRL0_GEN2, SYS_OCP_PLL_CTRL0_GEN2rv, __VA_ARGS__)
#define RFLD_SYS_OCP_PLL_CTRL0_GEN2(fld) (*((const volatile SYS_OCP_PLL_CTRL0_GEN2_T *)0xb8000200)).f.fld

typedef union {
	struct {
		unsigned int pllddr_en:1;
		unsigned int pll_ddr_rstb_in:1;
		unsigned int ssc_test_mode:2;
		unsigned int ssc_offset:8;
		unsigned int ssc_step:6;
		unsigned int ssc_period:7;
        unsigned int ssc_en:1;
        unsigned int pllocp1_en:1;
        unsigned int reg_sel_500M:1;
        unsigned int frac_en:1;
        unsigned int pllddr_fypdn:1;
        unsigned int pllocp0_en:1;
        unsigned int pll500M_en:1;
	} f;
	unsigned int v;
} SYS_OCP_PLL_CTRL1_GEN2_T;
#define SYS_OCP_PLL_CTRL1_GEN2rv (*((regval)0xb8000204))
#define RMOD_SYS_OCP_PLL_CTRL1_GEN2(...) rset(SYS_OCP_PLL_CTRL1_GEN2, SYS_OCP_PLL_CTRL1_GEN2rv, __VA_ARGS__)
#define RFLD_SYS_OCP_PLL_CTRL1_GEN2(fld) (*((const volatile SYS_OCP_PLL_CTRL1_GEN2_T *)0xb8000204)).f.fld


typedef union {
	struct {
		unsigned int reg_rs_cpu1:3;
		unsigned int reg_rs_cpu0:3;
		unsigned int reg_mbias1:1;
		unsigned int reg_mbias0:1;
		unsigned int reg_mcco1:2;
		unsigned int reg_mcco0:2;
		unsigned int reg_ldo_sel1:2;
		unsigned int reg_ldo_sel0:2;
		unsigned int reg_kvco_cpu1:1;
		unsigned int reg_kvco_cpu0:1;
		unsigned int reg_icp_double_cpu1:1;
		unsigned int reg_icp_double_cpu0:1;
		unsigned int reg_cp_bias_fix:3;
		unsigned int reg_dogenb_fix:1;
		unsigned int reg_dogenb_cpu1: 1;
		unsigned int reg_dogenb_cpu0: 1;
		unsigned int reg_cp_bias_cpu1:3;
		unsigned int reg_cp_bias_cpu0:3;
	} f;
	unsigned int v;
} SYS_OCP_PLL_CTRL2_GEN2_T;
#define SYS_OCP_PLL_CTRL2_GEN2rv (*((regval)0xb8000208))
#define RMOD_SYS_OCP_PLL_CTRL2_GEN2(...) rset(SYS_OCP_PLL_CTRL2_GEN2, SYS_OCP_PLL_CTRL2_GEN2rv, __VA_ARGS__)
#define RFLD_SYS_OCP_PLL_CTRL2_GEN2(fld) (*((const volatile SYS_OCP_PLL_CTRL2_GEN2_T *)0xb8000208)).f.fld

typedef union {
	struct {
		unsigned int tbc_31_23:9;
		unsigned int reg_cml_r:2;
		unsigned int tbc_20_19:2;
		unsigned int reg_en_DIV2_cpu0:1;
		unsigned int tbc_17_0:18;
	} f;
	unsigned int v;
} SYS_OCP_PLL_CTRL3_GEN2_T;
#define SYS_OCP_PLL_CTRL3_GEN2rv (*((regval)0xb800020C))
#define RMOD_SYS_OCP_PLL_CTRL3_GEN2(...) rset(SYS_OCP_PLL_CTRL3_GEN2, SYS_OCP_PLL_CTRL3rv, __VA_ARGS__)
#define RFLD_SYS_OCP_PLL_CTRL3_GEN2(fld) (*((const volatile SYS_OCP_PLL_CTRL3_GEN2_T *)0xb800020C)).f.fld

#endif  //__OCP_PLL_H__
