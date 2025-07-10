#include <util.h>
#include <misc/misc_setting.h>

SECTION_RECYCLE
void rtl9603cvd_stall_setup(void) {
	if (REG32(0xb8000250) != 0x0000002a) {
		REG32(0xb8000250) = 0x0000002a;
		REG32(0xb8003268) = 0x80000001;
		while (1);
	}
	return;
}

SECTION_RO series_mode_t rtl9603cvd_series_tbl[] = {
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

u32_t xlat_dram_size_num(void) {
	if (otto_is_apro()) {
		return apro_xlat_dram_size_num();
	} else {
    _soc.cid = ((_soc.cid&(0xFFFF<<16))|(rtl9603cvd_series_tbl[_soc_cid_sct].st));
		return rtl9603cvd_series_tbl[_soc_cid_sct].size;
	}
}
