#include <soc.h>
#include <cli/cli_util.h>
#include <cli/cli_access.h>

#define PLL_INFO_SET cg_target_freq
#define PLL_INFO_GET cg_info_query

cli_cmd_ret_t
cli_pll_setup(const void *user, u32_t argc, const char *argv[]) {
	if ((_soc.sid == PLR_SID_APRO) || (_soc.sid == PLR_SID_APRO_GEN2)){
		apro_cg_init();
	}else {
	    cg_info_query.bypass_cg_init = 1;
		rtl9603cvd_cg_init();
	}
	return CCR_OK;
}

cli_add_node(pll, get, VZERO);
cli_add_parent(pll, set);

#define DEFINE_PLL_INT_VAR(name, is_dec, get_func_body, set_func_body) \
	SECTION_CLI_VAR int _CLI_VAR_CLI_ ## name ## _get_int_(u32_t *result) { \
		get_func_body; return 0;}	\
	SECTION_CLI_VAR int _CLI_VAR_CLI_ ## name ## _set_int_(u32_t value) { \
		set_func_body; return 0;}	\
	CLI_DEFINE_VAR(name, pll, 1, 0, is_dec,	\
								 _CLI_VAR_CLI_ ## name ## _get_int_, \
								 _CLI_VAR_CLI_ ## name ## _set_int_)

DEFINE_PLL_INT_VAR(cpu0_mhz, 1, {*result=PLL_INFO_GET.cpu0_mhz;}, {PLL_INFO_SET.cpu0_mhz=value;});
DEFINE_PLL_INT_VAR(mem_mhz, 1, {*result=PLL_INFO_GET.mem_mhz;}, {PLL_INFO_SET.mem_mhz=value;});
DEFINE_PLL_INT_VAR(lx_mhz, 1, {*result=PLL_INFO_GET.lx_mhz;}, {PLL_INFO_SET.lx_mhz=value;});
DEFINE_PLL_INT_VAR(spif_mhz, 1, {*result=PLL_INFO_GET.spif_mhz;}, {PLL_INFO_SET.spif_mhz=value;});
DEFINE_PLL_INT_VAR(sram_mhz, 1, {*result=PLL_INFO_GET.sram_mhz;}, {PLL_INFO_SET.sram_mhz=value;});
