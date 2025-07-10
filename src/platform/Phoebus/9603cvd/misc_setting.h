#ifndef _9603CVD_MISC_SETTING_H_
#define _9603CVD_MISC_SETTING_H_

#include <misc/mem_size_def.h>

/* 19/03/14, wiki, package_bonding_efuse_info */
typedef enum {
	ST_RTL9603CVD4CG = 1,
	ST_RTL9603CVD5CG = 4,
	ST_RTL9602CVD5CG = 5,
	ST_RTL9603CVD6CG = 8,
	ST_RTL9603CEVDCG = 16,
	ST_RTL9601DVD3CG = 20,
	ST_RTL9601DVD4CG = 24,
} rtl9603cvd_sub_chip_type_t;

typedef struct{
    rtl9603cvd_sub_chip_type_t st;
    s32_t clk;
    memory_size_mapping_t size;
}rtl9603cvd_series_mode_t;


void rtl9603cvd_chip_pre_init(void);
u32_t rtl9603cvd_xlat_dram_size_num(void);

#endif
