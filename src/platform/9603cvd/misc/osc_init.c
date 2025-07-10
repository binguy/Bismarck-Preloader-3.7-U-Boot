#include <misc/osc.h>

void osc_init(otto_soc_context_t *otto_sc) {
	memcpy(
		(void *)&otto_sc->cg_mhz,
		(void *)&cg_info_query,
		sizeof(otto_sc->cg_mhz));
#if (_OSC_INCLUDE_SPI_NAND_INFO == 1)
	otto_sc->snaf_info = &plr_spi_nand_flash_info;
#endif
	return;
}
