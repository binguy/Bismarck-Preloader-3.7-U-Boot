#include <soc.h>
#include <reg_skc35.h>

#if defined(CONFIG_UNDER_UBOOT) && (CONFIG_UNDER_UBOOT == 1)
extern int otto_is_apro(void);
extern void puts(const char *);
#else
#	include <misc/misc_setting.h>
#endif

#ifndef OTTO_FT
#	define OTTO_FT (0)
#endif

extern void apro_efuse_setup_fcn(void);

typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

SKC35_REG_DEC(
	GPHY_IND_WD, 0xbb000000,
	RF_RSV(31, 16);
	RFIELD(15, 0, wr_dat);
	);

SKC35_REG_DEC(
	GPHY_IND_CMD, 0xbb000004,
	RF_RSV(31, 23);
	RFIELD(22, 22, wren);
	RFIELD(21, 21, cmd_en);
	RFIELD(20, 16, phyid);
	RFIELD(15, 0, ocpaddr);
	);

SKC35_REG_DEC(
	GPHY_IND_RD, 0xbb000008,
	RF_RSV(31, 17);
	RFIELD(16, 16, busy);
	RFIELD(15, 0, rd_dat);
	);

SKC35_REG_DEC(
	EFUSE_IND_WD, 0xbb00001c,
	RF_RSV(31, 16);
	RFIELD(15, 0, wr_dat);
	);

SKC35_REG_DEC(
	EFUSE_IND_CMD, 0xbb000020,
	RF_RSV(31, 18);
	RFIELD(17, 17, wren);
	RFIELD(16, 16, cmd_en);
	RFIELD(15, 0, adr);
	);

SKC35_REG_DEC(
	EFUSE_IND_RD, 0xbb000024,
	RF_RSV(31, 17);
	RFIELD(16, 16, busy);
	RFIELD(15, 0, rd_dat);
	);

SKC35_REG_DEC(
	SCLDO_CTRL, 0xbb000190,
	RF_DNC(31, 13);
	RFIELD(12, 11, sc_vout_adj_12_11);
	RFIELD(10, 7, sc_vout_adj_10_7);
	RFIELD(6, 6, en_sc_ldo);
	RF_DNC(5, 0);
	);

SKC35_REG_DEC(
	STRAP_STS, 0xbb000224,
	RF_DNC(31, 4);
	RFIELD(3, 3, strap_ddr3_mode);
	RFIELD(2, 2, strap_ddr2_mode);
	RF_DNC(1, 0);
	);

SKC35_REG_DEC(
	MODEL_NAME_INFO, 0xbb010000,
	RFIELD(31, 16, rtl_id);
	RFIELD(15, 11, model_char_1st);
	RFIELD(10, 6, model_char_2nd);
	RF_RSV(5, 5);
	RFIELD(4, 4, test_cut);
	RFIELD(3, 0, rtl_vid);
	);

typedef union {
	struct {
		RFIELD16(15, 15, en_ddrldo18_cal);
		RFIELD16(14, 14, en_ddrldo15_cal);
		RFIELD16(13, 13, disable_500m);
		RF_RSV16(12, 12);
		RFIELD16(11, 11, en_p3adco_cal);
		RFIELD16(10, 10, en_p2_r_cal);
		RFIELD16(9, 9, en_p1_r_cal);
		RFIELD16(8, 8, en_p0_r_cal);
		RF_DNC16(7, 7);
		RFIELD16(6, 6, en_phy3_cal);
		RFIELD16(5, 5, en_phy2_amp_cal);
		RFIELD16(4, 4, en_phy1_amp_cal);
		RFIELD16(3, 3, en_phy0_amp_cal);
		RF_DNC16(2, 0);
	};
	uint16_t v;
} EA6_T;

typedef union {
	struct {
		RF_RSV16(15, 14);
		RFIELD16(13, 10, ddrldo18);
		RF_RSV16(9, 9);
		RFIELD16(8, 5, ddrldo15);
		RF_RSV16(4, 0);
	};
	uint16_t v;
} EA8_T;

