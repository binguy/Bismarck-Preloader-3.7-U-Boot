#include <soc.h>
#include <uart/uart.h>
#include <cg/cg.h>
#include <apro/misc_setting.h>
#ifdef STICKY_FREQ_XLAT
#include <nor_spi/nor_spif_register.h>
#endif


#ifndef SECTION_CG_CORE_INIT
    #define SECTION_CG_CORE_INIT
#endif

#ifndef SECTION_CG_MISC
    #define SECTION_CG_MISC
#endif

#ifndef SECTION_CG_MISC_DATA
    #define SECTION_CG_MISC_DATA
#endif

cg_dev_freq_t cg_target_freq;

UTIL_FAR SECTION_CG_MISC
void reg_to_mhz(void)
{
    if(_soc.sid == PLR_SID_APRO){
        /* CPU0 */
        cg_info_query.cpu0_mhz = ((RFLD_SYS_OCP_PLL_CTRL0(ck_cpu_freq_sel0) + 2) * 50)/(1<<RFLD_SYS_OCP_PLL_CTRL3(reg_en_DIV2_cpu0));
        if(0 != RFLD_OC0_CMUGCR(cmu_mode)){
            cg_info_query.cpu0_mhz /= (1 << RFLD_OC0_CMUGCR(freq_div));
        }

        /* CPU1 */
        cg_info_query.cpu1_mhz = ((RFLD_SYS_OCP_PLL_CTRL0(ck_cpu_freq_sel1) + 2) * 25);
        if(0 != RFLD_OC1_CMUGCR(cmu_mode)){
            cg_info_query.cpu1_mhz /= (1 << RFLD_OC1_CMUGCR(freq_div));
        }
    }else if(_soc.sid == PLR_SID_APRO_GEN2){
        _cg_ocp_reg_to_mhz_gen2(&(cg_info_query.cpu0_mhz), &(cg_info_query.cpu1_mhz));
    }


    /* MEM */
    cg_info_query.mem_mhz = (25*((RFLD_SYS_MEM_PLL_CTRL3(n_code))+3)/(2*(1<<RFLD_SYS_MEM_PLL_CTRL2(pdiv))));
    cg_info_query.mem_mhz += (25*(RFLD_SYS_MEM_PLL_CTRL5(f_code))/2)/8192;

    /* LX */
    cg_info_query.lx_mhz  = 1000 / (RFLD_PHY_RG5X_PLL(ck200m_lx_pll) + 5);

    /* Flash: SPI NAND or SPI NOR*/
    #if defined (OTTO_FLASH_SPI_NAND)
        cg_info_query.spif_mhz = cg_info_query.lx_mhz / ((get_spi_ctrl_divisor() + 1)*2);
    #elif defined (OTTO_FLASH_NOR)
        cg_info_query.nor_pll_mhz = 1000 / (RFLD_PHY_RG5X_PLL(ck250m_spif_pll) + 4);
        cg_info_query.spif_mhz = cg_info_query.nor_pll_mhz / ((get_spi_ctrl_divisor() + 1)*2);
    #endif

    /* SRAM */
    u32_t div = (1<<(!RFLD_SYS_OCP_PLL_CTRL0(cksel_sram_500)));
    cg_info_query.sram_mhz = (RFLD_SYS_OCP_PLL_CTRL1(reg_sel_500M)==1)?(500/div):(400/div);
}

SECTION_CG_MISC
void apro_copy_proj_info_to_sram(void) {
	if (2 == (RFLD_MCR(dram_type)+1)) {
		memcpy(
			(char *)&cg_target_freq,
			(const char *)&apro_cg_ddr2_proj_freq,
			sizeof(cg_dev_freq_t));
	} else {
		memcpy(
			(char *)&cg_target_freq,
			(const char *)&apro_cg_ddr3_proj_freq,
			sizeof(cg_dev_freq_t));
	}

	if (0 == cg_target_freq.cpu0_mhz) {
		cg_target_freq.cpu0_mhz = apro_xlat_cpu_freq;
	}

	if (apro_xlat_ddr3_freq !=0)

	return;
}

SECTION_CG_MISC
void _cg_calc_sram(cg_register_set_t *rs)
{
    /* pll500M_en=1: the enable bit of SRAM_PLL
         * reg_sel_500M=1: SRAM_PLL=500
         * reg_sel_500M=0: SRAM_PLL=400
         * cksel_sram_500=1: divisor=1
         * cksel_sram_500=0: divisor=2
         */
    rs->sys_ocp_pll_ctrl1.f.pll500M_en = 1;
    if(cg_target_freq.sram_mhz == 500){
        rs->sys_ocp_pll_ctrl1.f.reg_sel_500M = 1;
        rs->sys_ocp_pll_ctrl0.f.cksel_sram_500 = 1;
    }else if(cg_target_freq.sram_mhz == 250){
        rs->sys_ocp_pll_ctrl1.f.reg_sel_500M = 1;
        rs->sys_ocp_pll_ctrl0.f.cksel_sram_500 = 0;
    }else if(cg_target_freq.sram_mhz == 400){
        rs->sys_ocp_pll_ctrl1.f.reg_sel_500M = 0;
        rs->sys_ocp_pll_ctrl0.f.cksel_sram_500 = 1;
    }else if(cg_target_freq.sram_mhz == 200){
        rs->sys_ocp_pll_ctrl1.f.reg_sel_500M = 0;
        rs->sys_ocp_pll_ctrl0.f.cksel_sram_500 = 0;
    }
    return;
}

