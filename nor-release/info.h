#define project_name Phoebus_nor_demo
#define platform_name Phoebus
#define OTTO_LMA_BASE 0x9fc00000
#define OTTO_ENTRY 0x9fc00000
#define OTTO_ENTRY_SYMB plr_S_entry
#define OTTO_PLR_STACK_DEF OTTO_SRAM_START + OTTO_SRAM_SIZE - 8
#define MIN_STACK_SIZE 0xC00
#define TARGET_OUT release/plr.out
#define EXPORT_SYMB_PROTO_PRINTF 1
#define EXPORT_SYMB_CACHE_OP 1
#define AFTER_PRELOADER_IMG /home/gavin/AC1200-3/USDK/sdk/bismarck_V3.7_22101116_ab04910e/uboot_v2020.01/u-boot.pimg
#define AFTER_PRELOADER_IMG /home/gavin/AC1200-3/USDK/sdk/bismarck_V3.7_22101116_ab04910e/uboot_v2020.01/u-boot.pimg
#define AFTER_PRELOADER_IMGQ "/home/gavin/AC1200-3/USDK/sdk/bismarck_V3.7_22101116_ab04910e/uboot_v2020.01/u-boot.pimg"
#define SRAM_SIZE_SEG0 SRAM_256KB
#define SRAM_SIZE_SEG1 SRAM_0KB
#define SRAM_SIZE_SEG2 SRAM_0KB
#define SRAM_SIZE_SEG3 SRAM_0KB
#define FOR_UBOOT_2020 1
#define template_name Phoebus_nor
#define name_of_project "Phoebus"
#define PREFERRED_TK_PREFIX /share/rlx/msdk-4.8.5-1004k-EB-3.18-u0.9.33-m32t-150818/bin/msdk-linux-
#define OTTO_FLASH_NOR 1
#define FILTER_OUT_OBJS 