typedef union {
	struct {
		RFIELD16(15, 12, val12);
		RFIELD16(11, 8, val8);
		RFIELD16(7, 4, val4);
		RFIELD16(3, 0, val0);
	};
	uint16_t v;
} EA10_T;

static int _efuse_is_fpga(void) {
	return !!(cg_query_freq(CG_DEV_LX) < 125);
}

static uint32_t saturating_add(
	int v0, const int v1,
	const int sat_ceiling, const int sat_floor) {
	v0 += v1;
	if (v0 > sat_ceiling) {
		v0 = sat_ceiling;
	} else if (v0 < sat_floor) {
		v0 = sat_floor;
	}
	return v0;
}

static void poll_ind_rd_busy(const uint32_t offset, const char *err_msg) {
	/* Spec. mentions timeout in 500ms; 500ms ~= 500*1000*1000 cycles in 1GHz CPU;
	   say each iter. takes 10 cycles, that's the loop count; */
	uint32_t timeout_500ms = 500*1000*1000/10;

	/* Randomly choose GPHY_IND_RD_T as prototype for no reason.
     Both GPHY_IND_RD_T and EFUSE_IND_RD_T has the same busy bit anyway. */
	volatile GPHY_IND_RD_T *dummy_ind_rd = (GPHY_IND_RD_T *)offset;

	while (dummy_ind_rd->f.busy) {
		if ((timeout_500ms--) == 0) {
			puts(err_msg);
			break;
		}
	}

	return;
}

static uint16_t read_efuse(const uint32_t id) {
	RMOD(
		EFUSE_IND_CMD,
		wren, 0,
		cmd_en, 1,
		adr, id);

	poll_ind_rd_busy(EFUSE_IND_RDar, "EE: read_efuse() timeout\n");

	return RFLD(EFUSE_IND_RD, rd_dat);
}

static void write_gphy(const uint8_t id, const uint16_t addr, const uint16_t data) {
	RMOD(
		GPHY_IND_WD,
		wr_dat, data);

	RMOD(
		GPHY_IND_CMD,
		wren, 1,
		cmd_en, 1,
		phyid, id,
		ocpaddr, addr);

	poll_ind_rd_busy(GPHY_IND_RDar, "EE: write_gphy() timeout\n");

	return;
}

static uint16_t read_gphy(uint8_t id, uint16_t addr) {
	RMOD(
		GPHY_IND_CMD,
		wren, 0,
		cmd_en, 1,
		phyid, id,
		ocpaddr, addr);

	poll_ind_rd_busy(GPHY_IND_RDar, "EE: read_gphy() timeout\n");

	return RFLD(GPHY_IND_RD, rd_dat);
}

static uint16_t read_fephy(uint8_t id, uint16_t page, uint16_t regid) {
	RMOD(GPHY_IND_WD, wr_dat, page);

	RMOD(
		GPHY_IND_CMD,
		wren, 1,
		cmd_en, 1,
		phyid, id,
		ocpaddr, 0x3e);

	poll_ind_rd_busy(GPHY_IND_RDar, "EE: write_fephy() timeout\n");

	RMOD(
		GPHY_IND_CMD,
		wren, 0,
		cmd_en, 1,
		phyid, id,
		ocpaddr, regid*2);

	poll_ind_rd_busy(GPHY_IND_RDar, "EE: read_fephy() timeout\n");

	return RFLD(GPHY_IND_RD, rd_dat);
}

static void write_fephy(
	uint8_t id, uint16_t page, uint16_t regid, uint16_t data) {
	RMOD(GPHY_IND_WD, wr_dat, page);
	RMOD(
		GPHY_IND_CMD,
		wren, 1,
		cmd_en, 1,
		phyid, id,
		ocpaddr, 0x3e);

	poll_ind_rd_busy(GPHY_IND_RDar, "EE: write_fephy() timeout\n");

	RMOD(GPHY_IND_WD, wr_dat, data);
	RMOD(
		GPHY_IND_CMD,
		wren, 1,
		cmd_en, 1,
		phyid, id,
		ocpaddr, regid*2);

	poll_ind_rd_busy(GPHY_IND_RDar, "EE: write_fephy() timeout\n");

	return;
}

