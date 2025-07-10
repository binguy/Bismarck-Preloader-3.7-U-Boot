#ifndef CLI_XMODEM_UPDATE_H
#define CLI_XMODEM_UPDATE_H
#include <soc.h>

#if defined (OTTO_FLASH_NOR)
#include <nor_spi/nor_spif_core.h>

#define SECTION_CLI_XMODEM_TEXT SECTION_UNS_TEXT
#define SECTION_CLI_XMODEM_RO   SECTION_DATA

#define ERASE_UNIT_B   (64*1024)
#define MAX_ERASE_BYTE (256*1024)

#define NEW_STACK_AT_SRAM (0x9F007800)

#else //#if defined (OTTO_FLASH_NOR)
#define SECTION_CLI_XMODEM_TEXT SECTION_UNS_TEXT
#define SECTION_CLI_XMODEM_RO   SECTION_UNS_RO
#define SECTION_CLI_XMODEM_DATA SECTION_DATA

#define MAX_ERASE_BLK_INDEX (3)

#define PAGE_SIZE_2K (2048)
#define PAGE_SIZE_4K (4096)

#if defined (OTTO_FLASH_SPI_NAND)
    #include <spi_nand/spi_nand_struct.h>
    #define cli_flash_info (_soc.flash_info.spi_nand_info)
#else
    #include <onfi/onfi_struct.h>
    #define cli_flash_info (_soc.flash_info.onfi_info)    
#endif

#define SIZE_1KB          (1024)
#define SIZE_4KB          (4*1024)
#define W_PAGE_BUF        (0x9f004000)
#define R_PAGE_BUF        (W_PAGE_BUF+SIZE_4KB+128+SIZE_1KB)
#define ECC_BUF           (R_PAGE_BUF+SIZE_4KB+256)
#define NEW_STACK_AT_SRAM (ECC_BUF+SIZE_4KB)

#define ecc_buf    ((char*)ECC_BUF)
#define w_buf_v    ((char*)W_PAGE_BUF)
#define r_buf_v    ((char*)R_PAGE_BUF)
#define w_buf_u32  ((u32_t*)w_buf_v)
#define r_buf_u32  ((u32_t*)r_buf_v)


#endif //#if defined (OTTO_FLASH_NOR)

extern void change_sp_to_dram(void);


#endif //CLI_XMODEM_UPDATE_H

