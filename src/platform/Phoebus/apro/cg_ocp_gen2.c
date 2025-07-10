#include <soc.h>
#include <cg/cg.h>
#include <apro/misc_setting.h>

#ifndef SECTION_CG_CORE_INIT
    #define SECTION_CG_CORE_INIT
#endif

#ifndef SECTION_CG_MISC
    #define SECTION_CG_MISC
#endif

#ifndef SECTION_CG_MISC_DATA
    #define SECTION_CG_MISC_DATA
#endif

SECTION_CG_MISC
void apro_gen2_copy_proj_info_to_sram(void) {
	if (2 == (RFLD_MCR(dram_type)+1)) {
		memcpy(
			(char *)&cg_target_freq,
			(const char *)&apro_gen2_cg_ddr2_proj_freq,
			sizeof(cg_dev_freq_t));
	} else {
		memcpy(
			(char *)&cg_target_freq,
			(const char *)&apro_gen2_cg_ddr3_proj_freq,
			sizeof(cg_dev_freq_t));

        if (0 != apro_xlat_ddr3_freq) {
            cg_target_freq.mem_mhz = apro_xlat_ddr3_freq;
        }
	}

	if (0 == cg_target_freq.cpu0_mhz) {
		cg_target_freq.cpu0_mhz = apro_xlat_cpu_freq;
	}
	return;
}


SECTION_CG_MISC
void _cg_ocp_reg_to_mhz_gen2(u16_t *cpu0_mhz, u16_t *cpu1_mhz)
{
    u32_t cpu0, cpu1;

	cpu0 = ((RFLD_SYS_OCP_PLL_CTRL0_GEN2(ck_cpu_freq_sel0) + 2) * 50)/(1<<RFLD_SYS_OCP_PLL_CTRL3_GEN2(reg_en_DIV2_cpu0));

	if(0 != RFLD_OC0_CMUGCR(cmu_mode)){
		cpu0 /= (1 << RFLD_OC0_CMUGCR(freq_div));
    }

	cpu1 = ((RFLD_SYS_OCP_PLL_CTRL0_GEN2(ck_cpu_freq_sel1) + 2) * 25);
	if(0 != RFLD_OC1_CMUGCR(cmu_mode)){
		cpu1 /= (1 << RFLD_OC1_CMUGCR(freq_div));
	}
	*cpu0_mhz = cpu0;
	*cpu1_mhz = cpu1;

    return;
}

