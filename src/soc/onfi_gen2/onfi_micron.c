#include <util.h>
#include <onfi/onfi_ctrl.h>
#include <onfi/onfi_common.h>
#include <onfi/onfi_util.h>

/****** ONFI ID Definition Table ******/
#define MID_MICRON            (0x2C)
#define RDID_MT29F1G08ABA     (0x2CF18095) //1Gb=(2048 +   64) bytes กั 64 pages กั 1024 blocks, ECC 4-bits
#define RDID_MT29F2G08ABA     (0x2CDA9095) //2Gb=(2048 +   64) bytes กั 64 pages กั 2048 blocks, ECC 8-bits, ODE
#define RDID_MT29F4G08ABADAWP (0x2CDC9095) //4Gb=(2048 +   64) bytes กั 64 pages กั 4096 blocks, 4-bit ECC per 528 bytes
#define DID_BYTE4_ODE_EN      (1<<7)


#ifdef ONFI_DRIVER_IN_ROM
    #include <arch.h>
    #define __SECTION_INIT_PHASE      SECTION_ONFI
    #define __SECTION_RUNTIME         SECTION_ONFI
    #ifdef IS_RECYCLE_SECTION_EXIST
        #error 'lplr should not have recycle section ...'
    #endif
#else
    #ifdef ONFI_USING_SYMBOL_TABLE_FUNCTION
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

__SECTION_RUNTIME void micron_onfi_ode_encode(u32_t ecc_ability, void *dma_addr, void *p_eccbuf)
{
    return;
}

__SECTION_RUNTIME s32_t
micron_onfi_ode_decode(u32_t ecc_ability, void *dma_addr, void *p_eccbuf)
{
    /* Status Register[4:3] = ECC status
         *    00 = Normal or uncorrectable
         *    01 = 4~6
         *    10 = 1~3
         *    11 = 7~8 (Rewrite recommended)
         *
         * Status Register[0] = 0 No Errors
         * Status Register[0] = 1 Uncorrectable
         */
    NACMRrv = (CECS0|CMD_READ_STATUS);
    WAIT_ONFI_CTRL_READY();
    u32_t status = NADRrv;
    DIS_ONFI_CE0_CE1();

    int ecc_sts = (status>>3)&0x3;
    if(ecc_sts==3){
        return 8;
    }else if(ecc_sts==2){
        return 3;
    }else if(ecc_sts==1){
        return 6;
    }else if((status&0x1)==1){
        return -1;
    }
    return 0;
}

__SECTION_INIT_PHASE
u32_t micron_rdid_byte4(void)
{
    NACMRrv = (CECS0|CMD_READ_ID);
    WAIT_ONFI_CTRL_READY();
    NAADRrv = 0x0;
    NAADRrv = (0x0|AD0EN);  // 1-dummy address byte
    WAIT_ONFI_CTRL_READY();

    u32_t id_chain __attribute__ ((unused)) = NADRrv;
    u32_t byte4 = NADRrv;

    //clear command/address register
    DIS_ONFI_CE0_CE1();
    NAADRrv = 0x0;
    return byte4;
}

__SECTION_INIT_PHASE onfi_info_t *
probe_micron_onfi_chip(onfi_info_t *ret_info)
{
    u32_t readid = ofu_read_onfi_id();
    u32_t array_operation;

    if(MID_MICRON == ((readid >>24)&0xFF)){
        u32_t byte4 = micron_rdid_byte4();

#ifdef ONFI_DRIVER_IN_ROM
        if(RDID_MT29F1G08ABA == readid){
            ret_info->_num_block   = ONFI_MODEL_NUM_BLK_1024;
            ret_info->_ecc_ability = ECC_MODEL_6T;
            ret_info->_ecc_encode  = ecc_encode_bch;
            ret_info->_ecc_decode  = ecc_decode_bch;
            ret_info->_model_info  = &onfi_rom_general_model;
        }else if((readid == RDID_MT29F2G08ABA)&&((DID_BYTE4_ODE_EN&byte4)==DID_BYTE4_ODE_EN)){
            ret_info->_num_block   = ONFI_MODEL_NUM_BLK_2048;
            ret_info->_ecc_ability = ECC_USE_ODE;
            ret_info->_ecc_encode  = micron_onfi_ode_encode;
            ret_info->_ecc_decode  = micron_onfi_ode_decode;
            ret_info->_model_info  = &onfi_ode_model;
        }else{
            return VZERO;
        }

        ret_info->id_code             = readid;
        ret_info->_num_page_per_block = ONFI_MODEL_NUM_PAGE_64;
        ret_info->_page_size          = ONFI_MODEL_PAGE_SIZE_2048B;
        ret_info->_spare_size         = ONFI_MODEL_SPARE_SIZE_64B;
        ret_info->_oob_size           = ONFI_MODEL_OOB_SIZE(24);
        ret_info->_ecc_ability        = ECC_MODEL_6T;
        ret_info->_reset              = ofc_reset_nand_chip;
        return ret_info;
#else
        if(RDID_MT29F1G08ABA == readid){
            ret_info->_num_block   = ONFI_MODEL_NUM_BLK_1024;
            ret_info->_ecc_ability = ECC_MODEL_6T;
            ret_info->_ecc_encode  = _ecc_encode_ptr;
            ret_info->_ecc_decode  = _ecc_decode_ptr;
            ret_info->_model_info  = &onfi_plr_model_info;
        }else if((readid == RDID_MT29F2G08ABA)&&((DID_BYTE4_ODE_EN&byte4)==DID_BYTE4_ODE_EN)){
            ret_info->_num_block   = ONFI_MODEL_NUM_BLK_2048;
            ret_info->_ecc_ability = ECC_USE_ODE;
            ret_info->_ecc_encode  = micron_onfi_ode_encode;
            ret_info->_ecc_decode  = micron_onfi_ode_decode;
            ret_info->_model_info  = &onfi_plr_model_info;
            ret_info->_model_info->_page_read_ecc  = _ofu_page_read_ode_ptr;
            ret_info->_model_info->_page_write_ecc = _ofu_page_write_ode_ptr;
        }else if(readid == RDID_MT29F4G08ABADAWP){
            /* Disable ODE, this model's ODE is located in spare space */
            array_operation = _ofu_get_feature_ptr(0x90) >> 24;
            _ofu_set_feature_ptr(0x90, (array_operation&(~(1<<3)))<<24);

            ret_info->_num_block   = ONFI_MODEL_NUM_BLK_4096;
            ret_info->_ecc_ability = ECC_MODEL_6T;
            ret_info->_ecc_encode  = _ecc_encode_ptr;
            ret_info->_ecc_decode  = _ecc_decode_ptr;
            ret_info->_model_info  = &onfi_plr_model_info;
        }else{
            return VZERO;
        }

        ret_info->id_code             = readid;
        ret_info->_num_page_per_block = ONFI_MODEL_NUM_PAGE_64;
        ret_info->_page_size          = ONFI_MODEL_PAGE_SIZE_2048B;
        ret_info->_spare_size         = ONFI_MODEL_SPARE_SIZE_64B;
        ret_info->_oob_size           = ONFI_MODEL_OOB_SIZE(24);
        ret_info->_reset              = _ofu_reset_ptr;
        return ret_info;
#endif
    }
    return VZERO;
}

REG_ONFI_PROBE_FUNC(probe_micron_onfi_chip);

