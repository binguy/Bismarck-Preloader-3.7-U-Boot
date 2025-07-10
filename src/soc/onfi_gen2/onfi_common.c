#include <util.h>
#include <onfi/onfi_ctrl.h>
#include <onfi/onfi_common.h>

SECTION_ONFI_DATA
u32_t onfi_drv_ver  = SYM_ONFI_VER;    // magic_1 + magic_2 + cntrl_ver + drv_ver


SECTION_ONFI 
s32_t ofc_read_id(void)
{
    NACMRrv = (CECS0|CMD_READ_ID);
    WAIT_ONFI_CTRL_READY();

    NAADRrv = 0x0;
    NAADRrv = (0x0|AD0EN);  // 1-dummy address byte
    WAIT_ONFI_CTRL_READY();

    u32_t id_chain = NADRrv;

    //clear command/address register
    DIS_ONFI_CE0_CE1();
    NAADRrv = 0x0;

    if(id_chain==0){
        //read flash fail.
        return -1;
    }
    return id_chain;
}

SECTION_ONFI 
void ofc_reset_nand_chip(void)
{
    NACMRrv = (CECS0|CMD_RESET);
}

SECTION_ONFI 
void ofc_set_feature(u32_t feature_addr, u32_t features)
{
    NACMRrv = (CECS0|CMD_FEATURE_SET_OP_ONFI);

    WAIT_ONFI_CTRL_READY();

    NAADRrv = 0x0;
    NAADRrv = (AD0EN|feature_addr);
    NADRrv  = features;
    WAIT_ONFI_CTRL_READY();

    DIS_ONFI_CE0_CE1();
    NAADRrv = 0x0;
}

SECTION_ONFI 
u32_t ofc_get_feature(u32_t feature_addr)
{
    NACMRrv = (CECS0|CMD_FEATURE_GET_OP_ONFI);

    WAIT_ONFI_CTRL_READY();

    NAADRrv = 0x0;
    NAADRrv = (AD0EN|feature_addr);
    u32_t features = NADRrv;
    WAIT_ONFI_CTRL_READY();

    DIS_ONFI_CE0_CE1();
    return features;
}


SECTION_ONFI 
u8_t ofc_status_read(void)
{
    DIS_ONFI_CE0_CE1();
    NACMRrv = (CECS0|CMD_READ_STATUS);
    WAIT_ONFI_CTRL_READY();

    u8_t status = RFLD_NADR(data3);
    DIS_ONFI_CE0_CE1();
    return status;
}

SECTION_ONFI  
void ofc_wait_nand_chip_ready(void)
{
    while((ofc_status_read()&0x60) != 0x60);
}

SECTION_ONFI 
s32_t ofc_check_program_erase_status(void)
{  
    return ((ofc_status_read() & 0x1)==0)?0:-1;
}

SECTION_ONFI  
s32_t ofc_block_erase(onfi_info_t *info, u32_t blk_page_id)
{
    int addr_cycle[5],page_shift;
    int real_page = blk_page_id;
    
    if ( real_page & (ONFI_NUM_OF_PAGE_PER_BLK(info)-1) ){
        return -1;
    }

    CLEAR_ONFI_CTRL_STS_REG();
    DIS_ONFI_CE0_CE1();

    //Command register , write erase command (1 cycle)
    NACMRrv = (CECS0|CMD_BLK_ERASE_C1);
    WAIT_ONFI_CTRL_READY();

    if(3 != info->bs_addr_cycle){
        for(page_shift=0; page_shift<3; page_shift++){
            addr_cycle[page_shift] = (real_page>>(8*page_shift)) & 0xff;
        }

        //NAND Flash Address Register1
        NAADRrv = ((~EN_NEXT_AD) & (AD2EN|AD1EN|AD0EN|(addr_cycle[0]<<CE_ADDR0) |(addr_cycle[1]<<CE_ADDR1)|(addr_cycle[2]<<CE_ADDR2)));        
    }else{
        addr_cycle[0] = 0;
        for(page_shift=0; page_shift<4; page_shift++){
            addr_cycle[page_shift+1] = (real_page>>(8*page_shift)) & 0xff;
        }
        NAADRrv = (((~EN_NEXT_AD) & AD2EN)|AD1EN|AD0EN|(addr_cycle[1]<<CE_ADDR0) |(addr_cycle[2]<<CE_ADDR1)|(addr_cycle[3]<<CE_ADDR2));
    }

    WAIT_ONFI_CTRL_READY();

    //write erase command cycle 2
    NACMRrv = (CECS0|CMD_BLK_ERASE_C2);
    WAIT_ONFI_CTRL_READY();

    info->_model_info->_wait_onfi_rdy();

    //read status
    return ofc_check_program_erase_status();
}


    
symb_fdefine(ONFI_READ_ID_FUNC, ofc_read_id);
symb_fdefine(ONFI_RESET_NAND_CHIP_FUNC, ofc_reset_nand_chip);
symb_fdefine(ONFI_BLOCK_ERASE_FUNC, ofc_block_erase);
symb_fdefine(ONFI_WAIT_NAND_CHIP_FUNC, ofc_wait_nand_chip_ready);
symb_fdefine(ONFI_STATUS_READ_FUNC, ofc_status_read);
symb_fdefine(ONFI_SET_FEATURE_FUNC, ofc_set_feature);
symb_fdefine(ONFI_GET_FEATURE_FUNC, ofc_get_feature);
symb_fdefine(ONFI_CHK_PROG_ERASE_STS_FUNC, ofc_check_program_erase_status);
symb_pdefine(onfi_drv_ver, ONFI_DRV_VER, &onfi_drv_ver);