SECTION_CG_MISC
void _cg_calc_ocp(cg_register_set_t *rs)
{
    // Step1: Calculate CPU0 PLL
    u32_t target_cpu0_clk = cg_target_freq.cpu0_mhz;
    rs->oc0_cmugcr.f.cmu_mode = DISABLE_CMU;

    if(cg_target_freq.cpu0_mhz>OCP0_PLL_MAX){
        target_cpu0_clk = OCP0_PLL_MAX;
    }else if(cg_target_freq.cpu0_mhz<OCP0_PLL_MIN){
        rs->oc0_cmugcr.f.cmu_mode = ENABLE_CMU_FIX_MODE;
    }

    u32_t cmu_div, input_pll, calc_clk=0, cur_clk=0, cur_pll=0, cur_cmu_div=0;
    if(target_cpu0_clk < 500){
        for(cmu_div=0 ; cmu_div<=7 ; cmu_div++){
            for(input_pll=OCP0_PLL_MIN ; input_pll<=OCP0_PLL_MAX ; input_pll+=25){
                calc_clk = input_pll/(1<<cmu_div);
                if((target_cpu0_clk >= calc_clk) && (calc_clk > cur_clk)){
                    cur_pll = input_pll;
                    cur_clk = calc_clk;
                    cur_cmu_div = cmu_div;
                    if(cur_clk == target_cpu0_clk){
                        rs->oc0_cmugcr.f.freq_div = cur_cmu_div;
                        target_cpu0_clk = cur_pll;
                        goto  ocp_pll_search;
                    }
                }
            }
        }
    }

    ocp_pll_search:
        if(target_cpu0_clk >= 800){
            rs->sys_ocp_pll_ctrl3.f.reg_en_DIV2_cpu0 = 0;
            rs->sys_ocp_pll_ctrl0.f.ck_cpu_freq_sel0 = (target_cpu0_clk/50)-2;
        }else{
            rs->sys_ocp_pll_ctrl3.f.reg_en_DIV2_cpu0 = 1;
            rs->sys_ocp_pll_ctrl0.f.ck_cpu_freq_sel0 = (target_cpu0_clk/25)-2;
        }

    // Step2: Calculate CPU1 PLL
    u32_t target_cpu1_clk = cg_target_freq.cpu1_mhz;
    rs->oc1_cmugcr.f.cmu_mode = DISABLE_CMU;

    if(cg_target_freq.cpu1_mhz > OCP1_PLL_MAX){
        target_cpu1_clk = OCP1_PLL_MAX;
    }else if(cg_target_freq.cpu1_mhz < OCP1_PLL_MIN){
        rs->oc1_cmugcr.f.cmu_mode = ENABLE_CMU_FIX_MODE;
    }

    if((target_cpu1_clk>=OCP1_PLL_MIN)&&(target_cpu1_clk<=OCP1_PLL_MAX)){
        rs->sys_ocp_pll_ctrl0.f.ck_cpu_freq_sel1 = (target_cpu1_clk/25)-2;
    }else{
        for(cmu_div=0 ; cmu_div<=7 ; cmu_div++){
            for(input_pll=OCP1_PLL_MIN ; input_pll<=OCP1_PLL_MAX ; input_pll+=25){
                calc_clk = input_pll/(1<<cmu_div);
                if((target_cpu1_clk >= calc_clk) && (calc_clk > cur_clk)){
                    cur_pll = input_pll;
                    cur_clk = calc_clk;
                    cur_cmu_div = cmu_div;
                    //printf("II: Enable OC1 CMU, %d/(1<<%d)=%d\n",cur_pll,cur_cmu_div,cur_clk);
                    if(target_cpu1_clk == calc_clk) goto assign_oc1reg;
                }
            }
        }
        assign_oc1reg:
            rs->sys_ocp_pll_ctrl0.f.ck_cpu_freq_sel1 = (cur_pll/25)-2;
            rs->oc1_cmugcr.f.freq_div = cur_cmu_div;
    }
    return;
}


extern u32_t find_mem_pll_setting(u32_t target_freq, mem_pll_info_t *ptr);
extern mem_pll_info_t apro_ddr2_pll[];
extern mem_pll_info_t apro_ddr2_mcm_pll[];
extern mem_pll_info_t apro_ddr3_pll[];
extern mem_pll_info_t apro_ddr3_mcm_pll[];
extern mem_pll_info_t apro_gen2_ddr2_pll[];
extern mem_pll_info_t apro_gen2_ddr2_mcm_pll[];
extern mem_pll_info_t apro_gen2_ddr3_pll[];
extern mem_pll_info_t apro_gen2_ddr3_mcm_pll[];

