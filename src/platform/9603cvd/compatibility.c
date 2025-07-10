#include <soc.h>
#include <swcore_reg.h>
#include <misc/misc_setting.h>

extern void apro_ctrl_ip_existence(void);
extern void ctrl_ip_existence(void);

SECTION_UNS_TEXT
int otto_is_apro(void) {
	return (RFLD(MODEL_NAME_INFO, rtl_id) == 0x9607);
}

static void ACQ_SOC_CID_FROM_REG(void) {
	u32_t __val;

	RMOD(SCT, en, 0xb);
	__val = RFLD(SCT, sct);
	RMOD(SCT, en, 0);

	_soc.cid = ((__val<<16)|(_soc.cid&0xFFFF));

	return;
}

SECTION_RECYCLE
static void chip_pre_init(void) {
	ACQ_SOC_CID_FROM_REG();

	if (otto_is_apro()) {
		_soc.sid = ((_lplr_soc_t.cid&0xffff)==0x6537)?1:0;
		apro_chip_pre_init();

		RMOD(IO_MODE_EN_apro, uart0_en, 1);
	} else {
		UART_IO_EN();
	}
	return;
}

REG_INIT_FUNC(chip_pre_init, 2);
