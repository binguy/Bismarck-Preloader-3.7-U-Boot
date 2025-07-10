#ifndef _MEMCNTLR_REG_H_
#define _MEMCNTLR_REG_H_

typedef union {
	struct {
		unsigned int dram_type:4; //1
		unsigned int boot_sel:4; //0
		unsigned int ip_ref:1; //0
		unsigned int dp_ref:1; //0
		unsigned int eeprom_type:1; //1
		unsigned int d_signal:1; //0
		unsigned int flash_map0_dis:1; //0
		unsigned int flash_map1_dis:1; //1
		unsigned int mbz_0:3; //0
		unsigned int d_init_trig:1; //0
		unsigned int ocp1_frq_slower:1; //0
		unsigned int lx1_frq_slower:1; //0
		unsigned int lx2_frq_slower:1; //0
		unsigned int lx3_frq_slower:1; //0
		unsigned int ocp0_frq_slower:1; //0
		unsigned int ocp1_rbf_mask_en:1; //0
		unsigned int ocp0_rbf_mask_en:1; //0
		unsigned int ocp1_rbf_f_dis:1; //1
		unsigned int ocp0_rbf_f_dis:1; //1
		unsigned int sync_ocp1_dram:1; //0
		unsigned int sync_lx0_dram:1; //0
		unsigned int sync_lx1_dram:1; //0
		unsigned int sync_lx2_dram:1; //0
		unsigned int sync_ocp0_dram:1; //0
	} f;
	unsigned int v;
} MCR_T;
#define MCRrv (*((regval)0xb8001000))
#define MCRdv (0x10200060)
#define RMOD_MCR(...) rset(MCR, MCRrv, __VA_ARGS__)
#define RIZS_MCR(...) rset(MCR, 0, __VA_ARGS__)
#define RFLD_MCR(fld) (*((const volatile MCR_T *)0xb8001000)).f.fld

typedef union {
	struct {
		unsigned int no_use31:2; //0
		unsigned int bankcnt:2; //1
		unsigned int no_use27:2; //0
		unsigned int dbuswid:2; //1
		unsigned int rowcnt:4; //0
		unsigned int colcnt:4; //0
		unsigned int dchipsel:1; //1
		unsigned int fast_rx:1; //0
		unsigned int bstref:1; //0
		unsigned int prl_bank_act_en:1; //0
		unsigned int no_use10:10; //0
		unsigned int eco_rd_buf_mech_5281:1; //0
		unsigned int eco_rd_buf_mech_iA:1; //0
	} f;
	unsigned int v;
} DCR_T;
#define DCRrv (*((regval)0xb8001004))
#define RMOD_DCR(...) rset(DCR, DCRrv, __VA_ARGS__)
#define RFLD_DCR(fld) (*((const volatile DCR_T *)0xb8001004)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:24;
		unsigned int sync_pbo_uw_dram:1; //0
		unsigned int sync_pbo_ur_dram:1; //0
		unsigned int sync_pbo_dw_dram:1; //0
		unsigned int sync_pbo_dr_dram:1; //0
		unsigned int pbo_uw_frq_slow:1; //0
		unsigned int pbo_ur_frq_slow:1; //0
		unsigned int pbo_dw_frq_slow:1; //0
		unsigned int pbo_dr_frq_slow:1; //0
	} f;
	unsigned int v;
} PBOLSRR_T;
#define PBOLSRRrv (*((regval)0xb8001014))
#define RMOD_PBOLSRR(...) rset(PBOLSRR, PBOLSRRrv, __VA_ARGS__)

typedef union {
	struct {
		unsigned int dtr_up_busy_mrs_busy:1; //0
		unsigned int mbz_0:5; //0
		unsigned int en_wr_leveling:1; //0
		unsigned int dis_dram_ref:1; //0
		unsigned int no_use23:3; //0
		unsigned int mr_mode_en:1; //0
		unsigned int no_use19:2; //0
		unsigned int mr_mode:2; //0
		unsigned int no_use15:2; //0
		unsigned int mr_data:14; //0
	} f;
	unsigned int v;
} DMCR_T;
#define DMCRrv (*((regval)0xb800101c))
#define RMOD_DMCR(...) rset(DMCR, DMCRrv, __VA_ARGS__)
#define RFLD_DMCR(fld) (*((const volatile DMCR_T *)0xb800101c)).f.fld

typedef union {
	struct {
		unsigned int ________________________mbz_1:13;
		unsigned int bond_rsvd1:1;
		unsigned int ________________________mbz_0:18;
	} f;
	unsigned int v;
} BOND_STS_T;
#define BOND_STSrv (*((regval)0xbb000220))
#define RMOD_BOND_STS(...) rset(BOND_STS, BOND_STSrv, __VA_ARGS__)
#define RFLD_BOND_STS(fld) (*((const volatile BOND_STS_T *)0xbb000220)).f.fld

__attribute__((unused)) static int _is_mcm(void) {
	return RFLD_BOND_STS(bond_rsvd1);
}

__attribute__((unused)) static int _is_ddr3(void) {
	/* dram_type: 1: ddr2; 2: ddr3 */
	return RFLD_MCR(dram_type) - 1;
}

#endif  // _MEMCNTLR_REG_H_