SECTION_CG_MISC
void _cg_calc_mem(cg_register_set_t *rs)
{
    u16_t target_freq = cg_target_freq.mem_mhz;
    u32_t ddr_type = (RFLD_MCR(dram_type)+1);
    u32_t is_mcm = (apro_xlat_dram_size_num==NA)?0:1;
    u32_t is_ddr3 = (ddr_type == 2)?0:1;
    u32_t idx;

    const mem_pll_info_t *pll_ptr;
    const mem_pll_info_t *mempll_preset[2][2] = {
        {apro_ddr2_pll, apro_ddr2_mcm_pll},
        {apro_ddr3_pll, apro_ddr3_mcm_pll}
    };

    const mem_pll_info_t *mempll_preset_gen2[2][2] = {
        {apro_gen2_ddr2_pll, apro_gen2_ddr2_mcm_pll},
        {apro_gen2_ddr3_pll, apro_gen2_ddr3_mcm_pll}
    };

    if(_soc.sid == PLR_SID_APRO){
        pll_ptr = mempll_preset[is_ddr3][is_mcm];
    }else if(_soc.sid == PLR_SID_APRO_GEN2){
        pll_ptr = mempll_preset_gen2[is_ddr3][is_mcm];
    }else{
        return;
    }

    idx = find_mem_pll_setting(target_freq, (mem_pll_info_t *)pll_ptr);
    rs->sys_mem_pll_ctrl0.v = pll_ptr[idx].pll0;
    rs->sys_mem_pll_ctrl1.v = pll_ptr[idx].pll1;
    rs->sys_mem_pll_ctrl2.v = pll_ptr[idx].pll2;
    rs->sys_mem_pll_ctrl3.v = pll_ptr[idx].pll3;
    rs->sys_mem_pll_ctrl5.v = pll_ptr[idx].pll5;
    rs->sys_mem_pll_ctrl6.v = SYS_MEM_PLL_CTRL6rv;
    return;
}


SECTION_CG_MISC_DATA
pll_freq_sel_info_t lx_pll_src[] = {
    {
        .freq_mhz = 200,
        .cfg_div  = 0,
    },
    {
        .freq_mhz = 166,
        .cfg_div  = 1,
    },
    {
        .freq_mhz = 142,
        .cfg_div  = 2,
    },
    {
        .freq_mhz = 125,
        .cfg_div  = 3,
    },
    { /* The end of structure */
        .freq_mhz = END_OF_INFO,
    }
};

SECTION_CG_MISC
void _cg_calc_lx(cg_register_set_t *rs)
{
    u32_t target_freq = cg_target_freq.lx_mhz;
    int i = -1;
    u32_t cur_lx;
    pll_freq_sel_info_t final = {0 ,0};

    while(1){
        cur_lx = lx_pll_src[++i].freq_mhz;
        if(END_OF_INFO == cur_lx) break;
        if((target_freq >= cur_lx) && (cur_lx > final.freq_mhz)){
            final.freq_mhz = cur_lx;
            final.cfg_div = lx_pll_src[i].cfg_div;
        }
    }

    if(0 == final.freq_mhz){
        final.freq_mhz = OTTO_LX_RESET_DEFAULT_MHZ;
        final.cfg_div = LX_PLL_DEFAULT_DIV;
        puts("WW: Unreasonable LX PLL. Set to Default.\n");
    }

    rs->phy_rg5x_pll.f.ck200m_lx_pll = final.cfg_div;
    return;
}


#if defined (OTTO_FLASH_SPI_NAND) || defined (OTTO_FLASH_NOR)

extern clk_div_sel_info_t sclk_divisor[];

SECTION_CG_MISC
void __cal_spi_ctrl_divisor(cg_sclk_info_t *info)
{
    u32_t target_sclk = cg_target_freq.spif_mhz;
    u32_t input_pll   = info->pll_freq_mhz;
    u32_t cur_sclk    = info->cal_sclk_mhz;
    int i = -1;
    u32_t cur_divisor;
    u32_t calc_sclk;

    while(1){
        cur_divisor = sclk_divisor[++i].divisor;

        if(END_OF_INFO == cur_divisor) break;

        calc_sclk = input_pll/cur_divisor;
        if((target_sclk >= calc_sclk) && (calc_sclk > cur_sclk)){
            cur_sclk = calc_sclk;
            info->cal_sclk_mhz  = calc_sclk;
            info->sclk_div2ctrl = sclk_divisor[i].div_to_ctrl;
        }
    }
    return;
}
#endif //#if defined (OTTO_FLASH_SPI_NAND) || defined (OTTO_FLASH_NOR)


#if defined (OTTO_FLASH_SPI_NAND)
SECTION_CG_MISC
void _cg_calc_spif(cg_register_set_t *rs)
{
    //Check the special limitation of sclk first
    cg_target_freq.spif_mhz = nsu_sclk_limit(cg_target_freq.spif_mhz);

    cg_sclk_info_t final = {0, 0, 0, 0};
    final.pll_freq_mhz    = cg_target_freq.lx_mhz;

    __cal_spi_ctrl_divisor(&final);

    if(final.cal_sclk_mhz >= 100){
        rs->spif_rx_delay = 2;
    }else if(final.cal_sclk_mhz >= 50){
        rs->spif_rx_delay = 1;
    }else if(final.cal_sclk_mhz == 0){
        final.sclk_div2ctrl = get_default_spi_ctrl_divisor();
    }

    rs->sclk_div2ctrl = final.sclk_div2ctrl;
    return;
}

SECTION_CG_CORE_INIT
void cg_spif_clk_init(cg_register_set_t *rs)
{
    if(cg_target_freq.spif_mhz != cg_info_query.spif_mhz){
        set_spi_ctrl_divisor(rs->sclk_div2ctrl, cg_target_freq.spif_mhz);
        set_spi_ctrl_latency(rs->spif_rx_delay);
    }
    return;
}

