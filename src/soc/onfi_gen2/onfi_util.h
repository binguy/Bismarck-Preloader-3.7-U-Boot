#ifndef ONFI_UTIL_H
#define ONFI_UTIL_H


#include <soc.h>

#ifdef ONFI_USING_SYMBOL_TABLE_FUNCTION
    #ifdef NSU_DRIVER_IN_ROM
        #error 'lplr should not use symbol_table function'
    #endif
    #include "onfi_symb_func.h"
#else
    #include "onfi_common.h"
    #define ofu_reset_onfi_chip ofc_reset_nand_chip
    #define ofu_read_onfi_id    ofc_read_id
    #define ofu_status_read     ofc_status_read
    #define ofu_set_feature     ofc_set_feature
#endif


extern u32_t otto_plr_page_buf_addr;
#define page_buffer ((void*)(otto_plr_page_buf_addr))
#define oob_buffer   ((oob_t*)(OTTO_PLR_OOB_BUFFER))
#define ecc_buffer   ((void*)(OTTO_PLR_ECC_BUFFER))

typedef plr_oob_t oob_t;


s32_t onfi_util_init(void);
s32_t onfi_util_logical_page_read(void *data, u32_t page_num);
s32_t onfi_probe(onfi_info_t *info, const onfi_probe_t **probe_func_start, const onfi_probe_t **probe_func_end);



#endif //ONFI_UTIL_H

