#ifndef __APRO_OCP_PLL_H__
#define __APRO_OCP_PLL_H__

typedef union {
	struct {
		unsigned int no_use31:25; //0
		unsigned int one_ejtag_sel:1; //0
		unsigned int sys_cpu2_en:1; //0
		unsigned int cf_cksel_lx:1; //1
		unsigned int cf_ckse_ocp1:1; //1
		unsigned int cf_ckse_ocp0:1; //1
		unsigned int rdy_for_pathch:1; //0
		unsigned int soc_init_rdy:1; //0
	} f;
	unsigned int v;
} SYS_STATUS_T;
#define SYS_STATUSrv (*((regval)0xb8000044))
#define SYS_STATUSdv (0x0000004C)
#define RMOD_SYS_STATUS(...) rset(SYS_STATUS, SYS_STATUSrv, __VA_ARGS__)
#define RIZS_SYS_STATUS(...) rset(SYS_STATUS, 0, __VA_ARGS__)
#define RFLD_SYS_STATUS(fld) (*((const volatile SYS_STATUS_T *)0xb8000044)).f.fld

typedef union {
	struct {
		unsigned int cksel_sram_500:1;
		unsigned int reserved_30_22:9;
		unsigned int ck_cpu_freq_sel0:6;
		unsigned int reserved_15_14:2;
		unsigned int ck_cpu_freq_sel1:6;
		unsigned int tbc_7_0:8;
	} f;
	unsigned int v;
} SYS_OCP_PLL_CTRL0_T;
#define SYS_OCP_PLL_CTRL0rv (*((regval)0xb8000200))
#define RMOD_SYS_OCP_PLL_CTRL0(...) rset(SYS_OCP_PLL_CTRL0, SYS_OCP_PLL_CTRL0rv, __VA_ARGS__)
#define RFLD_SYS_OCP_PLL_CTRL0(fld) (*((const volatile SYS_OCP_PLL_CTRL0_T *)0xb8000200)).f.fld

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
} SYS_OCP_PLL_CTRL1_T;
#define SYS_OCP_PLL_CTRL1rv (*((regval)0xb8000204))
#define RMOD_SYS_OCP_PLL_CTRL1(...) rset(SYS_OCP_PLL_CTRL1, SYS_OCP_PLL_CTRL1rv, __VA_ARGS__)
#define RFLD_SYS_OCP_PLL_CTRL1(fld) (*((const volatile SYS_OCP_PLL_CTRL1_T *)0xb8000204)).f.fld


typedef union {
	struct {
		unsigned int tbc_31_19:13;
		unsigned int reg_en_DIV2_cpu0:1;
		unsigned int tbc_17_0:18;
	} f;
	unsigned int v;
} SYS_OCP_PLL_CTRL3_T;
#define SYS_OCP_PLL_CTRL3rv (*((regval)0xb800020C))
#define RMOD_SYS_OCP_PLL_CTRL3(...) rset(SYS_OCP_PLL_CTRL3, SYS_OCP_PLL_CTRL3rv, __VA_ARGS__)
#define RFLD_SYS_OCP_PLL_CTRL3(fld) (*((const volatile SYS_OCP_PLL_CTRL3_T *)0xb800020C)).f.fld

#endif  //__OCP_PLL_H__