#elif defined (OTTO_FLASH_NOR)
SECTION_CG_MISC_DATA
pll_freq_sel_info_t nor_pll_src[] = {
    {
        .freq_mhz = 250,
        .cfg_div  = 0,
    },
    {
        .freq_mhz = 200,
        .cfg_div  = 1,
    },
    {
        .freq_mhz = 166,
        .cfg_div  = 2,
    },
    {
        .freq_mhz = 142,
        .cfg_div  = 3,
    },
    { /* The end of structure */
        .freq_mhz = END_OF_INFO,
    }
};

SECTION_CG_MISC
void _cg_calc_spif(cg_register_set_t *rs)
{
    cg_sclk_info_t current = {0 ,0 ,0 ,0};
    cg_sclk_info_t final = {0 ,0 ,0 ,0};

    int i = -1;
    while(1){
        current.pll_freq_mhz = nor_pll_src[++i].freq_mhz;
        if(END_OF_INFO == current.pll_freq_mhz) break;
        __cal_spi_ctrl_divisor(&current);

        if(final.cal_sclk_mhz >= 100){
            rs->spif_rx_delay = 0x1F1F1F1F;
        }else{
            rs->spif_rx_delay = 0;
        }

        if(current.cal_sclk_mhz > final.cal_sclk_mhz){
            final.pll_freq_mhz  = nor_pll_src[i].freq_mhz;
            final.pll_cfg_div   = nor_pll_src[i].cfg_div;
            final.sclk_div2ctrl = current.sclk_div2ctrl;
            final.cal_sclk_mhz  = current.cal_sclk_mhz;
        }
    }

    if(0 == final.cal_sclk_mhz){
        final.pll_cfg_div = NOR_PLL_DEFAULT_DIV;
        final.sclk_div2ctrl = get_default_spi_ctrl_divisor();
        puts("WW: Unreasonable SCLK. Set to Default divisor.\n");
    }
    cg_target_freq.nor_pll_mhz = final.pll_freq_mhz;
    rs->phy_rg5x_pll.f.ck250m_spif_pll = final.pll_cfg_div;
    rs->sclk_div2ctrl = final.sclk_div2ctrl;
    return;
}

SECTION_CG_CORE_INIT
void cg_spif_clk_init(cg_register_set_t *rs)
{
    /* Set SPI NOR slow bit */
    RMOD_OC0_CMUCR(se_spif, rs->oc0_cmucr.f.se_spif);
    RMOD_OC1_CMUCR(se_spif, rs->oc1_cmucr.f.se_spif);

    /* Set SPI NOR PLL */
    if(cg_target_freq.spif_mhz != cg_info_query.spif_mhz){
        PHY_RG5X_PLLrv = rs->phy_rg5x_pll.v;
        set_spi_ctrl_divisor(rs->sclk_div2ctrl, cg_target_freq.spif_mhz);
        set_spi_ctrl_latency(rs->spif_rx_delay);
        printf("II:%s done\r",__FUNCTION__);
    }
    return;
}
#endif //#elif defined (OTTO_FLASH_NOR)


SECTION_CG_MISC
void get_cg_reg_init_value(cg_register_set_t *rs)
{

    if(_soc.sid == PLR_SID_APRO){
        rs->sys_ocp_pll_ctrl0.v = SYS_OCP_PLL_CTRL0rv;
        rs->sys_ocp_pll_ctrl1.v = SYS_OCP_PLL_CTRL1rv;
        rs->sys_ocp_pll_ctrl3.v = SYS_OCP_PLL_CTRL3rv;
    }else if(_soc.sid == PLR_SID_APRO_GEN2){
        rs->sys_ocp_pll_ctrl0_gen2.v = SYS_OCP_PLL_CTRL0_GEN2rv;
        rs->sys_ocp_pll_ctrl1_gen2.v = SYS_OCP_PLL_CTRL1_GEN2rv;
        rs->sys_ocp_pll_ctrl2_gen2.v = SYS_OCP_PLL_CTRL2_GEN2rv;
        rs->sys_ocp_pll_ctrl3_gen2.v = SYS_OCP_PLL_CTRL3_GEN2rv;
    }

    rs->sys_mem_pll_ctrl0.v = SYS_MEM_PLL_CTRL0rv;
    rs->sys_mem_pll_ctrl1.v = SYS_MEM_PLL_CTRL1rv;
    rs->sys_mem_pll_ctrl2.v = SYS_MEM_PLL_CTRL2rv;
    rs->sys_mem_pll_ctrl3.v = SYS_MEM_PLL_CTRL3rv;
    rs->sys_mem_pll_ctrl5.v = SYS_MEM_PLL_CTRL5rv;
    rs->sys_mem_pll_ctrl6.v = SYS_MEM_PLL_CTRL6rv;

    rs->phy_rg5x_pll.v = PHY_RG5X_PLLrv;

    rs->oc0_cmugcr.v = OC0_CMUGCRrv;
    rs->oc1_cmugcr.v = OC1_CMUGCRrv;

    rs->oc0_cmucr.v  = OC0_CMUCRrv;
    rs->oc1_cmucr.v  = OC1_CMUCRrv;

    rs->lx_sbsr.v    = LBSBCRrv;

    rs->scats.v      = CPU_SRAM_SCATSrv;
    return;
}


