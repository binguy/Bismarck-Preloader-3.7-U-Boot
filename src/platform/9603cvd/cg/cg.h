#ifndef __CG_HEADER__
#define __CG_HEADER__

#include <register_map.h>
#include "cg_dev_freq.h"
#include "ocp_pll.h"
#include "mem_pll.h"
#include "gphy_pll.h"
#include "cmu.h"

SKC35_REG_DEC(
	SCATS, 0xb80040f8,
	RF_RSV(31, 9);
	RFIELD(8, 8, sram_to_lx);
	RFIELD(7, 7, sram_to_oc1);
	RFIELD(6, 6, sram_to_oc0);
	RFIELD(5, 5, oc1_to_rom);
	RFIELD(4, 4, oc0_to_rom);
	RFIELD(3, 2, oc1_to_sram);
	RFIELD(1, 0, oc0_to_sram);
	);

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
} mem_pll_info_t;

u32_t cg_query_freq(u32_t dev_type) __attribute__ ((far));

extern cg_dev_freq_t cg_info_query;

#ifndef GET_MEM_MHZ
#	define GET_MEM_MHZ() (cg_query_freq(CG_DEV_MEM))
#endif

#ifndef GET_CPU_MHZ
#	define GET_CPU_MHZ() (cg_query_freq(CG_DEV_CPU0))
#endif

extern void set_spi_ctrl_latency(u16_t latency);
extern void set_spi_ctrl_divisor(u16_t clk_div, u16_t spif_mhz);
extern u32_t get_spi_ctrl_divisor(void);
extern u32_t get_default_spi_ctrl_divisor(void);
extern void cg_init(void);
extern u16_t _cg_lx_init(u16_t freq);
#endif
