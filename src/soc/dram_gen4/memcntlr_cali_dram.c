#include <soc.h>
#include <dram/memcntlr.h>
#include <util.h>
#include <cg/cg.h>
extern mc_info_t meminfo;


/*
 * Constant definition
 */

#define MEMCTL_CALI_RETRY_LIMILT            (5)
#define MEMCTL_CALI_MIN_READ_WINDOW         (7)
#define MEMCTL_CALI_MIN_WRITE_WINDOW        (7)
#define CPU_DCACHE_SIZE                     (0x8000)
#define MEMCTL_CALI_TARGET_LEN              (CPU_DCACHE_SIZE * 2)
#define MEMCTL_CALI_FULL_SCAN_RESOLUTION    (1)
#define MEMCTL_CALI_WRITE_DELAY_START_TAP   (0)
#define MEMCTL_CALI_READ_DELAY_START_TAP    (0)
#define MEMCTL_DATA_PATTERN_8BIT            (0x00FF00FF)
#define MEMCTL_DATA_PATTERN_16BIT           (0x0000FFFF)
#define MEMCTL_CALI_TARGET_ADDR             (0x80000000)
#define DMCR_MRS_BUSY                       (0x80000000)

#define _memctl_DCache_flush_invalidate      _soc.bios.dcache_writeback_invalidate_all

#define C_CALI  0
#define Y_CALI  1
#define CALIBRATION_TYPE C_CALI

u32_t 
pat_ary[] SECTION_RO = { 0x00010000, 0x01234567, 0x00000000, 0x76543210,
                         0xFFFFFFFF, 0x89abcdef, 0x0000FFFF, 0xfedcba98,
                         0xFFFF0000, 0x00FF00FF, 0xFF00FF00, 0xF0F0F0F0,
                         0x0F0F0F0F, 0x5A5AA5A5, 0xA5A55A5A, 0x5A5AA5A5,
                         0xA5A55A5A, 0xA5A55A5A, 0x5A5AA5A5, 0xA5A55A5A,
                         0x5A5AA5A5, 0x5555AAAA, 0xAAAA5555, 0x5555AAAA,
                         0xAAAA5555, 0xAAAA5555, 0x5555AAAA, 0xAAAA5555,
                         0x5555AAAA, 0xCC3333CC, 0x33CCCC33, 0xCCCC3333 };

MEMCNTLR_SECTION
void _memctl_delay_clkm_cycles(u32_t delay_cycles)
{
    volatile u32_t read_tmp __attribute__((unused));

    while(delay_cycles--){
        read_tmp = MCRrv;
    }

    return;
}

MEMCNTLR_SECTION
void _memctl_update_phy_param(void)
{
    DACCR_T daccr;
    volatile u32_t dmcr_tmp;

    /* Write DMCR register to sync the parameters to phy control. */
    dmcr_tmp = DMCRrv;
    DMCRrv = dmcr_tmp;
    _memctl_delay_clkm_cycles(10);
    /* Waiting for the completion of the update procedure. */

    while((RFLD_DMCR(dtr_up_busy_mrs_busy))!=0);

    __asm__ __volatile__("": : :"memory");

    /* reset phy buffer pointer */
    daccr.v = DACCRrv;
    //daccr_tmp1 = daccr_tmp1 & (0xFFFFFFEF);
    daccr.f.ac_bptr_clear=0;    
    DACCRrv = daccr.v ;

    _memctl_delay_clkm_cycles(10);
    __asm__ __volatile__("": : :"memory");
    //daccr_tmp2 = daccr_tmp1 | (0x10);
    daccr.f.ac_bptr_clear=1;
    DACCRrv = daccr.v ;

    return;
}


MEMCNTLR_SECTION
void memctl_sync_write_buf(void)
{
    MSRRrv = 0x80000000;
    while(RFLD_MSRR(flush_ocp_cmd)!=0);
    return;
}

