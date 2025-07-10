#ifndef __APRO_CG_H__
#define __APRO_CG_H__

#include <reg_map_util.h>
#include <apro/ocp_pll.h>
#include <apro/ocp_pll_gen2.h>
#include <apro/mem_pll.h>
#include <apro/cmu_reg.h>

/****** Default PLL Frequency Information******/
#define CPU0_RESET_MHZ            (500)
#define LX_PLL_DEFAULT_DIV        (0)
#define NOR_PLL_DEFAULT_DIV       (1)
#define ROM_CLK_RESET_DEFAULT_MHZ (250)
#define OCP0_PLL_MIN              (500)
#define OCP0_PLL_MAX              (1200)
#define OCP1_PLL_MIN              (400)
#define OCP1_PLL_MAX              (600)
#define OCP0_PLL_MAX_GEN2         (1400)
#define OCP1_PLL_MAX_GEN2         (800)

/****** Gerneal ********/
#define END_OF_INFO (0xFFFF)

typedef struct {
    u16_t pll_freq_mhz;
    u16_t pll_cfg_div;
    u16_t cal_sclk_mhz;
    u16_t sclk_div2ctrl;
} cg_sclk_info_t;

typedef struct pll_freq_sel_info_s{
    u16_t freq_mhz;
    u16_t cfg_div;
}pll_freq_sel_info_t;

extern u32_t nsu_sclk_limit(u32_t cur_sclk);
extern void set_spi_ctrl_latency(u16_t latency);
extern void set_spi_ctrl_divisor(u16_t clk_div, u16_t spif_mhz);
extern u32_t get_spi_ctrl_divisor(void);
extern u32_t get_default_spi_ctrl_divisor(void);


typedef struct {
    SYS_OCP_PLL_CTRL0_T sys_ocp_pll_ctrl0;
    SYS_OCP_PLL_CTRL1_T sys_ocp_pll_ctrl1;
    SYS_OCP_PLL_CTRL3_T sys_ocp_pll_ctrl3;

    SYS_OCP_PLL_CTRL0_GEN2_T sys_ocp_pll_ctrl0_gen2;
    SYS_OCP_PLL_CTRL1_GEN2_T sys_ocp_pll_ctrl1_gen2;
    SYS_OCP_PLL_CTRL2_GEN2_T sys_ocp_pll_ctrl2_gen2;
    SYS_OCP_PLL_CTRL3_GEN2_T sys_ocp_pll_ctrl3_gen2;

    SYS_MEM_PLL_CTRL0_T sys_mem_pll_ctrl0;
    SYS_MEM_PLL_CTRL1_T sys_mem_pll_ctrl1;
    SYS_MEM_PLL_CTRL2_T sys_mem_pll_ctrl2;
    SYS_MEM_PLL_CTRL3_T sys_mem_pll_ctrl3;
    SYS_MEM_PLL_CTRL4_T sys_mem_pll_ctrl4;
    SYS_MEM_PLL_CTRL5_T sys_mem_pll_ctrl5;
    SYS_MEM_PLL_CTRL6_T sys_mem_pll_ctrl6;

    PHY_RG5X_PLL_T      phy_rg5x_pll;

    u16_t sclk_div2ctrl; /* for SPI NAND & SPI NOR controller divisor */
    u32_t spif_rx_delay; /* for SPI NAND & SPI NOR RX delay latency */

    OC0_CMUGCR_T     oc0_cmugcr;  /* for CMU static division */
    OC0_CMUCR_T      oc0_cmucr;   /* for CPU0 slow bit setting */
    OC1_CMUGCR_T     oc1_cmugcr;  /* for CMU static division */
    OC1_CMUCR_T      oc1_cmucr;   /* for CPU1 slow bit setting */
    LBSBCR_T         lx_sbsr;     /* for LXs slow bit setting */
    CPU_SRAM_SCATS_T scats;       /* for SRAM slow bit setting */
} cg_register_set_t;


/****** APro ******/
extern void apro_copy_proj_info_to_sram(void);
extern __attribute__ ((far)) void apro_cg_init(void);
extern const cg_dev_freq_t apro_cg_ddr2_proj_freq SECTION_PARAMETERS;
extern const cg_dev_freq_t apro_cg_ddr3_proj_freq SECTION_PARAMETERS;

/****** APro Gen2 ******/
extern void apro_gen2_copy_proj_info_to_sram(void);
extern void _cg_ocp_reg_to_mhz_gen2(u16_t *cpu0_mhz, u16_t *cpu1_mhz);
extern void _cg_calc_ocp_gen2(cg_register_set_t *rs);
extern const cg_dev_freq_t apro_gen2_cg_ddr2_proj_freq SECTION_PARAMETERS;
extern const cg_dev_freq_t apro_gen2_cg_ddr3_proj_freq SECTION_PARAMETERS;

#endif
