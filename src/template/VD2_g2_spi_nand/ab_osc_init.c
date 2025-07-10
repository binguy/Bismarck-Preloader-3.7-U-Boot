#include <misc/osc.h>

extern spi_nand_flash_info_t plr_spi_nand_flash_info SECTION_SDATA;

void osc_init(otto_soc_context_t *otto_sc) {
	memcpy((void *)&otto_sc->cg_mhz, (void *)&cg_info_query.dev_freq, sizeof(otto_sc->cg_mhz));
	otto_sc->snaf_info = &plr_spi_nand_flash_info;
	return;
}