static uint8_t read_sram(const uint8_t id, const uint16_t addr) {
	write_gphy(id, 0xa436, addr);
	return ((read_gphy(id, 0xa438)) >> 8);
}

static void write_sram(const uint8_t id, const uint16_t addr, const uint8_t data) {
	uint8_t next_addr_val;

	/* gphy write consumes 2 bytes; get the next byte to combine with data for 2 bytes */
	next_addr_val = read_sram(id, addr+1);
	write_gphy(id, 0xa436, addr);
	write_gphy(id, 0xa438, ((data << 8) | next_addr_val));

	return;
}

static void ddrldo18_cal(uint32_t en) {
	EA8_T ea8;
	SCLDO_CTRL_T scldo_ctrl;

	if (!en) {
		scldo_ctrl.v = 0xb96;
	} else {
		ea8.v = read_efuse(8);

		scldo_ctrl.v = RVAL(SCLDO_CTRL);
		scldo_ctrl.v |= (1 << 3);
		scldo_ctrl.f.sc_vout_adj_12_11 = 0x1;
		scldo_ctrl.f.sc_vout_adj_10_7 = ea8.ddrldo18;
	}

	RVAL(SCLDO_CTRL) = scldo_ctrl.v;

	RMOD(
		SCLDO_CTRL,
		en_sc_ldo, 1);

	return;
}

static void ddrldo15_cal(uint32_t en) {
	EA8_T ea8;
	SCLDO_CTRL_T scldo_ctrl;

	if (!en) {
		scldo_ctrl.v = 0x396;
	} else {
		ea8.v = read_efuse(8);

		scldo_ctrl.v = RVAL(SCLDO_CTRL);
		scldo_ctrl.v |= (1 << 3);
		scldo_ctrl.f.sc_vout_adj_12_11 = 0x0;
		scldo_ctrl.f.sc_vout_adj_10_7 = ea8.ddrldo15;
	}

	RVAL(SCLDO_CTRL) = scldo_ctrl.v;

	RMOD(
		SCLDO_CTRL,
		en_sc_ldo, 1);

	return;
}

static void phy3_cal(void) {
	uint16_t efuse_val;

	write_sram(3, 0x837d, 0x46);

	efuse_val = read_efuse(10);
	write_gphy(3, 0xbcdc, efuse_val);
	write_sram(3, 0x837e, efuse_val >> 8);
	write_sram(3, 0x837f, efuse_val & 0xff);

	efuse_val = read_efuse(11);
	write_gphy(3, 0xbcde, efuse_val);
	write_sram(3, 0x8380, efuse_val >> 8);
	write_sram(3, 0x8382, efuse_val >> 8);
	write_sram(3, 0x8384, efuse_val >> 8);
	write_sram(3, 0x8381, efuse_val & 0xff);
	write_sram(3, 0x8383, efuse_val & 0xff);
	write_sram(3, 0x8385, efuse_val & 0xff);

	efuse_val = read_efuse(9);
	write_gphy(3, 0xbce0, efuse_val);
	write_gphy(3, 0xbce2, efuse_val);

	efuse_val = read_efuse(12);
	write_gphy(3, 0xbcac, efuse_val);

	return;
}

static void p3adco_cal(void) {
	const uint16_t efuse_val = read_efuse(13);

	write_gphy(3, 0xbcfc, efuse_val);
	return;
}

static void disable_500m(void) {
	uint16_t p3_a4a2 = read_gphy(3, 0xa4a2);

	/* set bit 8 to 0 to disable */
	p3_a4a2 &= (~(1<<8));

	write_gphy(3, 0xa4a2, p3_a4a2);
	return;
}

