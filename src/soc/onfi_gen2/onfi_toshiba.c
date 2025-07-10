#include <util.h>
#include <onfi/onfi_ctrl.h>
#include <onfi/onfi_common.h>
#include <onfi/onfi_util.h>

/****** ONFI ID Definition Table ******/
#define MID_TOSHIBA             (0x98)
#define MID_DID_TC58BVG0S3HTA00 (0x98F18015) //1Gb=(2048 +   64) bytes กั 64 pages กั 1024 blocks, 8bit ECC
#define MID_DID_TC58BVG1S3HTA00 (0x98DA9015) //2Gb=(2048 +   64) bytes กั 64 pages กั 2048 blocks, 8bit ECC
#define MID_DID_TC58BVG2S3HTAI0 (0x98DC9115) //4Gb=(2048 +   64) bytes กั 64 pages กั 4096 blocks, 8bit ECC
#define MID_DID_TC58BVG2S0HTA00 (0x98DC9026) //4Gb=(4096 + 128) bytes กั 64 pages กั 2048 blocks, 8bit ECC
#define MID_DID_TC58BVG3S0HTA00 (0x98D39126) //8Gb=(4096 + 128) bytes กั 64 pages กั 4096 blocks, 8bit ECC, Byte4=page_blk_size


/**** Chip Dependent Commmand ****/
#define CMD_READ_ECC_STATUS 0x7A


#ifdef ONFI_DRIVER_IN_ROM
    #include <arch.h>
    #define __SECTION_INIT_PHASE      SECTION_ONFI
    #define __SECTION_INIT_PHASE_DATA SECTION_ONFI_DATA
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
        #define __SECTION_INIT_PHASE_DATA   SECTION_RECYCLE_DATA
        #define __SECTION_RUNTIME           SECTION_UNS_TEXT
    #else
        #define __SECTION_INIT_PHASE
        #define __SECTION_INIT_PHASE_DATA
        #define __SECTION_RUNTIME
    #endif
#endif

void toshiba_onfi_ode_encode(u32_t ecc_ability, void *dma_addr, void *p_eccbuf);
s32_t toshiba_onfi_ode_decode(u32_t ecc_ability, void *dma_addr, void *p_eccbuf);
s32_t toshiba_onfi_ode_decode_4Kpage(u32_t ecc_ability, void *dma_addr, void *p_eccbuf);

