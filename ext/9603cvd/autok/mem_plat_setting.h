#ifndef __MEM_PLAT_SETTING_H__
#define __MEM_PLAT_SETTING_H__ 1

#define MEM_PLL_INFO_SECTION SECTION_RO

typedef unsigned int uint32_t;
typedef unsigned short uint16_t;

extern mem_pll_info_t ddr2_pll[];
extern mem_pll_info_t ddr3_pll[];

extern void memcntlr_plat_preset(void);
extern void memcntlr_plat_postset(void);
#endif
