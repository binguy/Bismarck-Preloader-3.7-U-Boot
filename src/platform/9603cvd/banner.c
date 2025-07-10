#include <init_define.h>
#include <uart/uart.h>
#include <cg/cg.h>
#include <dram/memcntlr_reg.h>

extern const char tkinfo[];
char proj_name[] = name_of_project;
const char _banner_msg[] SECTION_RECYCLE_DATA = {"\n\nPreloader Bismarck%u.%u RV:%08x BD:%08x TK:%s\n"};
UTIL_FAR SECTION_UNS_TEXT void
puts(const char *s) {
	inline_puts(s);
}

SECTION_RECYCLE static void plr_banner(void) {
	/* uart pin mux is set at ab_compatibility.c */
#ifdef OTTO_PROJECT_FPGA
	uart_init(uart_info.baud, cg_proj_freq.lx_mhz);
#else
	/* Assume default LX freq. is 200MHz.
		 With this assumption, CG flow and cache ops. can be postponed and running on MMU,
		 and reduce SRAM usage.
	 */
	uart_init(uart_info.baud, OTTO_LX_RESET_DEFAULT_MHZ);
#endif
	_bios.uart_putc=uart_putc;
	_bios.uart_getc=uart_getc;
	_bios.uart_tstc=uart_tstc;

	printf(
		_banner_msg,
		(_soc.bios.header.version >> 24),
		(_soc.bios.header.version >> 8) & 0xff,
		VCS_VER, MAKE_DATE, tkinfo);

	return;
}
REG_INIT_FUNC(plr_banner, 3);
