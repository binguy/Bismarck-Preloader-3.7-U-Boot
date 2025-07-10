#include <soc.h>
#include <misc/misc_setting.h>

static void ACQ_SOC_CID_FROM_REG(void) {
	u32_t __val;

    RMOD_SCT(en, 0xb);
    __val = RFLD_SCT(sct);
    RMOD_SCT(en, 0);
	_soc.cid = (__val<<16);

	return;
}

SECTION_RECYCLE
static void chip_pre_init(void) {
	ACQ_SOC_CID_FROM_REG();

    apro_chip_pre_init();
    rtl9603cvd_chip_pre_init();
	return;
}

REG_INIT_FUNC(chip_pre_init, 2);
