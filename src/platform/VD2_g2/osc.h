#ifndef __OTTO_SOC_CONTEXT_H__
#define __OTTO_SOC_CONTEXT_H__ 1

#include <cross_env.h>
#if defined(OTTO_FLASH_SPI_NAND) || defined(CONFIG_OTTO_SNAF)
#include <spi_nand/spi_nand_struct.h>
#endif
typedef struct {
	cg_dev_freq_t cg_mhz;
#if defined(OTTO_FLASH_SPI_NAND) || defined(CONFIG_OTTO_SNAF)
	spi_nand_flash_info_t *snaf_info;
#endif
} otto_soc_context_t;

extern otto_soc_context_t otto_sc;

void osc_init(otto_soc_context_t *);
#endif