MEMCNTLR_SECTION
void _write_pattern_1(u32_t start_addr, u32_t len)
{
    volatile u32_t *p_start, data_tmp __attribute__((unused));
    u32_t b_len;
    u32_t ary_i;


    /* In case of write through D-Cache mechanisim, read data in DCache */
    p_start = (volatile u32_t *)start_addr;
    for(b_len = 0; b_len < len; b_len += sizeof(u32_t)){
        data_tmp = *p_start;
    }

    /* Write data */
    p_start = (volatile u32_t *)start_addr;
    ary_i = 0;
    for(b_len = 0; b_len < len; b_len += sizeof(u32_t)){
        *p_start = pat_ary[ary_i];
        p_start++;
        ary_i = (ary_i+1) % (sizeof(pat_ary)/sizeof(u32_t));
    }

    _memctl_DCache_flush_invalidate();

    return;
}

MEMCNTLR_SECTION
u32_t _verify_pattern_1(u32_t start_addr, u32_t len)
{
    volatile u32_t *p_start, data_tmp;
    u32_t b_len, err_result;
    u32_t ary_i, pat_data;

    _memctl_DCache_flush_invalidate();

    err_result = 0;

    /* Read data */
    ary_i = 0;
    p_start = (volatile u32_t *)start_addr;
    for(b_len = 0; b_len < len; b_len += sizeof(u32_t)){
        data_tmp = *p_start;
        pat_data = pat_ary[ary_i];
        ary_i = (ary_i+1) % (sizeof(pat_ary)/sizeof(u32_t));
        err_result = err_result | ( (pat_data | data_tmp) & ( ~(pat_data & data_tmp)));
        if(err_result == 0xffffffff)
            return err_result;
        p_start++;
    }

    return err_result;
}

MEMCNTLR_SECTION
void _memctl_set_phy_delay_all(u32_t w_delay, u32_t r_delay)
{
    u32_t i_dq;
    volatile u32_t *ddcrdqr_base;

    ddcrdqr_base = (volatile u32_t *)0xb8001510;

    //printf("%s:%d: wdelay(%d), r_delay(%d)\n", __FUNCTION__, __LINE__, w_delay, r_delay);
    for(i_dq = 0; i_dq < 32; i_dq++){
        *ddcrdqr_base = (w_delay << 24) | (r_delay << 8);
        ddcrdqr_base++;
    }

    _memctl_update_phy_param();

    return ;
}

MEMCNTLR_SECTION    //FIXME
u32_t get_memory_memctl_dq_write_delay_parameters(u32_t *para_array)
{
    u32_t ret=0;
#if 0
    u32_t reg_val;
    if( (*para_array) < 32) {
        reg_val = *((&(DRAMI.static_cal_data_0)) + *para_array) >> DACDQR_DQR_PHASE_SHIFT_90_FD_S;
        if(0xff==reg_val)
            ret=0;
        else {
            *para_array = (reg_val & DACDQR_DQR_PHASE_SHIFT_90_MASK);
            ret = 1;
        }
    } else {
        ret = 0;
    }
#endif

    return ret;
}

MEMCNTLR_SECTION
u32_t plat_memctl_calculate_dqrf_delay(u32_t max_w_seq_start, u32_t max_w_len, u32_t max_r_seq_start, u32_t max_r_len)
{
    u32_t ret_val;

    ret_val = (((max_w_seq_start + (max_w_len/4)) & 0x1f) << 24) |
              (((max_r_seq_start + max_r_len - 1) & 0x1f) << 16) |
              (((max_r_seq_start + (max_r_len/2)) & 0x1f) << 8) |
              (((max_r_seq_start) & 0x1f) << 0);

    return ret_val;
}

MEMCNTLR_SECTION
void _memctl_set_phy_delay_dqrf(u32_t bit_loc,u32_t max_w_seq_start,u32_t max_w_len,u32_t max_r_seq_start,u32_t max_r_len)
{
    volatile u32_t *ddcrdqr_base;

    ddcrdqr_base =(volatile u32_t *)0xb8001510;

    ddcrdqr_base += bit_loc;

    if(get_memory_memctl_dq_write_delay_parameters(&bit_loc)){
        max_w_len = 0;
        max_w_seq_start = bit_loc;
    }
    CG_MEM_PLL_OE_DIS();
    *ddcrdqr_base = plat_memctl_calculate_dqrf_delay(max_w_seq_start, max_w_len, max_r_seq_start, max_r_len);
    CG_MEM_PLL_OE_EN();
    
    _memctl_update_phy_param();

    return;
}

