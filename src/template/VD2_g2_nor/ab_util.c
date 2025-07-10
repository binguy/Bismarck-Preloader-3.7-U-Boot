#include <init_define.h>
#include <uart/uart.h>
#include <util.h>
#include <lib/lzma/LzmaDec.h>
#include <lib/lzma/tlzma.h>


#define STACK_GUIDE     0xcafecafe

u32_t util_ms_accumulator SECTION_SDATA =0;
s8_t chip_ver[4] SECTION_SDATA = {0};

// message
char toolkitinfo[] SECTION_RECYCLE_DATA = TKINFO;
char proj_name[] SECTION_RECYCLE_DATA = name_of_project;
static char __msg_dis_ocp_to_monitor[] SECTION_RECYCLE_DATA = {"II: Disable ocp0 timeout monitor\n"};
static char __msg_dis_lx_to_monitor[] SECTION_RECYCLE_DATA = {"II: Disable lx0 timeout monitor\n"};
static char _banner_msg[] SECTION_RECYCLE_DATA = {"\n\n%s\nPRELOADER Bismarck %u.%u\n"};
static char _ver_msg[] SECTION_RECYCLE_DATA = {"II: PLR:%x, Build_date:%x, Toolkit:%s\n"};

UTIL_FAR SECTION_UNS_TEXT void
puts(const char *s) {
	inline_puts(s);
}

SECTION_RECYCLE static void
setting_timeout_monitor(void)
{
    //Disable OCP timeout monitor
    OCP_TO_CTRLrv = 0;
    puts(__msg_dis_ocp_to_monitor);

    //Disable LX timeout monitor
    LX0_M_TO_CTRLrv = 0;
    puts(__msg_dis_lx_to_monitor);

}

SECTION_RECYCLE
void plr_init_utility(void)
{
    //Step1. Binding from PLR symbol table
#ifdef HAS_LIB_LZMA
    _lzma_decode = LzmaDecode;
#endif

    //Step2. Set UART0 IO_Enable, Baud-rate, LX Timer
    UART_IO_EN();
	uart_init(uart_baud_rate, cg_info_proj.dev_freq.lx_mhz);
	_bios.uart_putc=uart_putc;
	_bios.uart_getc=uart_getc;
	_bios.uart_tstc=uart_tstc;

	//Step3. Read SoC ID & using printf showing the banner
    printf(_banner_msg, proj_name, (_soc.bios.header.version >> 24), (_soc.bios.header.version >> 8) & 0xff);
    printf(_ver_msg, VCS_VER, MAKE_DATE, toolkitinfo);

	//Step6. disable timeout monitor
	setting_timeout_monitor();
}

REG_INIT_FUNC(plr_init_utility, 1);