__SECTION_INIT_PHASE_DATA
onfi_info_t toshiba_onfi_chip_info[] = {
    {
        .id_code             = MID_DID_TC58BVG0S3HTA00,
        ._num_block          = ONFI_MODEL_NUM_BLK_1024,
        ._num_page_per_block = ONFI_MODEL_NUM_PAGE_64,
        ._page_size          = ONFI_MODEL_PAGE_SIZE_2048B,
        ._spare_size         = ONFI_MODEL_SPARE_SIZE_64B,
        ._oob_size           = ONFI_MODEL_OOB_SIZE(24),
        ._ecc_ability        = ECC_USE_ODE,
        #ifdef ONFI_DRIVER_IN_ROM
        ._ecc_encode     = toshiba_onfi_ode_encode,
        ._ecc_decode     = toshiba_onfi_ode_decode,
        ._reset          = ofc_reset_nand_chip,
        ._model_info     = &onfi_ode_model,
        #else 
        ._ecc_encode     = VZERO,
        ._ecc_decode     = VZERO,
        ._reset          = VZERO,
        ._model_info     = VZERO,
        #endif
    },
    {
        .id_code             = MID_DID_TC58BVG1S3HTA00,
        ._num_block          = ONFI_MODEL_NUM_BLK_2048,
        ._num_page_per_block = ONFI_MODEL_NUM_PAGE_64,
        ._page_size          = ONFI_MODEL_PAGE_SIZE_2048B,
        ._spare_size         = ONFI_MODEL_SPARE_SIZE_64B,
        ._oob_size           = ONFI_MODEL_OOB_SIZE(24),
        ._ecc_ability        = ECC_USE_ODE,        
        #ifdef ONFI_DRIVER_IN_ROM
        ._ecc_encode     = toshiba_onfi_ode_encode,
        ._ecc_decode     = toshiba_onfi_ode_decode,
        ._reset          = ofc_reset_nand_chip,
        ._model_info     = &onfi_ode_model,
        #else 
        ._ecc_encode     = VZERO,
        ._ecc_decode     = VZERO,
        ._reset          = VZERO,
        ._model_info     = VZERO,
        #endif

    },
    {
        .id_code             = MID_DID_TC58BVG2S3HTAI0,
        ._num_block          = ONFI_MODEL_NUM_BLK_4096,
        ._num_page_per_block = ONFI_MODEL_NUM_PAGE_64,
        ._page_size          = ONFI_MODEL_PAGE_SIZE_2048B,
        ._spare_size         = ONFI_MODEL_SPARE_SIZE_64B,
        ._oob_size           = ONFI_MODEL_OOB_SIZE(24),
        ._ecc_ability        = ECC_USE_ODE,        
        #ifdef ONFI_DRIVER_IN_ROM
        ._ecc_encode     = toshiba_onfi_ode_encode,
        ._ecc_decode     = toshiba_onfi_ode_decode,
        ._reset          = ofc_reset_nand_chip,
        ._model_info     = &onfi_ode_model,
        #else 
        ._ecc_encode     = VZERO,
        ._ecc_decode     = VZERO,
        ._reset          = VZERO,
        ._model_info     = VZERO,
        #endif

    },
    {
        .id_code             = MID_DID_TC58BVG2S0HTA00,
        ._num_block          = ONFI_MODEL_NUM_BLK_2048,
        ._num_page_per_block = ONFI_MODEL_NUM_PAGE_64,
        ._page_size          = ONFI_MODEL_PAGE_SIZE_4096B,
        ._spare_size         = ONFI_MODEL_SPARE_SIZE_128B,
        ._oob_size           = ONFI_MODEL_OOB_SIZE(24),
        ._ecc_ability        = ECC_USE_ODE,        
        #ifdef ONFI_DRIVER_IN_ROM
        ._ecc_encode     = toshiba_onfi_ode_encode,
        ._ecc_decode     = toshiba_onfi_ode_decode,
        ._reset          = ofc_reset_nand_chip,
        ._model_info     = &onfi_ode_model,
        #else 
        ._ecc_encode     = VZERO,
        ._ecc_decode     = VZERO,
        ._reset          = VZERO,
        ._model_info     = VZERO,
        #endif
    },
    {
        .id_code             = MID_DID_TC58BVG3S0HTA00,
        ._num_block          = ONFI_MODEL_NUM_BLK_4096,
        ._num_page_per_block = ONFI_MODEL_NUM_PAGE_64,
        ._page_size          = ONFI_MODEL_PAGE_SIZE_4096B,
        ._spare_size         = ONFI_MODEL_SPARE_SIZE_128B,
        ._oob_size           = ONFI_MODEL_OOB_SIZE(24),
        ._ecc_ability        = ECC_USE_ODE,        
        #ifdef ONFI_DRIVER_IN_ROM
        ._ecc_encode     = toshiba_onfi_ode_decode_4Kpage,
        ._ecc_decode     = toshiba_onfi_ode_decode_4Kpage,
        ._reset          = ofc_reset_nand_chip,
        ._model_info     = &onfi_ode_model,
        #else 
        ._ecc_encode     = VZERO,
        ._ecc_decode     = VZERO,
        ._reset          = VZERO,
        ._model_info     = VZERO,
        #endif
    },
    {
        .id_code             = DEFAULT_DATA_BASE,
        ._num_block          = ONFI_MODEL_NUM_BLK_2048,
        ._num_page_per_block = ONFI_MODEL_NUM_PAGE_64,
        ._page_size          = ONFI_MODEL_PAGE_SIZE_2048B,
        ._spare_size         = ONFI_MODEL_SPARE_SIZE_64B,
        ._oob_size           = ONFI_MODEL_OOB_SIZE(24),
        ._ecc_ability        = ECC_USE_ODE,        
        #ifdef ONFI_DRIVER_IN_ROM
        ._ecc_encode     = ecc_encode_bch,
        ._ecc_decode     = ecc_decode_bch,
        ._reset          = ofc_reset_nand_chip,
        ._model_info     = &onfi_rom_general_model,
        #else        
        ._ecc_encode     = VZERO,
        ._ecc_decode     = VZERO,
        ._reset          = VZERO,
        ._model_info     = VZERO,
        #endif
    },
};