//2014-10-23: Re-write the read delay window search algorithm for reducing stack size.
#if CALIBRATION_TYPE==Y_CALI
MEMCNTLR_SECTION
void _memctl_result_to_DQ_RW_Array(u32_t result, u32_t w_delay, u32_t r_delay, u32_t RW_array_addr[32][32])
{
    /*
     * RW_array_addr [32]   [32] : [Rising 0~15, Falling 0~15] [w_delay]
     *              32bit  W_delay
     */

    u32_t bit_loc, correct_bit;


    /* We mark correct bit */
    result = ~result;

    if((DCRrv & 0x0f000000) == 0){ /* 8bit mode */
        result = ((result & 0xFF000000) >> 24) | ((result & 0x00FF0000)) | ((result & 0x0000FF00) >> 8) | ((result & 0x000000FF) << 16);
    }else{ /* 16bit mode */
        result = ((result & 0xFFFF0000) >> 16) | ((result & 0x0000FFFF) << 16);
    }

    for(bit_loc=0; bit_loc < 32; bit_loc++){
        correct_bit = (result >> bit_loc) & 0x1;
        RW_array_addr[bit_loc][w_delay] |= (correct_bit << r_delay);
        //printf("correct_bit(%d), RW_array_addr[%d][%d] = 0x%08x, bit_loc(%d)\n", correct_bit, bit_loc, w_delay, RW_array_addr[bit_loc][w_delay], bit_loc);
    }

    return;
}