SECTION_CG_MISC
void _cg_calc_slow_bit(cg_register_set_t *rs)
{
    u32_t ocp0_mhz;
    u32_t ocp1_mhz;
    u32_t slow_sc;
    u32_t slow_rc;
    u32_t slow_cs;
    u32_t slow_cr;

    /* Step1: SRAM & ROM relative slow bits
     *        Should:
     *         Could:
     *      Prohibit:
     */
    ocp0_mhz = cg_target_freq.cpu0_mhz/2;
    slow_sc = cg_target_freq.sram_mhz/ocp0_mhz;
    slow_rc = ROM_CLK_RESET_DEFAULT_MHZ/ocp0_mhz;
    //printf("II: slow_sc=%d, slow_rc=%d\n",slow_sc,slow_rc);
    if((slow_sc >= 2)||(slow_rc >= 2)){
        rs->oc0_cmucr.f.se_sram_rom = 1;
    }else{
        rs->oc0_cmucr.f.se_sram_rom = 0;
        if(slow_sc >= 1){
            rs->scats.f.sram_to_oc0 = 1;
        }else{
            rs->scats.f.sram_to_oc0 = 0;
        }
    }

    slow_cs = ocp0_mhz/cg_target_freq.sram_mhz;
    //printf("II: slow_cs=%d\n",slow_cs);
    if(slow_cs >= 2){
        rs->scats.f.oc0_to_sram = 2;
    }else if(slow_cs >= 1){
        rs->scats.f.oc0_to_sram = 1;
    }else{
        rs->scats.f.oc0_to_sram = 0;
    }

    slow_cr = ocp0_mhz / ROM_CLK_RESET_DEFAULT_MHZ;
    //printf("II: slow_sc=%d, slow_rc=%d\n",slow_sc,slow_rc);
    if(slow_cr >= 3){
        rs->scats.f.oc0_to_rom = 1;
    }else{
        rs->scats.f.oc0_to_rom = 0;
    }


    // Step2: Calculate CPU1 v.s. SRAM slow bits.
    ocp1_mhz = cg_target_freq.cpu1_mhz;
    slow_sc = cg_target_freq.sram_mhz/ocp1_mhz;
    slow_rc = ROM_CLK_RESET_DEFAULT_MHZ/ocp1_mhz;
    if((slow_sc >= 2)||(slow_rc >=2 )){
        rs->oc1_cmucr.f.se_sram_rom = 1;
    }else{
        rs->oc1_cmucr.f.se_sram_rom = 0;
        if(slow_sc >= 1){
            rs->scats.f.sram_to_oc1 = 1;
        }else{
            rs->scats.f.sram_to_oc1 = 0;
        }
    }

    slow_cs = ocp1_mhz/cg_target_freq.sram_mhz;
    if(slow_cs >= 2){
        rs->scats.f.oc1_to_sram = 2;
    }else if(slow_cs >= 1){
        rs->scats.f.oc1_to_sram = 1;
    }else{
        rs->scats.f.oc1_to_sram = 0;
    }

    slow_cr = ocp1_mhz/ROM_CLK_RESET_DEFAULT_MHZ;
    if(slow_cr >= 3){
        rs->scats.f.oc1_to_rom = 1;
    }else{
        rs->scats.f.oc1_to_rom = 0;
    }

    /* Step3: DRAM relative slow bits
     *        Should: OCP <= clkm,    LX <= clkm
     *         Could: OCP <  clkm *2
     *      Prohibit: OCP >  clkm *2
     */
    if(!(ocp0_mhz >= (cg_target_freq.mem_mhz *2))){
        rs->oc0_cmucr.f.se_dram = 1;
    }else{
        rs->oc0_cmucr.f.se_dram = 0;
    }


    if(!(ocp1_mhz >= (cg_target_freq.mem_mhz *2))){
        rs->oc1_cmucr.f.se_dram = 1;
    }else{
        rs->oc1_cmucr.f.se_dram = 0;
    }

    if(cg_target_freq.mem_mhz > cg_target_freq.lx_mhz){
        rs->lx_sbsr.f.pbo_egw_lx_frq_slower = 1;
        rs->lx_sbsr.f.pbo_usr_lx_frq_slower = 1;
        rs->lx_sbsr.f.pbo_usw_lx_frq_slower = 1;
        rs->lx_sbsr.f.pbo_dsr_lx_frq_slower = 1;
        rs->lx_sbsr.f.pbo_dsw_lx_frq_slower = 1;
        rs->lx_sbsr.f.lxp_frq_slower = 1;
        rs->lx_sbsr.f.oc2_frq_slower = 1;
        rs->lx_sbsr.f.lx2_frq_slower = 1;
        rs->lx_sbsr.f.lx1_frq_slower = 1;
        rs->lx_sbsr.f.lx0_frq_slower = 1;
    }

    #ifdef OTTO_FLASH_NOR
    /* Step4: SPI NOR relative slow bits
     *        Should: OCP0/1 <= sclk
     */
    rs->oc0_cmucr.f.se_spif = (cg_target_freq.nor_pll_mhz > ocp0_mhz);
    rs->oc1_cmucr.f.se_spif = (cg_target_freq.nor_pll_mhz > ocp1_mhz);
    #endif
}


