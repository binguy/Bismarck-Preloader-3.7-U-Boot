/* Including tlzma.h to automatically enable LZMA */
#include <lib/lzma/tlzma.h>
#include <lib/lzma/LzmaDec.h>

u32_t util_ms_accumulator SECTION_SDATA = 0;
char *_lplr_vv SECTION_SDATA;
char *_lplr_bd SECTION_SDATA;
char *_lplr_tk SECTION_SDATA;


SECTION_RECYCLE void plr_funcptr_init(void) {
#ifdef HAS_LIB_LZMA
	_lzma_decode = LzmaDecode;
#endif
  
  //FIXME   
//     symb_retrive_and_set(lplr, symb_retrive_and_set(lp, _lplr_vv);
//     symb_retrive_and_set(lplr, symb_retrive_and_set(lp, _lplr_vv);
 //    symb_retrive_and_set(lplr, symb_retrive_and_set(lp, _lplr_vv);

 //   _lplr_vv
//#define symb_retrive_plr(key) symb_retrive(key, _soc_header.export_symb_list, _soc_header.end_of_export_symb_list);i


	return;
}
REG_INIT_FUNC(plr_funcptr_init, 5);

SECTION_RECYCLE void
timer_init(void) {
	otto_lx_timer_init(TIMER_FREQ_MHZ);
	return;
}
REG_INIT_FUNC(timer_init, 5);