MEMCNTLR_SECTION
u32_t _memctl_find_proper_RW_dealy(u32_t resolution, u32_t w_start, u32_t r_start, u32_t DQ_RW_Array[32][32])
{
    u32_t max_r_seq_start, max_r_len, r_delay, r_seq_start, r_len;
    u32_t max_w_seq_start, max_w_len, w_delay, w_seq_start, w_len, search_seq_start, bit_loc;
    u32_t bit_fail, wr_bit_fail, mode_16bit;


    if(DCRrv & 0x0F000000){
        mode_16bit = 1;
    }else{
        mode_16bit = 0;
    }

    bit_fail = wr_bit_fail = 0;
    for(bit_loc = 0; bit_loc < 32; bit_loc++){
        if(mode_16bit == 0){
            if((bit_loc > 7) && (bit_loc < 16))
                    continue;
            if((bit_loc > 23) && (bit_loc < 32))
                    continue;
        }

        max_r_len = 0;
        max_r_seq_start = 0;
        max_w_len = 0;
        max_w_seq_start = 0;
        //printf("bit(%d):\n", bit_loc);
        /* Searching for the max. sequetial read window. */
        for(w_delay = w_start; w_delay < 32; w_delay+=resolution){
            r_len = 0;
            r_seq_start = 0;
            search_seq_start = 1;
            //printf("   w(%d) %08x\n", w_delay, DQ_RW_Array[bit_loc][w_delay]);
            for(r_delay = r_start; r_delay < 32; r_delay+=resolution){
                if(search_seq_start == 1){
                    if( (DQ_RW_Array[bit_loc][w_delay] >> r_delay) & 0x1 ){
                        r_seq_start = r_delay;
                        search_seq_start = 0;
                    }
                    if( (r_delay+resolution) >= 31 ){
                        r_len = 1;
                        if(r_len > max_r_len){
                            max_r_len = r_len;
                            max_r_seq_start = r_seq_start;
                            r_len = 0;
                            r_seq_start = r_delay + resolution;
                        }
                    }
                }else{
                    if( 0 == ((DQ_RW_Array[bit_loc][w_delay] >> r_delay) & 0x1) ){
                        r_len = r_delay - r_seq_start - resolution + 1;
                        if(r_len > max_r_len){
                                max_r_len = r_len;
                                max_r_seq_start = r_seq_start;
                                r_len = 0;
                                r_seq_start = r_delay + resolution;
                        }
                        search_seq_start = 1;
                    }else{
                        if((r_delay+resolution)  >= 31){
                            r_len = r_delay - r_seq_start + 1;
                            if(r_len > max_r_len){
                                max_r_len = r_len;
                                max_r_seq_start = r_seq_start;
                                r_len = 0;
                                r_seq_start = r_delay + resolution;
                            }
                        }
                    }
                }
            }
        }


        w_len = 0;
        w_seq_start = 0;
        search_seq_start = 1;
        /* Searching for the max. write delay window basing on max. read delay window. */
        for(r_delay = max_r_seq_start ; r_delay < (max_r_seq_start + max_r_len) ; r_delay += resolution){
            w_len = 0;
            w_seq_start = 0;
            search_seq_start = 1;
            for(w_delay = w_start; w_delay < 32; w_delay+=resolution){
                if(search_seq_start == 1){
                    if( (DQ_RW_Array[bit_loc][w_delay] >> r_delay) & 0x1 ){
                        w_seq_start = w_delay;
                        search_seq_start = 0;
                    }
                    if( (w_delay+resolution) >= 31 ){
                        w_len = 1;
                        if(w_len > max_w_len){
                            max_w_len = w_len;
                            max_w_seq_start = w_seq_start;
                            w_len = 0;
                            w_seq_start = w_delay + resolution;
                        }
                    }
                }else{
                    if( 0 == ((DQ_RW_Array[bit_loc][w_delay] >> r_delay) & 0x1) ){
                        w_len = w_delay - w_seq_start - resolution + 1;
                        if(w_len > max_w_len){
                            max_w_len = w_len;
                            max_w_seq_start = w_seq_start;
                            w_len = 0;
                            w_seq_start = w_delay + resolution;
                        }
                        search_seq_start = 1;
                    }else{
                        if((w_delay+resolution)  >= 31){
                            w_len = w_delay - w_seq_start + 1;
                            if(w_len > max_w_len){
                                max_w_len = w_len;
                                max_w_seq_start = w_seq_start;
                                w_len = 0;
                                w_seq_start = w_delay + resolution;
                            }
                        }
                    }
                }
            }
        }

        //printf("DD: bit[%02d], max_r_s(%d), max_r_l(%d), max_w_s(%d), max_w_len(%d)\n", bit_loc, max_r_seq_start, max_r_len,  max_w_seq_start, max_w_len);
        _memctl_set_phy_delay_dqrf(bit_loc, max_w_seq_start, max_w_len, max_r_seq_start, max_r_len);

        //if((max_w_len <= MEMCTL_CALI_MIN_WRITE_WINDOW) || (max_r_len <= MEMCTL_CALI_MIN_READ_WINDOW)){
        if((max_w_len <= MEMCTL_CALI_MIN_WRITE_WINDOW) ){
            //printf("\nII: small max_w_len=%d, bit_loc=%d\n", max_w_len, bit_loc);
            wr_bit_fail |= (1 << bit_loc);
        }
        if(max_r_len <= MEMCTL_CALI_MIN_READ_WINDOW){
            //printf("\nII: small max_r_len=%d, bit_loc=%d\n", max_r_len, bit_loc);
            bit_fail |= (1 << bit_loc);
        }
    }

	if(0xff00ff00==bit_fail) {
        //printf("II: this might be 8-bit\n");
        bit_fail=0x0;	
    } 
    if((0!=bit_fail)||(0!=wr_bit_fail)){
        printf("EE: calibration RD-0x%08x, WR-0x%08x\n", bit_fail, wr_bit_fail);
    }
    
    return bit_fail;
}

