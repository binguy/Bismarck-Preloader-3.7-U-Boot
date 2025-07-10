#include <util.h>
#include <onfi/onfi_ctrl.h>
#include <onfi/onfi_common.h>
#include <onfi/onfi_util.h>

/****** ONFI ID Definition Table ******/
#define MID_MXIC             (0xC2)
#define DID_1Gb  (0xF1)
#define DID_2Gb  (0xDA)
#define DID_4Gb  (0xDC)
#define DID_8Gb  (0xD3)

/**** Chip Dependent Commmand ****/
#define FEATURE_ADDR_BLK_PROTECT 0xA0

#ifdef ONFI_DRIVER_IN_ROM
    #include <arch.h>
    #define __SECTION_INIT_PHASE      SECTION_ONFI
    #define __SECTION_RUNTIME         SECTION_ONFI
    #ifdef IS_RECYCLE_SECTION_EXIST
        #error 'lplr should not have recycle section ...'
    #endif
#else
    #ifdef ONFI_USING_SYMBOL_TABLE_FUNCTION
        #define __DEVICE_REASSIGN 1
        #include <onfi/onfi_symb_func.h>   
    #endif
    #ifdef IS_RECYCLE_SECTION_EXIST
        #define __SECTION_INIT_PHASE        SECTION_RECYCLE
        #define __SECTION_RUNTIME           SECTION_UNS_TEXT
    #else
        #define __SECTION_INIT_PHASE
        #define __SECTION_RUNTIME
    #endif
#endif


__SECTION_INIT_PHASE  
void mxic_block_unprotect(void)
{
    ofu_set_feature(FEATURE_ADDR_BLK_PROTECT, 0);
}


__SECTION_INIT_PHASE onfi_info_t *
probe_mxic_onfi_chip(onfi_info_t *ret_info)
{
    u32_t readid = ofu_read_onfi_id();

    if(MID_MXIC == ((readid >>24)&0xFF)){
        ret_info->id_code = readid;

        u32_t did = (readid>>16)&0xFF;
        if(DID_8Gb == did){
            ret_info->_num_block = ONFI_MODEL_NUM_BLK_8192;
        }else if(DID_4Gb == did){
            ret_info->_num_block = ONFI_MODEL_NUM_BLK_4096;
        }else if(DID_2Gb == did){
            ret_info->_num_block = ONFI_MODEL_NUM_BLK_2048;
        }else{ //0xF1 & Others
            ret_info->_num_block = ONFI_MODEL_NUM_BLK_1024;
        }

        ret_info->_num_page_per_block = ONFI_MODEL_NUM_PAGE_64;
        ret_info->_page_size          = ONFI_MODEL_PAGE_SIZE_2048B;
        ret_info->_spare_size         = ONFI_MODEL_SPARE_SIZE_64B;
        ret_info->_oob_size           = ONFI_MODEL_OOB_SIZE(24);
        ret_info->_ecc_ability        = ECC_MODEL_6T;        
          
        #ifdef ONFI_DRIVER_IN_ROM
            ret_info->_ecc_encode     = ecc_encode_bch;
            ret_info->_ecc_decode     = ecc_decode_bch;
            ret_info->_reset          = ofc_reset_nand_chip;
            ret_info->_model_info     = &onfi_rom_general_model;
        #else
            ret_info->_ecc_encode     = _ecc_encode_ptr;
            ret_info->_ecc_decode     = _ecc_decode_ptr;
            ret_info->_reset          = _ofu_reset_ptr;
            ret_info->_model_info     = &onfi_plr_model_info;
        #endif

        mxic_block_unprotect();
        return ret_info;
    }
    return VZERO;
}

REG_ONFI_PROBE_FUNC(probe_mxic_onfi_chip);


