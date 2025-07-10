#include <util.h>
#include <onfi/onfi_ctrl.h>
#include <onfi/onfi_common.h>
#include <onfi/onfi_util.h>

/****** ONFI ID Definition Table ******/
#define MID_WINBOND             (0xEF)


/**** Chip Dependent Commmand ****/
#define CMD_READ_PARAMETER_PAGE (0xEC)


#ifdef ONFI_DRIVER_IN_ROM
    #include <arch.h>
    #define __SECTION_INIT_PHASE      SECTION_ONFI
    #ifdef IS_RECYCLE_SECTION_EXIST
        #error 'lplr should not have recycle section ...'
    #endif
#else
    #ifdef ONFI_USING_SYMBOL_TABLE_FUNCTION
        #include <onfi/onfi_symb_func.h>   
    #endif
    #ifdef IS_RECYCLE_SECTION_EXIST
        #define __SECTION_INIT_PHASE        SECTION_RECYCLE
    #else
        #define __SECTION_INIT_PHASE
    #endif
#endif

__SECTION_INIT_PHASE u32_t 
winbond_read_parameter(u32_t byte_off)
{
    NACMRrv = (CECS0|CMD_READ_PARAMETER_PAGE);
    WAIT_ONFI_CTRL_READY();

    NAADRrv = 0x0;
    NAADRrv = (0x0 |AD2EN|AD1EN|AD0EN);  // 1-dummy address byte
    WAIT_ONFI_CTRL_READY();

    s32_t cnt  = byte_off/4;
    u32_t parameter=0;
    while((cnt--)>=0){
        parameter = NADRrv;
    }

    return parameter;
}
    

__SECTION_INIT_PHASE onfi_info_t *
probe_winbond_onfi_chip(onfi_info_t *info)
{
    u32_t readid = ofu_read_onfi_id();

    if(MID_WINBOND == ((readid >>24)&0xFF)){
        info->id_code = readid;
        u32_t block_num = winbond_read_parameter(96)>>8;


        if(4096 == block_num){
            info->_num_block = ONFI_MODEL_NUM_BLK_4096;
        }else if(2048 == block_num){
            info->_num_block = ONFI_MODEL_NUM_BLK_2048;
        }else{
            info->_num_block = ONFI_MODEL_NUM_BLK_1024;
        }

        info->_num_page_per_block = ONFI_MODEL_NUM_PAGE_64;
        info->_page_size          = ONFI_MODEL_PAGE_SIZE_2048B;
        info->_spare_size         = ONFI_MODEL_SPARE_SIZE_64B;
        info->_oob_size           = ONFI_MODEL_OOB_SIZE(24);
        info->_ecc_ability        = ECC_MODEL_6T;

        #ifdef ONFI_DRIVER_IN_ROM
            info->_ecc_encode = ecc_encode_bch;
            info->_ecc_decode = ecc_decode_bch;
            info->_reset      = ofc_reset_nand_chip;
            inline_memcpy(info->_model_info, &onfi_rom_general_model, sizeof(onfi_model_info_t));
        #else
            info->_ecc_encode = _ecc_encode_ptr;
            info->_ecc_decode = _ecc_decode_ptr;
            info->_reset      = _ofu_reset_ptr;
            info->_model_info = &onfi_plr_model_info;
        #endif

        return info;
    }
    return VZERO;
}

REG_ONFI_PROBE_FUNC(probe_winbond_onfi_chip);


