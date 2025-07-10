#include <soc.h>
#include <uart/uart.h>
#include <cg/cg.h>

#ifndef SECTION_CG_MISC
    #define SECTION_CG_MISC
#endif

/*  Declaration of global parameter and functions */
cg_dev_freq_t cg_info_query.dev_freq;

SECTION_CG_MISC
u32_t cg_query_freq(u32_t dev_type)
{
	if(0 == cg_info_query.dev_freq.cpu0_mhz){
		reg_to_mhz();
	}
    return CG_QUERY_FREQUENCY(dev_type,(&cg_info_query.dev_freq));
}

SECTION_CG_MISC
void cg_result_decode(void)
{
    inline_memcpy(&cg_info_query, &cg_info_proj, sizeof(cg_info_t));

	printf("II: OCP %dMHz, MEM %dMHz, LX %dMHz, SPIF %dMHz\n",
		cg_info_query.dev_freq.cpu0_mhz,
		cg_info_query.dev_freq.mem_mhz,
		cg_info_query.dev_freq.lx_mhz,
		cg_info_query.dev_freq.spif_mhz);
	return;
}

SECTION_CG_MISC
void cg_init(void)
{
    reg_to_mhz();
    uart_init(uart_baud_rate, cg_info_query.dev_freq.lx_mhz);
    cg_result_decode();
}


REG_INIT_FUNC(cg_init, 13);

symb_pdefine(cg_info_dev_freq, SF_SYS_CG_DEV_FREQ, (void *)(&cg_info_query.dev_freq));

