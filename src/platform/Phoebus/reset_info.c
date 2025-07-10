#include <soc.h>

/* Due to rodata is placed in TLB, this func. must be called after TLB init. */
static void _plr_reset_info(void) {
	const char *rir_msg[] = {
		"power-on",
		"WDT",
		"command",
	};
	u32_t rir;

	if ( _soc.sid == PLR_SID_APRO ||  _soc.sid == PLR_SID_APRO_GEN2) {
		return;
	}

	rir = REG32(0xb8000700);
	printf("II: %s reset; info: %04x\n",
	       (rir_msg[rir >> 16]),
	       (rir & 0xffff));

	return;
}
REG_INIT_FUNC(_plr_reset_info, 13);