SECTION_CG_MISC
void _cg_calc_ocp_gen2(cg_register_set_t *rs)
{
    // Step1: Calculate CPU0 PLL
    u32_t target_cpu0_clk = cg_target_freq.cpu0_mhz;
    rs->oc0_cmugcr.f.cmu_mode = DISABLE_CMU;

    if(cg_target_freq.cpu0_mhz > OCP0_PLL_MAX_GEN2){
        target_cpu0_clk = OCP0_PLL_MAX_GEN2;
    }else if(cg_target_freq.cpu0_mhz < OCP0_PLL_MIN){
        rs->oc0_cmugcr.f.cmu_mode = ENABLE_CMU_FIX_MODE;
    }

    u32_t cmu_div, input_pll, calc_clk=0, cur_clk=0, cur_pll=0, cur_cmu_div=0;
    if(target_cpu0_clk < 500){
        for(cmu_div=0 ; cmu_div<=7 ; cmu_div++){
            for(input_pll=OCP0_PLL_MIN ; input_pll<=OCP0_PLL_MAX_GEN2 ; input_pll+=25){
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
        if(target_cpu0_clk>=800){
            rs->sys_ocp_pll_ctrl3_gen2.f.reg_en_DIV2_cpu0 = 0;
            rs->sys_ocp_pll_ctrl0_gen2.f.ck_cpu_freq_sel0 = (target_cpu0_clk/50)-2;
        }else{
            rs->sys_ocp_pll_ctrl3_gen2.f.reg_en_DIV2_cpu0 = 1;
            rs->sys_ocp_pll_ctrl0_gen2.f.ck_cpu_freq_sel0 = (target_cpu0_clk/25)-2;
        }

	if (rs->sys_ocp_pll_ctrl0_gen2.f.ck_cpu_freq_sel0 > 25) {
		rs->sys_ocp_pll_ctrl2_gen2.f.reg_cp_bias_cpu0 = 5;
		rs->sys_ocp_pll_ctrl2_gen2.f.reg_kvco_cpu0 = 1;
		rs->sys_ocp_pll_ctrl2_gen2.f.reg_mcco0 = 1;
	} else if (rs->sys_ocp_pll_ctrl0_gen2.f.ck_cpu_freq_sel0 > 20) {
		rs->sys_ocp_pll_ctrl2_gen2.f.reg_cp_bias_cpu0 = 4;
		rs->sys_ocp_pll_ctrl2_gen2.f.reg_kvco_cpu0 = 1;
		rs->sys_ocp_pll_ctrl2_gen2.f.reg_mcco0 = 0;
	} else {
		rs->sys_ocp_pll_ctrl2_gen2.f.reg_cp_bias_cpu0 = 4;
		rs->sys_ocp_pll_ctrl2_gen2.f.reg_kvco_cpu0 = 0;
		rs->sys_ocp_pll_ctrl2_gen2.f.reg_mcco0 = 0;
	}



    // Step2: Calculate CPU1 PLL
    u32_t target_cpu1_clk = cg_target_freq.cpu1_mhz;
    rs->oc1_cmugcr.f.cmu_mode = DISABLE_CMU;

    if(cg_target_freq.cpu1_mhz > OCP1_PLL_MAX_GEN2){
        target_cpu1_clk = OCP1_PLL_MAX_GEN2;
    }else if(cg_target_freq.cpu1_mhz < OCP1_PLL_MIN){
        rs->oc1_cmugcr.f.cmu_mode = ENABLE_CMU_FIX_MODE;
    }

    if((target_cpu1_clk>=OCP1_PLL_MIN) && (target_cpu1_clk<=OCP1_PLL_MAX_GEN2)){
        rs->sys_ocp_pll_ctrl0_gen2.f.ck_cpu_freq_sel1 = (target_cpu1_clk/25)-2;
    }else{
        for(cmu_div=0 ; cmu_div<=7 ; cmu_div++){
            for(input_pll=OCP1_PLL_MIN ; input_pll<=OCP1_PLL_MAX_GEN2 ; input_pll+=25){
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
            rs->sys_ocp_pll_ctrl0_gen2.f.ck_cpu_freq_sel1 = (cur_pll/25)-2;
            rs->oc1_cmugcr.f.freq_div = cur_cmu_div;
    }

	if (rs->sys_ocp_pll_ctrl0_gen2.f.ck_cpu_freq_sel1 > 25) {
		rs->sys_ocp_pll_ctrl2_gen2.f.reg_cp_bias_cpu1 = 5;
		rs->sys_ocp_pll_ctrl2_gen2.f.reg_kvco_cpu1 = 1;
		rs->sys_ocp_pll_ctrl2_gen2.f.reg_mcco1 = 1;
	} else if (rs->sys_ocp_pll_ctrl0_gen2.f.ck_cpu_freq_sel1 > 20) {
		rs->sys_ocp_pll_ctrl2_gen2.f.reg_cp_bias_cpu1 = 4;
		rs->sys_ocp_pll_ctrl2_gen2.f.reg_kvco_cpu1 = 1;
		rs->sys_ocp_pll_ctrl2_gen2.f.reg_mcco1 = 0;
	} else {
		rs->sys_ocp_pll_ctrl2_gen2.f.reg_cp_bias_cpu1 = 4;
		rs->sys_ocp_pll_ctrl2_gen2.f.reg_kvco_cpu1 = 0;
		rs->sys_ocp_pll_ctrl2_gen2.f.reg_mcco1 = 0;
	}

    return;
}

