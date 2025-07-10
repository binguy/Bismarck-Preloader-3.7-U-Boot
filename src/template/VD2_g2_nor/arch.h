#include <cpu/cpu.h>

#ifndef __ASSEMBLER__
#define SECTION_CMD_NODE    __attribute__ ((section (".cli_node")))
#define SECTION_CMD_MP_NODE __attribute__ ((section (".cli_mp_node")))
#define SECTION_CMD_TAIL    __attribute__ ((section (".cli_tail")))
#define SECTION_AUTOK       __attribute__ ((section (".flash_text")))
#include <inline_util.h>

#define _soc (*(soc_t *)(OTTO_SRAM_START+OTTO_HEADER_OFFSET))
#include <bios_io.h>
#include <plr_sections.h>
#include <util.h>
#include <register_map.h>


// system parameters
#include <cg/cg.h>
#define SECTION_CG_CORE_INIT UTIL_FAR SECTION_UNS_TEXT
#if 0
#define SECTION_CG_MISC      SECTION_TEXT
#else //For cli_xmodem_nor, the NOR driver should always exist.
#define SECTION_CG_MISC      SECTION_SRAM_TEXT
#endif
#define SECTION_CG_MISC_DATA SECTION_RO

    extern const cg_info_t cg_info_proj SECTION_PARAMETERS;
    extern const u32_t uart_baud_rate SECTION_SDATA;

#define SECTION_UART SECTION_SRAM_TEXT


/* NOR SPI-F driver uses udelay. Putting it on flash causes MMIO/PIO conflicts. */
#define SECTION_SYS  SECTION_SRAM_TEXT

/* For SPI-F driver. */
#define SECTION_NOR_SPIF_PROBE_FUNC    __attribute__ ((section (".nor_spif_probe_func")))
#define SECTION_NOR_SPIF_GEN2_CORE     __attribute__ ((section (".sram_text"), noinline))
#define SECTION_NOR_SPIF_GEN2_COREDATA __attribute__ ((section (".data")))
#if 0
#define SECTION_NOR_SPIF_GEN2_MISC     __attribute__ ((section (".text")))
#define SECTION_NOR_SPIF_GEN2_PARAM    __attribute__ ((section (".parameters")))
#else //For cli_xmodem_nor, the NOR driver should always exist.
#define SECTION_NOR_SPIF_GEN2_MISC     __attribute__ ((section (".sram_text"), noinline))
#define SECTION_NOR_SPIF_GEN2_PARAM    __attribute__ ((section (".data")))
#endif


#define NORSF_CHIP_NUM     (1)
#define NORSF_MMIO_4B_EN   (0)
#define NORSF_XREAD_EN     (0)
#define NORSF_WBUF_LIM_B   (128)
#define NORSF_CFLASH_BASE  (0x94000000)
#define NORSF_UCFLASH_BASE (NORSF_CFLASH_BASE | 0x20000000)

#define NORSF_CNTLR_4B_ADDR(enable) do {   \
		RMOD_PIN_STS(spi_flash_4b_en, enable); \
	} while (0)

#define SYSTEM_RESET() do { \
    WDT_CTRLrv = 0x0; \
    WDT_CTRLrv = 0x80000000;\
} while(0)
#endif //!__ASSEMBLER

#define UBOOT_TEXT_BASE    (0x80f00000)
#define NEW_STACK_AT_DRAM  (UBOOT_TEXT_BASE-32)

#define SWITCH_4B_ADDR_MODE()

#define TIMER_FREQ_MHZ     (cg_query_freq(CG_DEV_LX))
#define TIMER_STEP_PER_US  (4)
#define TIMER1_BASE        (0xb8003210)
#define UDELAY_TIMER_BASE  TIMER1_BASE

// for cache operation function
/* LFUNC - declare local function */
#define LFUNC(symbol)     \
        .text;       \
        .align 4;    \
        .ent symbol; \
symbol:

/* FUNC - declare global function */
#define GFUNC(symbol)     \
        .text;         \
        .globl symbol; \
        .align 4;      \
        .ent symbol;   \
symbol:

/* FUNC - declare global function in SRAM*/
#define GSFUNC(symbol)     \
        .section .sram_text, "ax", @progbits;   \
        .globl symbol; \
        .align 4;      \
        .ent symbol;   \
symbol:

/* END - mark end of function */
#define END(symbol)       \
        .end symbol

