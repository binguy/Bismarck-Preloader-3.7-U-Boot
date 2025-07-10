#include <init_define.h>
#include <uart/uart.h>
#include <cg/cg.h>
#include <dram/memcntlr_reg.h>

extern const char tkinfo[];
char proj_name[] = name_of_project;
const char _banner_msg[] SECTION_RECYCLE_DATA = {"\n\nBismarck Preloader %u.%u \n"};
const char _apro_lplr_msg[] SECTION_RECYCLE_DATA = {"II: LPLR:%04x%x "};
const char _plr_msg[] SECTION_RECYCLE_DATA = {"PLR:%08x BD:%08x TK:%s\n"};
const char _lplr_info[] SECTION_RECYCLE_DATA = {"II: LPLR:%04x RV:%08x BD:%08x TK:%s\nII: "};
extern char *_lplr_vv SECTION_SDATA;
extern char *_lplr_bd SECTION_SDATA;
extern char *_lplr_tk SECTION_SDATA;


UTIL_FAR SECTION_UNS_TEXT void
puts(const char *s) {
	inline_puts(s);
}

SECTION_RECYCLE static void banner(void) {
	/* uart pin mux is set at ab_compatibility.c */
	/* Assume default LX freq. is 200MHz.
		 With this assumption, CG flow and cache ops. can be postponed and running on MMU,
		 and reduce SRAM usage.
	 */
	uart_init(uart_info.baud, OTTO_LX_RESET_DEFAULT_MHZ);
	_bios.uart_putc=uart_putc;
	_bios.uart_getc=uart_getc;
	_bios.uart_tstc=uart_tstc;

    printf(_banner_msg, (_soc.bios.header.version >> 24), (_soc.bios.header.version >> 8) & 0xff);

    if ((_soc.sid == PLR_SID_APRO) ||  (_soc.sid == PLR_SID_APRO_GEN2)) {
        printf(_apro_lplr_msg, (_lplr_soc_t.cid>>16),_soc.sid);
    } else {
        printf(_lplr_info, (_lplr_soc_t.cid>>16), _lplr_vv, _lplr_bd, _lplr_tk);
    }
    printf(_plr_msg, VCS_VER, MAKE_DATE, tkinfo);

	return;
}
REG_INIT_FUNC(banner, 3);

