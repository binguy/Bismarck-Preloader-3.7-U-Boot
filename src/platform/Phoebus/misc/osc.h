#ifndef __OTTO_SOC_CONTEXT_H__
#define __OTTO_SOC_CONTEXT_H__ 1

#include <cross_env.h>

#if ( \
	(defined(OTTO_FLASH_SPI_NAND) && (OTTO_FLASH_SPI_NAND == 1)) || \
	(defined(CONFIG_CMD_NAND)))
#	define _OSC_INCLUDE_SPI_NAND_INFO (1)
#else
#	define _OSC_INCLUDE_SPI_NAND_INFO (0)
#endif

#if (_OSC_INCLUDE_SPI_NAND_INFO == 1)
#	include <spi_nand/spi_nand_struct.h>
extern spi_nand_flash_info_t plr_spi_nand_flash_info;
#endif

typedef struct {
	cg_dev_freq_t cg_mhz;
#if (_OSC_INCLUDE_SPI_NAND_INFO == 1)
	spi_nand_flash_info_t *snaf_info;
#endif
} otto_soc_context_t;

extern otto_soc_context_t otto_sc;

void osc_init(otto_soc_context_t *);
#endif
