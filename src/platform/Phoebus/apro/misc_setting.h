#ifndef _APRO_MISC_SETTING_H_
#define _APRO_MISC_SETTING_H_

#include <misc/mem_size_def.h>

typedef enum {
    ST_RTL9603CT  = 0,
    ST_RTL9603C   = 1,
    ST_RTL9603CP  = 2,
    ST_RTL9603CW  = 3,
    ST_RTL9603CE  = 4,
    ST_RTL9606C   = 6,
    ST_RTL9607C   = 8,
    ST_RTL9607CP  = 9,
    ST_RTL9607E   = 10,
    ST_RTL9607EP  = 11,
    ST_RTL8198D   = 20,
    ST_RTL8198DE  = 21,
} apro_sub_chip_type_t;

typedef struct{
    apro_sub_chip_type_t st;
    u16_t clk;
    memory_size_mapping_t size;
}apro_series_mode_t;

typedef struct{
    apro_sub_chip_type_t st;
    u16_t cclk;
	u16_t mclk;
    memory_size_mapping_t size;
}apro_series_mode_gen2_t;

extern void apro_chip_pre_init(void);
extern u32_t apro_xlat_cpu_freq;
extern u32_t apro_xlat_ddr3_freq;
extern u32_t apro_xlat_dram_size_num;
extern u32_t xlat_chip_mode(void);

#endif
