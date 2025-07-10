// This file is used to store parameters for DDR, and/or flash
#include <util.h>
#define DONT_DECLAIRE__SOC_SYMBOLS
#include <uart/uart.h>
#include <plr_sections.h>
#include <symb_define.h>
#include <spi_nand/spi_nand_struct.h>

extern init_table_entry_t start_of_init_func_table, end_of_init_func_table;
extern symbol_table_entry_t start_of_symble_table, end_of_symble_table;
extern unsigned int lma_offset_before_mapped_area;
extern void plr_tlb_miss_isr(void);

#ifndef SECTION_SOC
#	define SECTION_SOC SECTION_SOC_STRU
#endif

spi_nand_cmd_info_t plr_cmd_info SECTION_SDATA;
spi_nand_model_info_t plr_model_info SECTION_SDATA;
spi_nand_flash_info_t plr_spi_nand_flash_info SECTION_SDATA = {
	._cmd_info   = &plr_cmd_info,
	._model_info = &plr_model_info,
};

soc_t _soc SECTION_SOC = {
	.bios={
		.header= {
			.signature=SIGNATURE_PLR_FL,
			.version=PLR_VERSION,
			.export_symb_list=&start_of_symble_table,
			.end_of_export_symb_list=&end_of_symble_table,
			.init_func_list=&start_of_init_func_table,
			.end_of_init_func_list=&end_of_init_func_table,
		},
		.isr=plr_tlb_miss_isr,
		.size_of_plr_load_firstly=(u32_t)&lma_offset_before_mapped_area,
		.uart_putc=VZERO,
		.uart_getc=VZERO,
		.uart_tstc=VZERO,
		.dcache_writeback_invalidate_all=VZERO,
		.icache_invalidate_all=VZERO,
	},
	.flash_info.spi_nand_info = &plr_spi_nand_flash_info,
	.cid = 0, //This filed will be updated to the SCT
	.sid = 0, //This field will be updated as SoC type
};

const cg_dev_freq_t rtl9603cvd_cg_proj_freq SECTION_PARAMETERS = {
	.cpu0_mhz = 1050,
	.mem_mhz  = 600,
	.lx_mhz   = 200,
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
	.spif_mhz = 100,
	.sram_mhz = 500
};

symb_idefine(boot_storage_type, SF_BOOT_STORAGE_TYPE, BOOT_FROM_SPI_NAND);

const uart_parameter_t uart_info SECTION_PARAMETERS = {
	.uid = 0,
	.baud = 115200,
};