MEMCNTLR_SECTION
u32_t _DDR_Calibration_Full_Scan(u32_t target_addr, u32_t len, u32_t resolution, u32_t w_start, u32_t r_start)
{
    u32_t w_delay, r_delay, WR_Result;
    u32_t DQ_RW_Array[32][32];

    //printf("DQ_RW_Array: 0x%08p\n", &DQ_RW_Array[0][0]);

    /* Initialize DQ_RW_Array */
    for(w_delay = 0; w_delay < 32; w_delay++){
            for(r_delay = 0; r_delay < 32; r_delay++){
                    DQ_RW_Array[w_delay][r_delay] = 0;
            }
    }


    /* Fully scan whole delay tap. */
    for(w_delay = w_start; w_delay < 32; w_delay += resolution){
        for(r_delay = r_start; r_delay < 32; r_delay += resolution){
            _memctl_set_phy_delay_all(w_delay, r_delay);

            _write_pattern_1(target_addr, len);
            memctl_sync_write_buf();
            WR_Result = _verify_pattern_1(target_addr, len);

            _memctl_result_to_DQ_RW_Array(WR_Result, w_delay, r_delay, DQ_RW_Array);
        }
    }

#if 0
    /* Resolution post processing. */
    if(resolution > 1){
            _memctl_dq_rw_array_post_processing(w_start, r_start, resolution, &DQ_RW_Array[0][0]);
    }
#endif

    /* All scan result is in DQ_RW_Array, choose a proper delay tap setting. */
    if( 0 == _memctl_find_proper_RW_dealy( resolution, w_start, r_start, DQ_RW_Array)){
        return 0;/* Pass */
    }else{
        return 1;/* Fali */
    }
}
#else

