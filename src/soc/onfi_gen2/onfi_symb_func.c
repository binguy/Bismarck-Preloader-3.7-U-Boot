#include <soc.h>
#include <onfi/onfi_symb_func.h>
#include <symb_define.h>


#ifndef ALWAYS_RETURN_ZERO
    #define _onfi_dummy_func ALWAYS_RETURN_ZERO
#else
    s32_t _onfi_dummy_func(void) {return 0;}
#endif

u32_t *onfi_drv_ver_ptr SECTION_RECYCLE_DATA;
ecc_encode_t *_ecc_encode_ptr     SECTION_RECYCLE_DATA = (ecc_encode_t *)_onfi_dummy_func;
ecc_decode_t *_ecc_decode_ptr     SECTION_RECYCLE_DATA = (ecc_decode_t *)_onfi_dummy_func;
ecc_encode_t *_ecc_encode_4Kpage_ptr SECTION_RECYCLE_DATA = (ecc_encode_t *)_onfi_dummy_func;
ecc_decode_t *_ecc_decode_4Kpage_ptr SECTION_RECYCLE_DATA = (ecc_decode_t *)_onfi_dummy_func;
ecc_engine_t *_ecc_engine_act_ptr SECTION_RECYCLE_DATA = (ecc_engine_t *)_onfi_dummy_func;
fpv_t        *_ofu_reset_ptr      SECTION_RECYCLE_DATA = (fpv_t *)_onfi_dummy_func;
onfi_page_read_write_ecc_t *_ofu_page_read_ode_ptr SECTION_RECYCLE_DATA = (onfi_page_read_write_ecc_t *)_onfi_dummy_func;
onfi_page_read_write_ecc_t *_ofu_page_write_ode_ptr SECTION_RECYCLE_DATA = (onfi_page_read_write_ecc_t *)_onfi_dummy_func;

// Should globally exists
onfi_status_read_t *_ofu_status_read_ptr SECTION_SDATA = (onfi_status_read_t *)_onfi_dummy_func;
onfi_set_feature_t *_ofu_set_feature_ptr SECTION_SDATA = (onfi_set_feature_t *)_onfi_dummy_func;
onfi_get_feature_t *_ofu_get_feature_ptr SECTION_SDATA = (onfi_get_feature_t *)_onfi_dummy_func;
fps32_t            *_ofu_chk_program_erase_sts_ptr SECTION_SDATA = (fps32_t *)_onfi_dummy_func;
fpu32_t            *_ofu_read_onfi_id SECTION_SDATA = (fpu32_t *)_onfi_dummy_func;


symb_retrive_entry_t onfi_func_retrive_list[] SECTION_RECYCLE_DATA = {
    {ONAF_PIO_READ_FUNC,           &(onfi_plr_model_info._pio_read)},
    {ONAF_PIO_WRITE_FUNC,          &(onfi_plr_model_info._pio_write)},
    {ONAF_PAGE_READ_FUNC,          &(onfi_plr_model_info._page_read)},
    {ONAF_PAGE_WRITE_FUNC,         &(onfi_plr_model_info._page_write)},
    {ONAF_PAGE_READ_ECC_FUNC,      &(onfi_plr_model_info._page_read_ecc)},
    {ONAF_PAGE_WRITE_ECC_FUNC,     &(onfi_plr_model_info._page_write_ecc)},
    {ONAF_PAGE_READ_ODE_FUNC,      &_ofu_page_read_ode_ptr},
    {ONAF_PAGE_WRITE_ODE_FUNC,     &_ofu_page_write_ode_ptr},
    {ONFI_BLOCK_ERASE_FUNC,        &(onfi_plr_model_info._block_erase)},
    {ONFI_WAIT_NAND_CHIP_FUNC,     &(onfi_plr_model_info._wait_onfi_rdy)},
    {ONFI_RESET_NAND_CHIP_FUNC,    &_ofu_reset_ptr},
    {ONFI_STATUS_READ_FUNC,        &_ofu_status_read_ptr},
    {ONFI_SET_FEATURE_FUNC,        &_ofu_set_feature_ptr},
    {ONFI_GET_FEATURE_FUNC,        &_ofu_get_feature_ptr},
    {ONFI_CHK_PROG_ERASE_STS_FUNC, &_ofu_chk_program_erase_sts_ptr},
    {ONFI_READ_ID_FUNC,            &_ofu_read_onfi_id},
    {ECC_BCH_ENCODE_FUNC,          &_ecc_encode_ptr},
    {ECC_BCH_DECODE_FUNC,          &_ecc_decode_ptr},
    {ECC_BCH_ENCODE_4KPAGE_FUNC,   &_ecc_encode_4Kpage_ptr},
    {ECC_BCH_DECODE_4KPAGE_FUNC,   &_ecc_decode_4Kpage_ptr},
    {ECC_ENGINE_ACTION_FUNC,       &_ecc_engine_act_ptr},
    {ONFI_DRV_VER,                 &onfi_drv_ver_ptr},
    {ENDING_SYMB_ID, VZERO},
};

char _onfi_ver_fail_msg[] SECTION_RECYCLE_DATA = {"WW: ONFI version check failed!\n"};
char _onfi_ver_ok_msg[] SECTION_RECYCLE_DATA = {"II: ONFI version check OK!\n"};

SECTION_RECYCLE void 
onfi_func_symbol_retrive(void)
{
    symb_retrive_list(onfi_func_retrive_list, lplr_symb_list_range);

    if (!onfi_drv_ver_ptr&&(*onfi_drv_ver_ptr>SYM_ONFI_VER)) {
        puts(_onfi_ver_fail_msg);
        while(1);
    }
    puts(_onfi_ver_ok_msg);
}

REG_INIT_FUNC(onfi_func_symbol_retrive, 2);

