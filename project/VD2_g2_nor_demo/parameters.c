#include <soc.h>
#include <cg/cg.h>
#include <uart/uart.h>
#include <plr_sections.h>

#undef _soc

extern void writeback_invalidate_dcache_all(void);
extern void invalidate_icache_all(void);
extern init_table_entry_t start_of_init_func_table, end_of_init_func_table;
extern symbol_table_entry_t start_of_symble_table, end_of_symble_table;

soc_t _soc SECTION_SOC_STRU = {
    .bios={
        .header= {
            .signature=SIGNATURE_PLR,
            .version=PLR_VERSION,
            .export_symb_list=&start_of_symble_table,
            .end_of_export_symb_list=&end_of_symble_table,
            .init_func_list=&start_of_init_func_table,
            .end_of_init_func_list=&end_of_init_func_table,
        },
        .isr=VZERO,
        .size_of_plr_load_firstly=0,
        .uart_putc=VZERO,
        .uart_getc=VZERO,
        .uart_tstc=VZERO,
        .dcache_writeback_invalidate_all= &writeback_invalidate_dcache_all,
        .icache_invalidate_all=&invalidate_icache_all,
    },
};


const cg_info_t cg_info_proj SECTION_PARAMETERS = {
	.dev_freq.spif_mhz = 100,
	.dev_freq.lx_mhz = 200,
};

const u32_t uart_baud_rate SECTION_PARAMETERS=115200;

symb_fdefine(SF_SYS_UDELAY, otto_lx_timer_udelay);
symb_fdefine(SF_SYS_MDELAY, otto_lx_timer_mdelay);
symb_fdefine(SF_SYS_GET_TIMER, otto_lx_timer_get_timer);
symb_fdefine(SF_PROTO_PRINTF, proto_printf);