UTIL_FAR SECTION_CG_MISC
void cg_mem_pll_init(cg_register_set_t *rs)
{
    /* Set slow bit */
    RMOD_OC0_CMUCR(se_dram, rs->oc0_cmucr.f.se_dram);
    RMOD_OC1_CMUCR(se_dram, rs->oc1_cmucr.f.se_dram);

#if 1
    /* Update DDR_PLL Change flow:
         * Toogle one bit of reg_crt_n_code at each write
         */
    CG_MEM_PLL_OE_DIS();
    udelay(200);

    SYS_MEM_PLL_CTRL0rv = rs->sys_mem_pll_ctrl0.v;
    SYS_MEM_PLL_CTRL1rv = rs->sys_mem_pll_ctrl1.v;
    SYS_MEM_PLL_CTRL2rv = rs->sys_mem_pll_ctrl2.v;

//    printf("DD: Target mem_clk=%d, target_ncode=0x%x, cur_reg=0x%x\n",cg_target_freq.mem_mhz, rs->sys_mem_pll_ctrl3.f.n_code, RFLD_SYS_MEM_PLL_CTRL3(n_code));
    u32_t tobe = rs->sys_mem_pll_ctrl3.f.n_code, cur_reg, temp;
    u32_t shift, next_cnt=0;
    u8_t next_time[8];
    u32_t min_ncode=9, max_ncode=85;
    for(shift=0;shift<8;shift++){
        cur_reg = RFLD_SYS_MEM_PLL_CTRL3(n_code);
        if(((cur_reg>>shift)&1) != ((tobe>>shift)&1)){
            temp = (cur_reg & (~(1<<shift))) | ((!((cur_reg>>shift)&1))<<shift);
            if((temp<min_ncode) || (temp>max_ncode)){
                next_time[next_cnt++] = shift;
            }else{
                rs->sys_mem_pll_ctrl3.f.n_code = temp;
                SYS_MEM_PLL_CTRL3rv = rs->sys_mem_pll_ctrl3.v;
//                printf("   (%d) current= %d %d %d %d %d %d %d %d\n", shift, ((cur_reg>>7)&1), ((cur_reg>>6)&1), ((cur_reg>>5)&1), ((cur_reg>>4)&1), ((cur_reg>>3)&1), ((cur_reg>>2)&1), ((cur_reg>>1)&1), ((cur_reg>>0)&1));
//                printf("       thisrun= %d %d %d %d %d %d %d %d\n",((temp>>7)&1), ((temp>>6)&1), ((temp>>5)&1), ((temp>>4)&1), ((temp>>3)&1), ((temp>>2)&1), ((temp>>1)&1), ((temp>>0)&1));
            }
        }
    }

    while(next_cnt>0){
        cur_reg  = RFLD_SYS_MEM_PLL_CTRL3(n_code);
        shift = next_time[--next_cnt];
        if((cur_reg>>shift) != (tobe>>shift)){
            temp = (cur_reg & (~(1<<shift))) | ((!((cur_reg>>shift)&1))<<shift);
            if((temp<min_ncode) || (temp>max_ncode)){
                printf("EE: Range error, temp=%d\n", temp);
            }else{
                rs->sys_mem_pll_ctrl3.f.n_code = temp;
                SYS_MEM_PLL_CTRL3rv = rs->sys_mem_pll_ctrl3.v;
//                printf("   (%d) current= %d %d %d %d %d %d %d %d\n", shift, ((cur_reg>>7)&1), ((cur_reg>>6)&1), ((cur_reg>>5)&1), ((cur_reg>>4)&1), ((cur_reg>>3)&1), ((cur_reg>>2)&1), ((cur_reg>>1)&1), ((cur_reg>>0)&1));
//                printf("       thisrun= %d %d %d %d %d %d %d %d\n\n", ((temp>>7)&1), ((temp>>6)&1), ((temp>>5)&1), ((temp>>4)&1), ((temp>>3)&1), ((temp>>2)&1), ((temp>>1)&1), ((temp>>0)&1));
            }
        }
    }


    if(tobe != RFLD_SYS_MEM_PLL_CTRL3(n_code)){
        for(shift=0;shift<8;shift++){
            cur_reg = RFLD_SYS_MEM_PLL_CTRL3(n_code);
            if(((cur_reg>>shift)&1) != ((tobe>>shift)&1)){
                temp = (cur_reg & (~(1<<shift))) | ((!((cur_reg>>shift)&1))<<shift);
                rs->sys_mem_pll_ctrl3.f.n_code = temp;
                SYS_MEM_PLL_CTRL3rv = rs->sys_mem_pll_ctrl3.v;
//                printf("   [%d] current= %d %d %d %d %d %d %d %d\n", shift, ((cur_reg>>7)&1), ((cur_reg>>6)&1), ((cur_reg>>5)&1), ((cur_reg>>4)&1), ((cur_reg>>3)&1), ((cur_reg>>2)&1), ((cur_reg>>1)&1), ((cur_reg>>0)&1));
//                printf("       thisrun= %d %d %d %d %d %d %d %d\n",((temp>>7)&1), ((temp>>6)&1), ((temp>>5)&1), ((temp>>4)&1), ((temp>>3)&1), ((temp>>2)&1), ((temp>>1)&1), ((temp>>0)&1));
            }
        }
    }

    SYS_MEM_PLL_CTRL5rv = rs->sys_mem_pll_ctrl5.v;
    SYS_MEM_PLL_CTRL6rv = rs->sys_mem_pll_ctrl6.v;
    udelay(5);

    CG_MEM_PLL_OE_EN();
    udelay(200);
#endif

#if 0 //Adding DDR_PLL Reset step
    u32_t ocp_freq = cg_target_freq.cpu0_mhz;
    CG_MEM_PLL_OE_DIS();
    cg_udelay(200, ocp_freq);
    RMOD_SYS_OCP_PLL_CTRL1(pll_ddr_rstb_in, 0);
    cg_udelay(200, ocp_freq);

    SYS_MEM_PLL_CTRL0rv = rs->sys_mem_pll_ctrl0.v;
    SYS_MEM_PLL_CTRL1rv = rs->sys_mem_pll_ctrl1.v;
    SYS_MEM_PLL_CTRL2rv = rs->sys_mem_pll_ctrl2.v;
    SYS_MEM_PLL_CTRL3rv = rs->sys_mem_pll_ctrl3.v;
    SYS_MEM_PLL_CTRL5rv = rs->sys_mem_pll_ctrl5.v;
    SYS_MEM_PLL_CTRL6rv = rs->sys_mem_pll_ctrl6.v;
    cg_udelay(5, ocp_freq);

    RMOD_SYS_OCP_PLL_CTRL1(pll_ddr_rstb_in, 1);
    cg_udelay(200, ocp_freq);
    CG_MEM_PLL_OE_EN();
    cg_udelay(200, ocp_freq);
#endif

#if 0 //Original DDR_PLL Change Flow
    if(cg_target_freq.mem_mhz != cg_info_query.mem_mhz){
        /* Change MEM PLL*/
        u32_t ocp_freq = cg_target_freq.cpu0_mhz;
        CG_MEM_PLL_OE_DIS();
        cg_udelay(200, ocp_freq);
        SYS_MEM_PLL_CTRL0rv = rs->sys_mem_pll_ctrl0.v;
        SYS_MEM_PLL_CTRL1rv = rs->sys_mem_pll_ctrl1.v;
        SYS_MEM_PLL_CTRL2rv = rs->sys_mem_pll_ctrl2.v;
        SYS_MEM_PLL_CTRL3rv = rs->sys_mem_pll_ctrl3.v;
        SYS_MEM_PLL_CTRL5rv = rs->sys_mem_pll_ctrl5.v;;
        SYS_MEM_PLL_CTRL6rv = rs->sys_mem_pll_ctrl6.v;;
        cg_udelay(5, ocp_freq);
        CG_MEM_PLL_OE_EN();
        cg_udelay(200, ocp_freq);
    }
#endif
    return;
}