MEMCNTLR_SECTION
static u32_t 
__DDR_Calibration_Full_Scan_Window(u32_t target_addr, 
                                   u32_t len,
                                   u32_t resolution,
                                   u32_t w_start,
                                   u32_t r_start)
{
    u8_t max_r_seq_start_array[32], max_r_len_array[32];
    u8_t max_w_seq_start_array[32], max_w_len_array[32];
    u8_t seq_start_array[32];
    u8_t search_seq_start_array[32];
    u8_t w_delay, r_delay, bit_loc;
    u8_t is_this_bit_correct;
    u8_t mode_16bit;
    u8_t max_r_seq_start=0, max_r_len=0;
    u8_t max_w_seq_start=0, max_w_len=0;
    u8_t r_seq_start, r_len;
    u8_t w_seq_start, w_len;
    u8_t search_seq_start;
    u32_t WR_Result;

    if(DCRrv & 0x0F000000){
        mode_16bit = 1;
    }else{
        mode_16bit = 0;
    }

    /**************************************************************
     ************ Searching for the max. sequetial read window.***********
     **************************************************************/
    for(bit_loc = 0; bit_loc < 32; bit_loc++){
        max_r_seq_start_array[bit_loc] = 0;
        max_r_len_array[bit_loc]       = 0;
    }

    for(w_delay = w_start; w_delay < 1; w_delay += resolution){
        //For each w_delay, it is a new search for each w_delay, so that reset the relative information to initial state.
        for(bit_loc = 0; bit_loc < 32; bit_loc++){
            seq_start_array[bit_loc]       = 0;
            search_seq_start_array[bit_loc]= 1;
        }

        for(r_delay = r_start; r_delay < 32; r_delay += resolution){
            _memctl_set_phy_delay_all(w_delay, r_delay);
            _write_pattern_1(target_addr, len);
            memctl_sync_write_buf();
            /* We mark correct bit */
            WR_Result = ~(_verify_pattern_1(target_addr, len));

            if( mode_16bit == 0){ /* 8bit mode */
                WR_Result = ((WR_Result & 0xFF000000) >> 24) | ((WR_Result & 0x00FF0000)) | ((WR_Result & 0x0000FF00) >> 8) | ((WR_Result & 0x000000FF) << 16);
            }else{ /* 16bit mode */
                WR_Result = ((WR_Result & 0xFFFF0000) >> 16) | ((WR_Result & 0x0000FFFF) << 16);
            }

            for(bit_loc = 0; bit_loc < 32; bit_loc++){
                if(mode_16bit == 0){
                    if((bit_loc > 7) && (bit_loc < 16)) continue;
                    if((bit_loc > 23) && (bit_loc < 32)) continue;
                }
        
                max_r_seq_start  = max_r_seq_start_array[bit_loc];
                max_r_len        = max_r_len_array[bit_loc];
                r_seq_start      = seq_start_array[bit_loc];
                search_seq_start = search_seq_start_array[bit_loc];
                is_this_bit_correct = ((WR_Result>>bit_loc) & 0x1);
        
                if(search_seq_start == 1){
                    if(is_this_bit_correct == 1){
                        r_seq_start = r_delay;
                        search_seq_start = 0;
                    }
                    if( (r_delay+resolution) >= 31 ){
                        r_len = 1;
                        if(r_len > max_r_len){
                            max_r_len = r_len;
                            max_r_seq_start = r_seq_start;
                            r_len = 0;
                            r_seq_start = r_delay + resolution;
                        }
                    }
                }else{
                    if(is_this_bit_correct == 0){
                        r_len = r_delay - r_seq_start - resolution + 1;
                        if(r_len > max_r_len){
                            max_r_len = r_len;
                            max_r_seq_start = r_seq_start;
                            r_len = 0;
                            r_seq_start = r_delay + resolution;
                        }
                        search_seq_start = 1;
                    }else{
                        if((r_delay+resolution) >= 31){
                            r_len = r_delay - r_seq_start + 1;
                            if(r_len > max_r_len){
                                max_r_len = r_len;
                                max_r_seq_start = r_seq_start;
                                r_len = 0;
                                r_seq_start = r_delay + resolution;
                            }
                        }
                    }
                }
                if((max_r_len >= max_r_len_array[bit_loc])&&(max_r_seq_start >= max_r_seq_start_array[bit_loc])){
                    max_r_seq_start_array[bit_loc] = max_r_seq_start;
                    max_r_len_array[bit_loc]       = max_r_len;
                }
                seq_start_array[bit_loc]     = r_seq_start;
                search_seq_start_array[bit_loc]= search_seq_start;
            }
        }
    }
                    
    /******************************************************************
    Searching for the max. write delay window basing on max. read delay window.
    ******************************************************************/
    for(bit_loc = 0; bit_loc < 32; bit_loc++){
        max_w_seq_start_array[bit_loc] = 0;
        max_w_len_array[bit_loc]       = 0;
    }

    for(r_delay = max_r_seq_start ; r_delay < (max_r_seq_start + max_r_len) ; r_delay += resolution){
        //For each r_delay, it is a new search for each r_delay, so that reset the relative information to initial state.
        for(bit_loc = 0; bit_loc < 32; bit_loc++){
            seq_start_array[bit_loc]       = 0;
            search_seq_start_array[bit_loc]= 1;
        }
                    
        for(w_delay = w_start; w_delay < 32; w_delay+=resolution){
            _memctl_set_phy_delay_all(w_delay, r_delay);
            _write_pattern_1(target_addr, len);
            memctl_sync_write_buf();
            /* We mark correct bit */
            WR_Result = ~(_verify_pattern_1(target_addr, len));
                    
            if( mode_16bit == 0){ /* 8bit mode */
                WR_Result = ((WR_Result & 0xFF000000) >> 24) | ((WR_Result & 0x00FF0000)) | ((WR_Result & 0x0000FF00) >> 8) | ((WR_Result & 0x000000FF) << 16);
            }else{ /* 16bit mode */
                WR_Result = ((WR_Result & 0xFFFF0000) >> 16) | ((WR_Result & 0x0000FFFF) << 16);
            }
            for(bit_loc = 0; bit_loc < 32; bit_loc++){
                if(mode_16bit == 0){
                    if((bit_loc > 7) && (bit_loc < 16)) continue;
                    if((bit_loc > 23) && (bit_loc < 32)) continue;
                }
                    
                max_w_seq_start  = max_w_seq_start_array[bit_loc];
                max_w_len        = max_w_len_array[bit_loc];
                w_seq_start      = seq_start_array[bit_loc];
                search_seq_start = search_seq_start_array[bit_loc];
                is_this_bit_correct = ((WR_Result>>bit_loc) & 0x1);
                                
                if(search_seq_start == 1){
                    if(is_this_bit_correct == 1){
                        w_seq_start = w_delay;
                        search_seq_start = 0;
                    }
                    if( (w_delay+resolution) >= 31 ){
                        w_len = 1;
                        if(w_len > max_w_len){
                            max_w_len = w_len;
                            max_w_seq_start = w_seq_start;
                            w_len = 0;
                            w_seq_start = w_delay + resolution;
                        }
                    }
                }else{
                    if(is_this_bit_correct == 0){
                        w_len = w_delay - w_seq_start - resolution + 1;
                        if(w_len > max_w_len){
                            max_w_len = w_len;
                            max_w_seq_start = w_seq_start;
                            w_len = 0;
                            w_seq_start = w_delay + resolution;
                        }
                        search_seq_start = 1;
                    }else{
                        if((w_delay+resolution) >= 31){
                            w_len = w_delay - w_seq_start + 1;
                            if(w_len > max_w_len){
                                max_w_len = w_len;
                                max_w_seq_start = w_seq_start;
                                w_len = 0;
                                w_seq_start = w_delay + resolution;
                            }
                        }
                    }
                }
                if((max_w_len >= max_w_len_array[bit_loc])&&(max_w_seq_start >= max_w_seq_start_array[bit_loc])){
                    max_w_seq_start_array[bit_loc] = max_w_seq_start;
                    max_w_len_array[bit_loc]       = max_w_len;
                }
                seq_start_array[bit_loc]     = w_seq_start;
                search_seq_start_array[bit_loc]= search_seq_start;
            }
        }
    }
                                    
    for(bit_loc = 0; bit_loc < 32; bit_loc++){
        if (1==meminfo.cntlr_opt->dbg_en) { 
            printf("DD: bit[%02d], max_w_s(%d), max_w_l(%d), max_r_s(%d), max_r_len(%d)\n",
                    bit_loc, max_w_seq_start_array[bit_loc], max_w_len_array[bit_loc], max_r_seq_start_array[bit_loc], max_r_len_array[bit_loc]);
        }
        //write back the calibrated delay taps
        //_memctl_set_phy_delay_dqrf(bit_loc, 0, 0, max_r_seq_start_array[bit_loc], max_r_len_array[bit_loc]);
        _memctl_set_phy_delay_dqrf(bit_loc,
                                   0,//max_w_seq_start_array[bit_loc],
                                   0,//max_w_len_array[bit_loc],
                                   max_r_seq_start_array[bit_loc],
                                   max_r_len_array[bit_loc]);
    }
    return 0;
}                                
#endif

