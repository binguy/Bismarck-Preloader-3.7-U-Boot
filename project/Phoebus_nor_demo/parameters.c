#include <soc.h>
#include <cg/cg.h>
#include <plr_sections.h>
#include <util.h>
#include <uart/uart.h>

#define SECTION_PROBEINFO __attribute__ ((section (".parameters")))

#undef _soc

#ifndef SECTION_SOC
#define SECTION_SOC      __attribute__ ((section (".sdata.soc_stru")))
#endif

#ifndef PLR_VERSION
#define PLR_VERSION 0x04000000
#endif

extern init_table_entry_t start_of_init_func_table, end_of_init_func_table;
extern symbol_table_entry_t start_of_symble_table, end_of_symble_table;

soc_t _soc SECTION_SOC = {
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
		.dcache_writeback_invalidate_all=&writeback_invalidate_dcache_all,
		.dcache_writeback_invalidate_range=&writeback_invalidate_dcache_range,
		.icache_invalidate_all=&invalidate_icache_all,
		.icache_invalidate_range=&invalidate_icache_range,
	},
};


const cg_dev_freq_t rtl9603cvd_cg_proj_freq SECTION_PARAMETERS = {
	.cpu0_mhz = 1050,
	.mem_mhz = 600,
	.lx_mhz = 200,
	.spif_mhz = 50,
	.sram_mhz = 250,
	.bypass_cg_init = 0, //don't touch registers; trust the given freq.
};

const cg_dev_freq_t apro_cg_ddr2_proj_freq SECTION_PARAMETERS = {
	.cpu1_mhz = 500,
	.mem_mhz  = 525,
	.lx_mhz   = 200,
	.spif_mhz = 100,
	.sram_mhz = 500
};

const cg_dev_freq_t apro_cg_ddr3_proj_freq SECTION_PARAMETERS = {
	.cpu1_mhz = 500,
	.mem_mhz  = 600,
	.lx_mhz   = 200,
	.spif_mhz = 100,
	.sram_mhz = 500
};

const cg_dev_freq_t apro_gen2_cg_ddr2_proj_freq SECTION_PARAMETERS = {
	.cpu1_mhz = 600,
	.mem_mhz  = 525,
	.lx_mhz   = 200,
	.spif_mhz = 100,
	.sram_mhz = 500
};

const cg_dev_freq_t apro_gen2_cg_ddr3_proj_freq SECTION_PARAMETERS = {
	.cpu1_mhz = 600,
	.mem_mhz  = 666,
	.lx_mhz   = 200,
	.spif_mhz = 50,
	.sram_mhz = 500
};

const uart_parameter_t uart_info = {
	.uid = 0,
	.baud = 115200,
};
