#ifndef _SWCORE_REG_H_
#define _SWCORE_REG_H_
#include <reg_skc35.h>

SKC35_REG_DEC(
	MODEL_NAME_INFO_9603cvd, 0xbb010000,
	RFIELD(31, 16, rtl_id);
	RFIELD(15, 11, model_char_1st);
	RFIELD(10, 6, model_char_2nd);
	RF_RSV(5, 5);
    RFIELD(4, 4, test_cut);
	RFIELD(3, 0, rtl_vid);
	);

SKC35_REG_DEC(
	IO_MODE_EN_9603cvd, 0xbb023014,
	RF_RSV(31, 21);
	RF_DNC(20, 7);
	RFIELD(6, 6, uart0_en);
	RF_DNC(5, 0);
	);
#endif