#if 0
MEMCNTLR_SECTION
void _memctl_disable_hw_auto_cali(void)
{
    u32_t i;
    volatile u32_t *dacdqr, *dacspcr;

    dacspcr = (volatile u32_t *)0xb8001504;
    dacdqr = (volatile u32_t *)0xb8001510;

    *dacspcr &= (~(1<<31));

    for(i=0;i<32;i++){
        *dacdqr &= (~(1<<31));
        dacdqr++;
    }

    _memctl_update_phy_param();

    return;
}

MEMCNTLR_SECTION
void _memctl_enable_hw_auto_cali(void)
{
    u32_t i, sil_addr, sil_pat, c_max, c_min, cur_tap;
    u32_t all_dq_c_max, all_dq_c_min;
    volatile u32_t *dacspcr, *dacdqr, *d3zqccr;
    volatile u32_t *dacspar, *pdata;

    /* 0. Set Register parameters */
    dacspcr = (volatile u32_t *)0xb8001504;
    dacdqr  = (volatile u32_t *)0xb8001510;
    dacspar = (volatile u32_t *)0xb8001508;
    d3zqccr = (volatile u32_t *)0xb8001080;
    
    /* 1. Silence pattern address */
    sil_addr = DACSPARrv;
    if(((sil_addr >= 0x80000000) && (sil_addr < 0x90000000)) 
    || ((sil_addr >= 0xa0000000) && (sil_addr < 0xb0000000))){
    }else{
        sil_addr = 0xa2000000;
    }
    *dacspar = sil_addr;

    /* 2. Silence pattern */
    /* Pattern 1: 0x5555aaaa/16bit 0x55aa55aa/8bit  
       Pattern 2: 0xffff0000/16bit 0xff00ff00/8bit */
    if(DCRrv & 0x01000000) {
        //sil_pat = 0x5555aaaa;
        sil_pat = 0xffff0000;
    }else{
        //sil_pat = 0x55aa55aa;
        sil_pat = 0xff00ff00;
    }
    
    /* 3. Write silence pattern to DRAM */
    /* DDR2/8bit:  burst=4, data width=8bit  --> 4*1=4bytes 
       DDR2/16bit: burst=4, data width=16bit --> 4*2=8bytes 
       DDR3/8bit:  burst=8, data width=8bit  --> 8*1=8bytes 
       DDR3/16bit: burst=8, data width=16bit --> 8*2=16bytes
       We write slient pattern for 16bytes for covering above cases  */
    pdata = (volatile u32_t *)(sil_addr | 0x20000000);
    for(i=0;i<4;i++){
        *pdata = sil_pat;
        pdata++;
    }

    /* 4. Enable short ZQ */
//    if(memctlc_is_DDR3()){        //FIXME
        *d3zqccr |= (1<<30);
//    }else{
//        *d3zqccr &= (~(1<<30));
//    }

    /* 5. Configure and enable all DQ auto calibration */
    all_dq_c_max = 10;/* The value is obtained by experiments */
    all_dq_c_min = 10;/* The value is obtained by experiments */

    for(i=0;i<32;i++){
        /* extract current */
        cur_tap = (*dacdqr & 0x0000FF00) >> 8;
        if((cur_tap+all_dq_c_max)>31){
            c_max = 31 << 16;
        }else{
            c_max = ((cur_tap+all_dq_c_max)<<16);
        }
        if(all_dq_c_min > cur_tap){
            c_min = 0;
        }else{
            c_min = (cur_tap-all_dq_c_min);
        }
        cur_tap = cur_tap << 8;

        *dacdqr = (1<<31) | ((*dacdqr & 0xFF000000) | c_max | cur_tap | c_min);

        dacdqr++;
    }

    /* 6. Enable auto calibraton control */
//    *dacspcr = DRAMI.dacspcr;     //FIXME

    /* 7. Update PHY parameter */
    _memctl_update_phy_param();

    return;
}
#endif