SECTION_CG_CORE_INIT
void cg_sram_pll_init(cg_register_set_t *rs)
{
    /* Set SRAM Slow Bit */
    RMOD_OC0_CMUCR(se_sram_rom, rs->oc0_cmucr.f.se_sram_rom);
    RMOD_OC1_CMUCR(se_sram_rom, rs->oc1_cmucr.f.se_sram_rom);
    CPU_SRAM_SCATSrv = rs->scats.v;

    if(cg_target_freq.sram_mhz!= cg_info_query.sram_mhz){
        /* Set SRAM PLL */
        RMOD_SYS_OCP_PLL_CTRL0(cksel_sram_500, rs->sys_ocp_pll_ctrl0.f.cksel_sram_500);
        printf("II: %s done\r\r",__FUNCTION__);
    }
    return;
}

SECTION_CG_CORE_INIT
inline static void cg_cpu_pll_init(cg_register_set_t *rs)
{
    /* Step1: Disable CMU, avoiding OCP PLL too slow*/
    if(RFLD_OC0_CMUGCR(cmu_mode) != DISABLE_CMU){
        RMOD_OC0_CMUGCR(cmu_mode, DISABLE_CMU);
        udelay(10);
    }

    /* Step2: min SRAM_freq=250, 250/(200/2)=2.5, set SRAM slow bit for safe */
    RMOD_OC0_CMUCR(se_sram_rom, 1);
    RMOD_OC1_CMUCR(se_sram_rom, 1);

    /* Step3: Change OCP PLL to a stable frequency "LX" */
    RMOD_SYS_STATUS(cf_ckse_ocp0, 0);
    RMOD_SYS_STATUS(cf_ckse_ocp1, 0);
    udelay(1);

    /* Step4: Restore cpu pll to default & Set CPU1_N_L to change CPU PLL*/
    if(_soc.sid == PLR_SID_APRO){
        SYS_OCP_PLL_CTRL0rv = rs->sys_ocp_pll_ctrl0.v;
        SYS_OCP_PLL_CTRL1rv = rs->sys_ocp_pll_ctrl1.v;
        SYS_OCP_PLL_CTRL3rv = rs->sys_ocp_pll_ctrl3.v;
    }else if(_soc.sid == PLR_SID_APRO_GEN2){
        SYS_OCP_PLL_CTRL0_GEN2rv = rs->sys_ocp_pll_ctrl0_gen2.v;
        SYS_OCP_PLL_CTRL1_GEN2rv = rs->sys_ocp_pll_ctrl1_gen2.v;
        SYS_OCP_PLL_CTRL2_GEN2rv = rs->sys_ocp_pll_ctrl2_gen2.v;
        SYS_OCP_PLL_CTRL3_GEN2rv = rs->sys_ocp_pll_ctrl3_gen2.v;
    }

    /* Step5: Delay 40 us for safe*/
    udelay(40);

    if(cg_target_freq.sram_mhz!= cg_info_query.sram_mhz){
        /* Set SRAM PLL */
        RMOD_SYS_OCP_PLL_CTRL0(cksel_sram_500, rs->sys_ocp_pll_ctrl0.f.cksel_sram_500);
//        printf("II:%s abc\r",__FUNCTION__);
    }


    /* Step6: Recover OCP PLL */
    RMOD_SYS_STATUS(cf_ckse_ocp0, 1);
    RMOD_SYS_STATUS(cf_ckse_ocp1, 1);
    udelay(1);

    RMOD_OC0_CMUCR(se_sram_rom, rs->oc0_cmucr.f.se_sram_rom);
    RMOD_OC1_CMUCR(se_sram_rom, rs->oc1_cmucr.f.se_sram_rom);
    CPU_SRAM_SCATSrv = rs->scats.v;


    /* Step7: Set CPU to ROM/SRAM/DRAM slow bit*/
    OC0_CMUCRrv = rs->oc0_cmucr.v;
    OC1_CMUCRrv = rs->oc1_cmucr.v;

    /* Step8: Set CMU Mode, Divisor*/
    RMOD_OC0_CMUGCR(freq_div, rs->oc0_cmugcr.f.freq_div);
    RMOD_OC1_CMUGCR(freq_div, rs->oc1_cmugcr.f.freq_div);

    RMOD_OC0_CMUGCR(cmu_mode, rs->oc0_cmugcr.f.cmu_mode);
    RMOD_OC1_CMUGCR(cmu_mode, rs->oc1_cmugcr.f.cmu_mode);

    printf("II: %s done\r\r",__FUNCTION__);
    return;
}

