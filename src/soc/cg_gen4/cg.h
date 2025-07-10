#ifndef __CG_HEADER__
#define __CG_HEADER__

#include <register_map.h>
#include "cg_dev_freq.h"


#define CPU_CLK_IN_DEFAULT (400)

typedef struct {
	SYSPLLCTR_T  syspllctr;  /* for CPU & DRAM factor */
	PLL2_T       pll2;       /* for 6422 PLL CPU divisor */
	MCKG_DIV_T   mckg_div;   /* for DRAM divisor */
	LX_CLK_PLL_T lx_clk_pll; /* for SPI-F (PLL) & LX */
    u32_t        sclk_div2ctrl;
	CMUGCR_T     cmugcr;     /* for CMU static division */
	CMUO0CR_T    cmuo0cr;    /* for OCP0 slow bits */
} cg_register_set_t;

#define END_OF_INFO (0xFFFF)

typedef struct {
    u16_t pll_freq_mhz;
    u16_t pll_cfg_div;
    u16_t cal_sclk_mhz;
    u16_t sclk_div2ctrl;
} cg_sclk_info_t;

typedef struct clk_div_sel_info_s{
    u16_t divisor;
    u16_t div_to_ctrl;
}clk_div_sel_info_t;


typedef struct {
	cg_dev_freq_t     dev_freq;
	cg_register_set_t register_set;
} cg_info_t;

extern u32_t cg_query_freq(u32_t dev_type);
void cg_xlat_n_assign(void);
void cg_init(void);
extern void set_spi_ctrl_divisor(u16_t clk_div, u16_t spif_mhz);
extern u32_t get_spi_ctrl_divisor(void);
extern u32_t get_default_spi_ctrl_divisor(void);
void cg_result_decode(void);

extern cg_info_t cg_info_query;


#endif
