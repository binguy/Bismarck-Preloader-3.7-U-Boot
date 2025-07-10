/*
 * arch.h for project/APro_spi_nand_demo
 */

#ifdef OTTO_PROJECT_FPGA
	#define KEEP_TIMEOUT_MONITOR
    #define BTG_LX0_BASE   (0)
    #define BTG_LX1_BASE   (0xB800A000)
    #define BTG_LX2_BASE   (0xB8018000)

    // BTG, SNAF, ECC & Concurrent test Address
    #define MT_DRAM_ADDR_V         (0xA0000000)
    #define MT_DRAM_SIZE           (0x01000000)
    #define BTG_LX0_ADDR_PHY       (0xA1000000)
    #define BTG_LX1_ADDR_PHY       (0xA1800000)
    #define BTG_LX2_ADDR_PHY       (0xA2000000)
    #define BTG_SIZE_W             (0x00800000-1)
    #define BTG_SIZE_R             (0x00800000-0x40)
#else
    #define BTG_LX0_BASE   (0xB8144000)
    #define BTG_LX1_BASE   (0xB800A000)
    #define BTG_LX2_BASE   (0xB814C000)
    #define BTG_LX3_BASE   (0xB8018000)

    #define PROJECT_WITH_PBO_LX_BUS
    #define BTG_LXUW_BASE  (0xB8148000)
    #define BTG_LXUR_BASE  (0xB8148000)
    #define BTG_LXDW_BASE  (0xB8148000)
    #define BTG_LXDR_BASE  (0xB8148000)

    // BTG, SNAF, ECC & Concurrent test Address
    #define MT_DRAM_ADDR_V         (0xA0000000)
    #define MT_DRAM_SIZE           (0x01000000)
    #define BTG_LX0_ADDR_PHY       (0xA1000000)
    #define BTG_LX1_ADDR_PHY       (0xA1800000)
    #define BTG_LX2_ADDR_PHY       (0xA2000000)
    #define BTG_LX3_ADDR_PHY       (0xA2800000)
    #define BTG_LXDS_ADDR_PHY      (0xA3000000)
    #define BTG_LXUS_ADDR_PHY      (0xA3800000)
    #define BTG_SIZE_W             (0x00800000-1)
    #define BTG_SIZE_R             (0x00800000-0x40)

    #define ENABLE_BTG_IP() ({\
        RFLD_IP_EN_CTRL(en_gdma0,1, en_gdma1,1); \
        RFLD_NEW_IP_EN_CTRL(en_gdma2,1, en_gdma3,1, en_gdma4,1, en_gdma5,1, en_gdma6,1, en_gdma7,1);\
    })
#endif

//For PLR-CLI's high memory test
#define USING_MEM_ZONE

// UBOOT relative definition
#define MAX_UBOOT_SIZE         (0x33000)
#define ECC_DECODED_UBOOT_ADDR (0x80700000)
#define NEW_STACK_AT_DRAM      (ECC_DECODED_UBOOT_ADDR-32)
#define UBOOT_DECOMP_ADDR      (0x80000000)

// SNAF & ECC Test relative definition
#define SIZE_5KB             (5*1024)
#define SNAF_SRC_CHUNK_BUF   (MT_DRAM_ADDR_V)
#define SNAF_CHK_CHUNK_BUF   (SNAF_SRC_CHUNK_BUF + SIZE_5KB)
#define SNAF_ECC_TAG_BUF     (SNAF_CHK_CHUNK_BUF + SIZE_5KB)
#define src_page_buf ((void*)(SNAF_SRC_CHUNK_BUF))
#define chk_page_buf ((void*)(SNAF_CHK_CHUNK_BUF))
#define mt_ecc_buf    ((void*)(SNAF_ECC_TAG_BUF))
#define SECTION_SPI_NAND_MT __attribute__ ((section (".pge_align")))