SECTION_CG_CORE_INIT
static inline void cg_lx_pll_init(cg_register_set_t *rs)
{
    /* Set LX Slow Bit */
    LBSBCRrv = rs->lx_sbsr.v;

    /* Change LX PLL */
    if(cg_target_freq.lx_mhz != cg_info_query.lx_mhz){
        PHY_RG5X_PLLrv = rs->phy_rg5x_pll.v;
        printf("II:%s done\r",__FUNCTION__);
    }

    /* Update UART baudrate and LX timer configuration */
    uart_init(uart_info.baud, cg_info_query.lx_mhz);
    lx_timer_init(cg_info_query.lx_mhz);

    return;
}

SECTION_CG_MISC
void cg_register_preset(cg_register_set_t *rs)
{
    //Step1: Get all register default values for initialization
    get_cg_reg_init_value(rs);
    reg_to_mhz();

#ifdef STICKY_FREQ_XLAT
    if(SFPPRrv == 4){
        cg_target_freq.mem_mhz = 750;
        SFPPRrv = 0;
    }
#endif

    //Step2: Translate the target_freq into register
    if( _soc.sid == PLR_SID_APRO) {
        _cg_calc_ocp(rs);
    } else if ( _soc.sid == PLR_SID_APRO_GEN2) {
        _cg_calc_ocp_gen2(rs);
    }

    _cg_calc_mem(rs);
    _cg_calc_sram(rs);
    _cg_calc_lx(rs);
    #if defined (OTTO_FLASH_SPI_NAND) || defined (OTTO_FLASH_NOR)
    _cg_calc_spif(rs);
    #endif
    _cg_calc_slow_bit(rs);
    return;
}

SECTION_CG_CORE_INIT
void cg_init_in_unswap_sram(cg_register_set_t *rs)
{
    cg_cpu_pll_init(rs);
    cg_sram_pll_init(rs);
    cg_lx_pll_init(rs);
    #if defined (OTTO_FLASH_SPI_NAND) || defined (OTTO_FLASH_NOR)
    cg_spif_clk_init(rs);
    #endif

    //Add Branch prediction barrier
    BP_BARRIER();
    return;
}

SECTION_CG_MISC
void cg_init_in_swap_ram(cg_register_set_t *rs)
{
    cg_mem_pll_init(rs);

    return;
}

SECTION_CG_MISC
void cg_result_decode(void)
{
    printf("II: CPU0 %dMHz, CPU1 %dMHz, DRAM %dMHz, LX %dMHz, SRAM %dMHz",
        cg_info_query.cpu0_mhz,
        cg_info_query.cpu1_mhz,
        cg_info_query.mem_mhz,
        cg_info_query.lx_mhz,
        cg_info_query.sram_mhz);

    #if defined (OTTO_FLASH_SPI_NAND) || defined (OTTO_FLASH_NOR)
    printf(", SPIF %dMHz\n", cg_info_query.spif_mhz);
    #else
    puts("\n");
    #endif
    return;
}

SECTION_CG_MISC
void apro_cg_init(void)
{
	cg_register_set_t cg_rs;

	cg_register_preset(&cg_rs);

	cg_init_in_unswap_sram(&cg_rs);

	cg_init_in_swap_ram(&cg_rs);

	reg_to_mhz();

	cg_result_decode();
	return;
}

