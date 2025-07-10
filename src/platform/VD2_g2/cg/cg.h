#ifndef __CG_HEADER__
#define __CG_HEADER__

#include <register_map.h>
#include <dram/memcntlr_reg.h>
#include "cg_dev_freq.h"
#include "ocp_pll.h"
#include "mem_pll.h"
#include "gphy_pll.h"

#define END_OF_INFO (0xFFFF)

#define CPU_CLK_IN_DEFAULT (500)
#define MEM_PLL_MIN        (250)
#define MEM_PLL_MAX        (600)
#define MEM_PLL_STEP       (25)
#define MEM_PLL_ELEMENTS   (((MEM_PLL_MAX-MEM_PLL_MIN)/MEM_PLL_STEP)+1)

typedef struct {
} cg_register_set_t;

typedef struct {
	cg_dev_freq_t     dev_freq;
	cg_register_set_t register_set;
} cg_info_t;

typedef struct {
	u16_t divisor;
	u16_t div_to_ctrl;
} clk_div_sel_info_t;

typedef struct {
    u16_t cal_sclk_mhz;
    u16_t sclk_div2ctrl;
    u16_t spif_rx_delay;
} cg_sclk_info_t;

extern u32_t cg_query_freq(u32_t dev_type);
void cg_xlat_n_assign(void);
void cg_init(void);
void cg_result_decode(void);

extern cg_info_t cg_info_query;

extern u32_t get_spi_ctrl_divisor(void) __attribute__ ((far));
extern u32_t nsu_sclk_limit(u32_t cur_sclk);
extern void set_spi_ctrl_latency(u16_t latency);
extern void set_spi_ctrl_divisor(u16_t clk_div, u16_t spif_mhz);
extern u32_t get_default_spi_ctrl_divisor(void);
#endif