MEMCNTLR_SECTION
u32_t DDR_Calibration(void)
{
    u32_t target_addr, len, resolution, w_start, r_start;
    u32_t retry_limit;
    target_addr = MEMCTL_CALI_TARGET_ADDR;
    len         = MEMCTL_CALI_TARGET_LEN;
    resolution  = MEMCTL_CALI_FULL_SCAN_RESOLUTION;
    w_start     = MEMCTL_CALI_WRITE_DELAY_START_TAP;
    r_start     = MEMCTL_CALI_READ_DELAY_START_TAP;

    
    /* Configure Calibation mode: Digital delay line or Analog DLL */
    retry_limit = 0;

    /* Do a fully scan to choose a proper point. */
#if CALIBRATION_TYPE==Y_CALI
    while( 0 != _DDR_Calibration_Full_Scan(target_addr, len, resolution, w_start, r_start))
#else
    while( 0 != __DDR_Calibration_Full_Scan_Window(target_addr, len, resolution, w_start, r_start))
#endif
    {
        /* Base on the proper point, we do a one dimension scan for the reliabilties. */
        //return _DDR_Calibration_One_Dimension();
        retry_limit++;
        if(retry_limit > MEMCTL_CALI_RETRY_LIMILT){
            return 1; /* Fail, need to define failure status. */
        }
    }
    puts("II: DDR Calibration... done\n");
#if 0   //FIXME
    if(DRAMI.dacspcr & MEMCTL_DACSPCR_PERIOD_EN_MASK) {
        _memctl_enable_hw_auto_cali();
    }else{
        _memctl_disable_hw_auto_cali();
    }
    _memctl_disable_hw_auto_cali();
#endif

    return 0;
}

