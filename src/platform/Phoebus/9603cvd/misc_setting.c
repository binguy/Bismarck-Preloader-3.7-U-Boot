#include <util.h>
#include <9603cvd/swcore_reg.h>
#include <9603cvd/misc_setting.h>

SECTION_RECYCLE
static void rtl9603cvd_stall_setup(void) {
	if (REG32(0xb8000250) != 0x0000002a) {
		REG32(0xb8000250) = 0x0000002a;
		REG32(0xb8003268) = 0x80000001;
		while (1);
	}
	return;
}


SECTION_RECYCLE
void rtl9603cvd_chip_pre_init(void) {
    MODEL_NAME_INFO_9603cvd_T model_info;
    model_info.v = RVAL(MODEL_NAME_INFO_9603cvd);

	if ((model_info.f.rtl_id==0x9603) && (model_info.f.model_char_1st==0x4)) {
		_soc.sid = PLR_SID_9603CVD;
	    rtl9603cvd_stall_setup();
		RMOD(IO_MODE_EN_9603cvd, uart0_en, 1);

	}
	return;
}


SECTION_RO rtl9603cvd_series_mode_t rtl9603cvd_series_tbl[] = {
	{.st = NA,               .clk= NA, .size= SIZE_NA}, //idx_0
	{.st = ST_RTL9603CVD4CG, .clk= NA, .size= SIZE_512Mb},
	{.st = NA,               .clk= NA, .size= SIZE_NA},
	{.st = NA,               .clk= NA, .size= SIZE_NA},
	{.st = ST_RTL9603CVD5CG, .clk= NA, .size= SIZE_1Gb}, //idx 4
	{.st = ST_RTL9602CVD5CG, .clk= NA, .size= SIZE_1Gb},
	{.st = NA,               .clk= NA, .size= SIZE_NA},
	{.st = NA,               .clk= NA, .size= SIZE_NA},
	{.st = ST_RTL9603CVD6CG, .clk= NA, .size= SIZE_2Gb}, //idx 8
	{.st = NA,               .clk= NA, .size= SIZE_NA},
	{.st = NA,               .clk= NA, .size= SIZE_NA},
	{.st = NA,               .clk= NA, .size= SIZE_NA},
	{.st = NA,               .clk= NA, .size= SIZE_NA}, //idx 12
	{.st = NA,               .clk= NA, .size= SIZE_NA},
	{.st = NA,               .clk= NA, .size= SIZE_NA},
	{.st = NA,               .clk= NA, .size= SIZE_NA},
	{.st = ST_RTL9603CEVDCG, .clk= NA, .size= SIZE_NA}, //idx 16
	{.st = NA,               .clk= NA, .size= SIZE_NA},
	{.st = NA,               .clk= NA, .size= SIZE_NA},
	{.st = NA,               .clk= NA, .size= SIZE_NA},
	{.st = ST_RTL9601DVD3CG, .clk= NA, .size= SIZE_256Mb}, //idx 20
	{.st = NA,               .clk= NA, .size= SIZE_NA},
	{.st = NA,               .clk= NA, .size= SIZE_NA},
	{.st = NA,               .clk= NA, .size= SIZE_NA},
	{.st = ST_RTL9601DVD4CG, .clk= NA, .size= SIZE_512Mb}, //idx 24
	{.st = NA,               .clk= NA, .size= SIZE_NA},
	{.st = NA,               .clk= NA, .size= SIZE_NA},
	{.st = NA,               .clk= NA, .size= SIZE_NA},
	{.st = NA,               .clk= NA, .size= SIZE_NA}, //idx 28
	{.st = NA,               .clk= NA, .size= SIZE_NA},
	{.st = NA,               .clk= NA, .size= SIZE_NA},
	{.st = NA,               .clk= NA, .size= SIZE_NA},
};

u32_t rtl9603cvd_xlat_dram_size_num(void) {
    _soc.cid = ((_soc.cid&(0xFFFF<<16))|(rtl9603cvd_series_tbl[_soc_cid_sct].st));
    return rtl9603cvd_series_tbl[_soc_cid_sct].size;
}
