#include <util.h>
#include <misc/misc_setting.h>

u32_t xlat_dram_size_num(void) {
	if ( _soc.sid == PLR_SID_APRO ||  _soc.sid == PLR_SID_APRO_GEN2) {
		return apro_xlat_dram_size_num;
	} else {
		return rtl9603cvd_xlat_dram_size_num();
	}
}

