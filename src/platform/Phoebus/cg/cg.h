#ifndef __CG_HEADER__
#define __CG_HEADER__

#include <cg/cg_dev_freq.h>
#include <cg/gphy_pll.h>
#include <apro/cg.h>
#include <9603cvd/cg.h>


#define OTTO_LX_RESET_DEFAULT_MHZ (200)

#define END_OF_INFO (0xFFFF)

typedef struct clk_div_sel_info_s{
    u16_t divisor;
    u16_t div_to_ctrl;
} clk_div_sel_info_t;


typedef struct{
    u16_t mhz;
    u32_t pll0;
    u32_t pll1;
    u32_t pll2;
    u32_t pll3;
    u32_t pll5;
} mem_pll_info_t;

extern cg_dev_freq_t cg_target_freq;
extern cg_dev_freq_t cg_info_query;

extern  u32_t cg_query_freq(u32_t dev_type);

#define GET_CPU0_MHZ()	(cg_query_freq(CG_DEV_CPU0))
#define GET_CPU1_MHZ()	(cg_query_freq(CG_DEV_CPU1))
#define GET_MEM_MHZ()	(cg_query_freq(CG_DEV_MEM))
#define GET_LX_MHZ()	(cg_query_freq(CG_DEV_LX))
#define GET_SPIF_MHZ()	(cg_query_freq(CG_DEV_SPIF))

#endif
