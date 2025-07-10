#include <soc.h>
#include <cg/cg.h>

cg_dev_freq_t cg_info_query;


#ifndef SECTION_CG_CORE_INIT
    #define SECTION_CG_CORE_INIT
#endif

#ifndef SECTION_CG_MISC
    #define SECTION_CG_MISC
#endif

#ifndef SECTION_CG_MISC_DATA
    #define SECTION_CG_MISC_DATA
#endif


/********************************
	CG main flow
********************************/
SECTION_CG_MISC
u32_t cg_query_freq(u32_t dev_type) {
	return CG_QUERY_FREQUENCY(dev_type, (&cg_info_query));
}

void copy_proj_info_to_sram(void) {
    if ( _soc.sid == PLR_SID_APRO) {
        apro_copy_proj_info_to_sram();
    } else if ( _soc.sid == PLR_SID_APRO_GEN2) {
        apro_gen2_copy_proj_info_to_sram();
    } else {
        rtl9603cvd_cg_get_parameters();
    }
    return;
}

void cg_init(void) {
    if ((_soc.sid == PLR_SID_APRO) || ( _soc.sid == PLR_SID_APRO_GEN2)) {
        apro_cg_init();
    } else {
        rtl9603cvd_cg_init();
    }
    return;
}


REG_INIT_FUNC(copy_proj_info_to_sram, 10);
REG_INIT_FUNC(cg_init, 11);

