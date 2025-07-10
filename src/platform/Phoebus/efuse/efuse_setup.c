#include <soc.h>
#include <efuse/efuse_setup.h>

unsigned short efuse_read(unsigned char entry)
{
	if ( _soc.sid == PLR_SID_APRO) {
		return apro_efuse_read_entry(entry);
	} else if ( _soc.sid == PLR_SID_APRO_GEN2) {
        return apro_otp_read_entry(entry);
	} else {
        return rtl9603cvd_efuse_read_entry(entry);
	}
}

__attribute__ ((unused))
static void efuse_setup_fcn(void) {
	if ( _soc.sid == PLR_SID_APRO) {
		apro_efuse_setup_fcn();
	} else if ( _soc.sid == PLR_SID_APRO_GEN2) {
        apro_otp_setup_fcn();
	} else {
        rtl9603cvd_efuse_setup_fcn();
	}

	return;
}
#if defined(CONFIG_UNDER_UBOOT) && (CONFIG_UNDER_UBOOT == 1)
#else
REG_INIT_FUNC(efuse_setup_fcn, 11);
#endif
