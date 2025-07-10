// arch.h for 8388_nand (following 8685S_nand_spi_fpga)

// UBOOT relative definition
#define MAX_UBOOT_SIZE         (0x80000)
#define ECC_DECODED_UBOOT_ADDR (0x81C00000)
#define NEW_STACK_AT_DRAM      (ECC_DECODED_UBOOT_ADDR-32)
#define UBOOT_DECOMP_ADDR      (0x80000000)
