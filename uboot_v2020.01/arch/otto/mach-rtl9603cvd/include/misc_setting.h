#ifndef _MISC_SETTING_H_
#define _MISC_SETTING_H_

#include <apro/misc_setting.h>

/* 19/03/14, wiki, package_bonding_efuse_info */
typedef enum {
	ST_RTL9603CVD4CG = 1,
	ST_RTL9603CVD5CG = 4,
	ST_RTL9603CVD6CG = 8,
	ST_RTL9603CEVDCG = 9,
} rtl9603cvd_sub_chip_type_t;

__attribute__ ((far)) int otto_is_apro(void);
void rtl9603cvd_stall_setup(void);
u32_t xlat_dram_size_num(void);

#endif