static void pxadco_cal(uint16_t id) {
	uint16_t reg29, reg28;
	short r_code, offset;
	uint16_t efuse_val;
	struct {signed int x:4;} to_sign_4bit;

	/* write 0 to reg29[2] */
	reg29 = read_fephy(id, 1, 29);
	reg29 = reg29 & (~(1 << 2));
	write_fephy(id, 1, 29, reg29);

	/* get r_code */
	reg28 = read_fephy(id, 0x1, 28);
	r_code = (reg28 >> 12) & 0xf;

	/* get offset with sign extension */
	efuse_val = read_efuse(id + 14);
	to_sign_4bit.x = (efuse_val >> 4) & 0xf;
	offset = to_sign_4bit.x;

	/* apply offset to r_code */
	r_code = saturating_add(r_code, offset, 0xf, 0x0);

	/* write 1 to reg29[2] */
	reg29 = reg29 | (1<<2);
	write_fephy(id, 1, 29, reg29);

	/* update r_code */
	reg28 = reg28 & (~(0xf << 12));
	reg28 = reg28 | (r_code << 12);
	write_fephy(id, 1, 28, reg28);

	return;
}

static void patch_abiq_iol(void) {
	int id;
	uint16_t reg16;

	for (id=0; id<3; id++) {
		reg16 = read_fephy(id, 1, 16);
		reg16 |= (0x7 << 13);
		write_fephy(id, 1, 16, reg16);
	}

	return;
}

static void patch_cf_iol(void) {
	int id;
	uint16_t reg17;

	for (id=0; id<3; id++) {
		reg17 = read_fephy(id, 1, 17);
		reg17 &= (~(7 << 11));
		reg17 |= 4 << 11;
		write_fephy(id, 1, 17, reg17);
	}

	return;
}

static void phyx_amp_cal(int id, int en) {
	uint16_t reg18;
	uint16_t bit7_4;

	reg18 = read_fephy(id, 1, 18);
	reg18 &= (~0x00f0);

	if (en) {
		bit7_4 = read_efuse(14 + id) & 0xf;
	} else {
		bit7_4 = 8 - id;
	}

	reg18 |= (bit7_4 << 4);
	write_fephy(id, 1, 18, reg18);

	return;
}

static void efuse_procedure_b(void) {
	EA6_T ea6;

	ea6.v = read_efuse(6);

	if (RFLD(STRAP_STS, strap_ddr2_mode) == 1) {
		ddrldo18_cal(ea6.en_ddrldo18_cal | OTTO_FT);
	} else if (RFLD(STRAP_STS, strap_ddr3_mode) == 1) {
		ddrldo15_cal(ea6.en_ddrldo15_cal | OTTO_FT);
	} else {
		puts("EE: unknown DRAM mode for LDO\n");
	}

	return;
}

void efuse_procedure_c(void) {
	EA6_T ea6;

	ea6.v = read_efuse(6);

	if (ea6.en_phy3_cal == 1) {
		phy3_cal();
	} else {
		write_sram(3, 0x837d, 0x46);
		write_sram(3, 0x837e, 0x88);
		write_sram(3, 0x837f, 0x88);
		write_sram(3, 0x8380, 0x88);
		write_sram(3, 0x8381, 0x88);
		write_sram(3, 0x8382, 0x88);
		write_sram(3, 0x8383, 0x88);
		write_sram(3, 0x8384, 0x88);
		write_sram(3, 0x8385, 0x88);
	}

	if (ea6.en_p3adco_cal == 1) {
		p3adco_cal();
	}

	if (ea6.disable_500m == 1) {
		disable_500m();
	}

	if (ea6.en_p2_r_cal == 1) {
		pxadco_cal(2);
	}
	if (ea6.en_p1_r_cal == 1) {
		pxadco_cal(1);
	}
	if (ea6.en_p0_r_cal == 1) {
		pxadco_cal(0);
	}

	patch_abiq_iol();

	patch_cf_iol();

	phyx_amp_cal(2, ea6.en_phy2_amp_cal);
	phyx_amp_cal(1, ea6.en_phy1_amp_cal);
	phyx_amp_cal(0, ea6.en_phy0_amp_cal);

	return;
}

__attribute__ ((unused))
static void efuse_setup_fcn(void) {
	if (_efuse_is_fpga()) {
		return;
	}

	if (otto_is_apro()) {
		apro_efuse_setup_fcn();
		return;
	}

	efuse_procedure_b();

	efuse_procedure_c();

	return;
}
#if defined(CONFIG_UNDER_UBOOT) && (CONFIG_UNDER_UBOOT == 1)
#else
REG_INIT_FUNC(efuse_setup_fcn, 11);
#endif