__SECTION_RUNTIME void 
toshiba_onfi_ode_encode(u32_t ecc_ability, void *dma_addr, void *p_eccbuf)
{
    return;
}

__SECTION_RUNTIME u32_t 
toshiba_ecc_sts_read(u32_t total_sec)
{
    typedef union{
        struct {
            u8_t sec_num:4;
            u8_t ecc_sts:4;
        } f;
        u8_t v;
    } ecc_max_err_report_t;
    ecc_max_err_report_t sector[8];
   
    NACMRrv = (CECS0|CMD_READ_ECC_STATUS);
    WAIT_ONFI_CTRL_READY();
    u32_t i;
    for(i=0;i<total_sec;i++){
        u32_t tmp = NADRrv;
        sector[i++].v = (tmp>>24)&0xFF;        
        sector[i++].v = (tmp>>16)&0xFF;
        sector[i++].v = (tmp>>8)&0xFF;
        sector[i].v =   (tmp>>0)&0xFF;
    }
    DIS_ONFI_CE0_CE1();

    u32_t uncorrectable = 0xF;
    ecc_max_err_report_t worst = {.v=0};
    for(i=0 ; i<total_sec ; i++){
        if(sector[i].f.ecc_sts == uncorrectable){
            return (ECC_CTRL_ERR|sector[i].f.sec_num);
        }
        if(sector[i].f.ecc_sts >= worst.f.ecc_sts){
            worst.f.ecc_sts = sector[i].f.ecc_sts;
            worst.f.sec_num = sector[i].f.sec_num;
        }
    }

    return worst.f.ecc_sts;
}


__SECTION_RUNTIME s32_t 
toshiba_onfi_ode_decode(u32_t ecc_ability, void *dma_addr, void *p_eccbuf)
{
    return toshiba_ecc_sts_read(2048/512);
}
__SECTION_RUNTIME s32_t 
toshiba_onfi_ode_decode_4Kpage(u32_t ecc_ability, void *dma_addr, void *p_eccbuf)
{
    return toshiba_ecc_sts_read(4096/512);
}

__SECTION_INIT_PHASE onfi_info_t *
probe_toshiba_onfi_chip(onfi_info_t *ret_info)
{
    u32_t readid = ofu_read_onfi_id();    
    if(MID_TOSHIBA == ((readid >>24)&0xFF)){
        u32_t i;   
        for(i=0 ; i<ELEMENT_OF_ONFI_INFO(toshiba_onfi_chip_info) ; i++){
            if((toshiba_onfi_chip_info[i].id_code == readid) || (toshiba_onfi_chip_info[i].id_code == DEFAULT_DATA_BASE)){                
                #ifdef ONFI_DRIVER_IN_ROM
                    ret_info = &toshiba_onfi_chip_info[i];
                #else
                    inline_memcpy(ret_info, &toshiba_onfi_chip_info[i],sizeof(onfi_info_t));
                    if(toshiba_onfi_chip_info[i]._num_block == ONFI_MODEL_NUM_BLK_4096){
                        ret_info->_ecc_decode = toshiba_onfi_ode_decode_4Kpage;
                    }else{
                        ret_info->_ecc_decode = toshiba_onfi_ode_decode;
                    }
                    ret_info->_ecc_encode = toshiba_onfi_ode_encode;
                    ret_info->_reset      = _ofu_reset_ptr;
                    ret_info->_model_info = &onfi_plr_model_info;
                    ret_info->_model_info->_page_read_ecc  = _ofu_page_read_ode_ptr;
                    ret_info->_model_info->_page_write_ecc = _ofu_page_write_ode_ptr;
                #endif
                return ret_info;
            }
        }
    }
    return VZERO;
}

REG_ONFI_PROBE_FUNC(probe_toshiba_onfi_chip);


