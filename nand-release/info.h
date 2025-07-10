#define project_name Phoebus_spi_nand_demo
#define platform_name Phoebus
#define OTTO_ENTRY 0x9f000000
#define OTTO_LMA_BASE 0
#define OTTO_LDS ./src/lds/spi_nand_with_lplr.lds
#define OTTO_STARTUP ./src/template/Phoebus_spi_nand/startup_nand_with_lplr.S
#define MMU_PHY_SIZE 0x8000
#define MMU_VM_SIZE 0x40000
#define MMU_PHY_BASE_ADDR 0x9f000000
#define MMU_CHECK_INETRNAL_ERROR 1
#define USE_CC_GPOPT 1
#define CC_GPOPT -mgpopt -mlocal-sdata -mextern-sdata -G 4
#define IS_RECYCLE_SECTION_EXIST 
#define APRO_NSU_DRIVER_PATCH 
#define FOR_UBOOT_2020 1
#define template_name Phoebus_spi_nand
#define name_of_project "Phoebus"
#define PREFERRED_TK_PREFIX /share/rlx/msdk-4.8.5-1004k-EB-3.18-u0.9.33-m32t-150818/bin/msdk-linux-
#define OTTO_FLASH_SPI_NAND 1
#define NSU_USING_SYMBOL_TABLE_FUNCTION 
#define CONFIG_SPI_NAND_FLASH_INIT_FIRST 1
#define spi_nand_path src/platform/Phoebus/spi_nand
#define spi_nand_path_prefix src/platform/Phoebus/spi_nand/spi_nand_
#define FILTER_OUT_OBJS 
