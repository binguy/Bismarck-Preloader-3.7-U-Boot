#include "./bspchip.h"
#include "./memctl.h"
#include "./memctl_func.h"


#define MEMCTL_CALI_RETRY_LIMILT 		(5)
#define MEMCTL_CALI_MIN_READ_WINDOW 		(7)
#define MEMCTL_CALI_MIN_WRITE_WINDOW 		(7)
#ifdef CONFIG_MIPS
#define CPU_DCACHE_SIZE 			(0x1000)
#else
#define CPU_DCACHE_SIZE 			(0x8000)
#endif
#define MEMCTL_CALI_TARGET_LEN                  (CPU_DCACHE_SIZE * 2)
#define MEMCTL_CALI_FULL_SCAN_RESOLUTION        (2)
#define MEMCTL_CALI_WRITE_DELAY_START_TAP       (0)
#define MEMCTL_CALI_READ_DELAY_START_TAP        (0)
#define MEMCTL_CALI_TARGET_ADDR 		(0x80b00000)

#define _REDUCE_CALIBRATION_

/* DRAM patch from efuse */
extern unsigned int EFPH_patch_num;
extern unsigned int EFPH_DQR_delay_en;
extern unsigned int EFPH_DQW_delay_en;
extern unsigned int EFPH_DM_delay_en;
extern unsigned int EFPH_DQR_delay;
extern unsigned int EFPH_DQW_delay;
extern unsigned int EFPH_DM_delay;
extern unsigned int EFVD_Vendor;


/* Adds for SD5-----------------------Start--*/
#ifdef CONFIG_MIPS
#define _memctl_DCache_flush_invalidate _1004K_L1_DCache_flush
#else
void _memctl_DCache_flush_invalidate(void){
        __asm__ volatile(
                        ".set  push\n\t"
                        ".set  noreorder\n\t"
                        "mfc0    $8, $20\n\t"
                        "ori     $8, 0x202\n\t"
                        "xori    $9, $8, 0x202\n\t"
                        "mtc0    $9, $20\n\t"
                        "mtc0    $8, $20\n\t"
                        ".set pop\n"
                        "nop"
                        : /* no output */
                        : /* no input */
                                );
}
#endif
/* Adds for SD5-----------------------End--*/

/*
 * Function Implementation
 */

void sys_watchdog_enable(unsigned int ph1, unsigned int ph2);

uint32 pat_ary[] __attribute__ ((section(".text")))= {
 			0x00010000, 0x01234567, 0x00000000, 0x76543210,
			0xFFFFFFFF, 0x89abcdef, 0x0000FFFF, 0xfedcba98,
			0xFFFF0000, 0x00FF00FF, 0xFF00FF00, 0xF0F0F0F0,
			0x0F0F0F0F, 0x5A5AA5A5, 0xA5A55A5A, 0x5A5AA5A5,
			0xA5A55A5A, 0xA5A55A5A, 0x5A5AA5A5, 0xA5A55A5A,
			0x5A5AA5A5, 0x5555AAAA, 0xAAAA5555, 0x5555AAAA,
			0xAAAA5555, 0xAAAA5555, 0x5555AAAA, 0xAAAA5555,
			0x5555AAAA, 0xCC3333CC, 0x33CCCC33, 0xCCCC3333
		   };

#if 1
void _write_pattern_1(uint32 start_addr, uint32 len)
{
        volatile uint32 *p_start, data_tmp;
        uint32 b_len;
        uint32 ary_i;

		//puts("In _write_pattern_1: writes 0x"); puthex(start_addr); puts("len: "); puthex(len); puts("\n\r");

        /* In case of write through D-Cache mechanisim, read data in DCache */
        p_start = (volatile uint32 *)start_addr;
        for(b_len = 0; b_len < len; b_len += sizeof(uint32)){
                data_tmp = *p_start;
				data_tmp = data_tmp;
				p_start++;
        }

		//puts("store data in L1$\n\r");

        /* Write data */
        p_start = (volatile uint32 *)start_addr;
		ary_i = 0;
        for(b_len = 0; b_len < len; b_len += sizeof(uint32)){
                *p_start = pat_ary[ary_i];
                p_start++;
		ary_i = (ary_i+1) % (sizeof(pat_ary)/sizeof(uint32));
        }

		//puts("write pattern to L1$\n\r");

        _memctl_DCache_flush_invalidate();

        return;
}
#else
void _write_pattern_1(uint32 start_addr, uint32 len, uint32 r_delay, uint32 w_delay)
{
        volatile uint32 *p_start, data_tmp;
        uint32 b_len;
        uint32 ary_i;

		//puts("In _write_pattern_1: writes 0x"); puthex(start_addr); puts("len: "); puthex(len); puts("\n\r");

        /* In case of write through D-Cache mechanisim, read data in DCache */
        p_start = (volatile uint32 *)start_addr;
        for(b_len = 0; b_len < len; b_len += sizeof(uint32)){
                data_tmp = *p_start;
				p_start++;
        }

		//puts("store data in L1$\n\r");

        /* Write data */
        p_start = (volatile uint32 *)start_addr;
		ary_i = 0;
        for(b_len = 0; b_len < len; b_len += sizeof(uint32)){
                *p_start = pat_ary[ary_i];
                p_start++;
		ary_i = (ary_i+1) % (sizeof(pat_ary)/sizeof(uint32));
        }

		//puts("write pattern to L1$\n\r");

        _memctl_DCache_flush_invalidate();

#if 0		//DEBUG

		p_start = (volatile uint32 *)start_addr;

        for(b_len = 0; b_len < len; b_len += sizeof(uint32)){
			if(*p_start != pat_ary[ary_i] && w_delay==16 && r_delay ==16){
			puts("(0)w_delay = 0x"); puthex(w_delay); puts("  r_delay = 0x"); puthex(r_delay);
			puts("  *p_start = 0x");puthex(*p_start); puts("  pat_ary[ary_i] = 0x");puthex(pat_ary[ary_i]);puts("\n\r");
                p_start++;
			ary_i = (ary_i+1) % (sizeof(pat_ary)/sizeof(uint32));
			}
        }
#endif

        return;
}
#endif

uint32 _verify_pattern_1(uint32 start_addr, uint32 len)
{

        volatile uint32 *p_start, data_tmp;
        uint32 b_len, err_result;
        uint32 ary_i, pat_data;

        _memctl_DCache_flush_invalidate();

        err_result = 0;

        /* Read data */
		ary_i = 0;
        p_start = (volatile uint32 *)start_addr;
        for(b_len = 0; b_len < len; b_len += sizeof(uint32)){
		data_tmp = *p_start;
		pat_data = pat_ary[ary_i];
		ary_i = (ary_i+1) % (sizeof(pat_ary)/sizeof(uint32));
		err_result = err_result | ( (pat_data | data_tmp) & ( ~(pat_data & data_tmp)));
		if(err_result == 0xffffffff)
		        return err_result;
		p_start++;
        }

        return err_result;
}

void memctl_sync_write_buf(void)
{
	*((volatile unsigned int *)0xB8001038) = 0x80000000;
	while(*((volatile unsigned int *)0xB8001038) & 0x80000000);
	return;
}

void _memctl_set_phy_delay_all(uint32 w_delay, uint32 r_delay)
{
	uint32 i_dq;
        volatile uint32 *ddcrdqr_base;

        ddcrdqr_base = (uint32 *)DACDQR;
	//puts("w_delay = ");puthex(w_delay);puts("  r_delay = ");puthex(r_delay);puts("\n\r");
	//printf("%s:%d: wdelay(%d), r_delay(%d)\n", __FUNCTION__, __LINE__, w_delay, r_delay);

	for(i_dq = 0; i_dq < 32; i_dq++){
		*ddcrdqr_base = (w_delay << 24) | (r_delay << 8);
		ddcrdqr_base++;
	}

        _memctl_update_phy_param();
		//puts("ddcrdqr_base(70) = ");puthex(REG32(0xb8001570));puts("\n\r");
		//puts("ddcrdqr_base(74) = ");puthex(REG32(0xb8001574));puts("\n\r");
		//puts("ddcrdqr_base(78) = ");puthex(REG32(0xb8001578));puts("\n\r");

	return ;
}

void  _memctl_set_phy_delay_dqrf(uint32 bit_loc,uint32 max_w_seq_start,uint32 max_w_len,uint32 max_r_seq_start,uint32 max_r_len)
{
	volatile uint32 *ddcrdqr_base,*dwdqor_base,*dwdqor;
	unsigned char r_delay_tap, w_delay_tap, r_holdtime;
	ddcrdqr_base = (volatile uint32 *)DACDQR;
	dwdqor_base = (volatile uint32 *)DWDQOR;

	ddcrdqr_base += bit_loc;

	/* For DDR2, DDR3 read delay tap*/

	if(EFVD_Vendor==1)
		r_holdtime = 16;
	else
		r_holdtime = 12;

	if(max_r_seq_start==0 && max_r_len>=r_holdtime){
		r_delay_tap = max_r_len - r_holdtime;
	}else if(max_r_seq_start==0 && max_r_len<=r_holdtime){
		r_delay_tap = 0;
	}else{
		r_delay_tap = max_r_seq_start + max_r_len/2;
	}

	/* For DDR2, DDR3 write delay tap*/

	if(max_w_seq_start==0){
	w_delay_tap = max_w_seq_start + max_w_len/3;
	}else{
		w_delay_tap = max_w_seq_start + max_w_len/2;
	}
	// DRAM patch from efuse
	if (EFPH_patch_num!=0 && EFPH_DQR_delay_en==1){
		//printf("DQ read delay enable\n");
		if(EFPH_DQR_delay<8){
			if(r_delay_tap>=(EFPH_DQR_delay+1))
				r_delay_tap = r_delay_tap - (EFPH_DQR_delay+1);
			else
				r_delay_tap = 0;
		}else{
				r_delay_tap = r_delay_tap + (EFPH_DQR_delay+1-8);
		}
	}
	if (EFPH_patch_num!=0 && EFPH_DQW_delay_en==1){
		//printf("DQ write delay enable\n");
		if(EFPH_DQW_delay<8){
			if(w_delay_tap>=(EFPH_DQW_delay+1))
				w_delay_tap = w_delay_tap - (EFPH_DQW_delay+1);
			else
				w_delay_tap = 0;
		}else{
			w_delay_tap = w_delay_tap + (EFPH_DQW_delay+1-8);
		}
	}

	/* write delay tap into controller */
	*ddcrdqr_base = ((w_delay_tap & 0x1f) << 24) | \
					(((max_r_seq_start + max_r_len) & 0x1f) << 16) | \
					(((max_r_seq_start + r_delay_tap) & 0x1f) << 8) | \
					((max_r_seq_start & 0x1f) << 0);

	/* Enable delay tap sync with write delay tap*/
	if (bit_loc < 16){
		dwdqor = dwdqor_base + (bit_loc / 4);
		*dwdqor = *dwdqor | (w_delay_tap << ((bit_loc % 4) * 8));
	}

	_memctl_update_phy_param();

	return;
}

#ifndef _REDUCE_CALIBRATION_
void _memctl_result_to_DQ_RW_Array(uint32 result, uint32 w_delay, uint32 r_delay, uint32 RW_array_addr[32][32])
{
        /*
         * RW_array_addr [32]   [32] : [Rising 0~15, Falling 0~15] [w_delay]
         *              32bit  W_delay
         */

        uint32 bit_loc, correct_bit;

        /* We mark correct bit */
 	result = ~result;

	if(REG32(DCR) & 0x0F000000 == 0){ /* 8bit mode*/
		result = ((result & 0xFF000000) >> 24) | ((result & 0x00FF0000)) |\
					((result & 0x0000FF00) >> 8) | ((result & 0x000000FF) << 16);
	}else { /* 16bit mode*/
        	result = ((result & 0xFFFF0000) >> 16) | ((result & 0x0000FFFF) << 16);
	}

        for(bit_loc=0; bit_loc < 32; bit_loc++){
                correct_bit = (result >> bit_loc) & 0x1;
                RW_array_addr[bit_loc][w_delay] |= (correct_bit << r_delay);

#if 0	//DEBUG
		if(bit_loc==0x19){
			puts("(!!!)w_delay = 0x"); puthex(w_delay); puts("  r_delay = 0x"); puthex(r_delay);
			puts("  bit_loc = 0x"); puthex(bit_loc);
			puts("  correct_bit = 0x"); puthex(correct_bit);
			puts("\n\r");

		}
#endif

        }

        return;
}


uint32 _memctl_find_proper_RW_dealy(uint32 resolution, uint32 w_start, uint32 r_start, uint32 DQ_RW_Array[32][32])
{
        uint32 max_r_seq_start, max_r_len, r_delay, r_seq_start, r_len;
        uint32 max_w_seq_start, max_w_len, w_delay, w_seq_start, w_len, search_seq_start, bit_loc;
        uint32 bit_fail, mode_16bit;
		uint32 r_start_shift_L=0, r_start_shift_U=0, last_value_L=0, last_value_U=0;

		// Read DQS group delay auto K: Read DQS group delay value
		last_value_L = (REG32(DACCR) >> 16) & 0x1F;
		last_value_U = (REG32(DACCR) >> 8) & 0x1F;

        if(REG32(DCR) & 0x0F000000){
                mode_16bit = 1;
        }else{
                mode_16bit = 0;
        }

        bit_fail = 0;
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
		//puts("Bit:"); puthex(bit_loc); puts(" \n");

                /* Searching for the max. sequetial read window. */
                for(w_delay = w_start; w_delay < 32; w_delay+=resolution){
                        r_len = 0;
                        r_seq_start = 0;
                        search_seq_start = 1;
			//printf("   w(%d) %08x\n", w_delay, DQ_RW_Array[bit_loc][w_delay]);
			//puts("w("); puthex(w_delay); puts(") ");
			//puthex(DQ_RW_Array[bit_loc][w_delay]); puts(" \n");


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

#if 0	//DEBUG
		if(bit_loc==0x19){
			puts("(!!!)w_delay = 0x"); puthex(w_delay); puts("  r_delay = 0x"); puthex(r_delay);
			puts("  bit_loc = 0x"); puthex(bit_loc);
			puts("  DQ_RW_Array = 0x"); puthex((DQ_RW_Array[bit_loc][w_delay] >> r_delay) & 0x1);
			puts("\n\r");

		}
#endif
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

		//printf("bit:%02d, max_r_s(%d), max_r_l(%d), max_w_s(%d), max_w_len(%d)\n", bit_loc, max_r_seq_start, max_r_len,  max_w_seq_start, max_w_len);
			puts("AK: Bit:"); puthex(bit_loc); puts(" ");
			puts("max_r_s:"); puthex(max_r_seq_start); puts(" ");
			puts("max_r_l:"); puthex(max_r_len); puts(" ");
			puts("max_w_s:"); puthex(max_w_seq_start); puts(" ");
			puts("max_w_l:"); puthex(max_w_len); puts(" \n\r");

			_memctl_set_phy_delay_dqrf(bit_loc, max_w_seq_start, max_w_len, max_r_seq_start, max_r_len);

			if((max_w_len <= MEMCTL_CALI_MIN_WRITE_WINDOW) || (max_r_len <= MEMCTL_CALI_MIN_READ_WINDOW)){
                        bit_fail |= (1 << bit_loc);
			}

			// Read DQS group delay auto K: Search the max_r_start of all bit_loc
            if((bit_loc < 8) || ((bit_loc >= 16)&&(bit_loc < 24))){
				if(max_r_seq_start > r_start_shift_L){		//search the MAX "max_r_seq_start" in LDQ
					r_start_shift_L = max_r_seq_start + last_value_L;	//group delay is equal to start value add lest group delay
				}
            }
            if(((bit_loc >= 8)&&(bit_loc < 16)) || (bit_loc >= 24)){
				if(max_r_seq_start > r_start_shift_U){		//search the MAX "max_r_seq_start" in HDQ
					r_start_shift_U = max_r_seq_start + last_value_U;	//group delay is equal to start value add lest group delay
				}
            }
        }

		//Read DQS group delay auto K: Save the DQS group value
		if(r_start_shift_L == 0){		//if r_start_shift_L =0 means the group delay is good now.
			r_start_shift_L = last_value_L;		//so the group delay is last group delay
		}else{							//if r_start_shift_L !=0 means the group delay is not enough.
			last_value_L = r_start_shift_L;		//so save the value in "last_value," write the "r_start_shift" and do the best again.
		}
		if(r_start_shift_U == 0){
			r_start_shift_U = last_value_U;
		}else{
			last_value_U = r_start_shift_U;
		}

		//Read DQS group delay auto K: Write the group delay
		if((((REG32(DACCR) >> 16) & 0x1F) != r_start_shift_L) || (((REG32(DACCR) >> 8) & 0x1F) != r_start_shift_U)) {
			REG32(DACCR) &= ~((0x1F << 16) | (0x1F << 8));		//clear DQS group delay
			puts("\n\rAK: Let's calibrate DRAM again, it will be better!\n\r");
			puts("AK: Set DQS0 group delay: "); puthex(r_start_shift_L); puts("\n\r");
			puts("AK: Set DQS1 group delay: "); puthex(r_start_shift_U); puts("\n\r");
			REG32(DACCR) = REG32(DACCR) | ((r_start_shift_L & 0x1F) << 16) | ((r_start_shift_U & 0x1F) << 8);
			REG32(DMCR) = REG32(DMCR);
			return 2;
		}

		puts("AK: ENABLE: DQ output enable delay SYNC\r\n");	//code locat in "_memctl_set_phy_delay_dqrf"
        return bit_fail;
}

uint32 _DDR_Calibration_Full_Scan(uint32 target_addr, uint32 len, uint32 resolution, uint32 w_start, uint32 r_start)
{
        uint32 w_delay, r_delay, WR_Result,ret;
        uint32 DQ_RW_Array[32][32];

		scan_start:
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

			/*	puts("w_delay: "); puthex(w_delay);
				puts(" r_delay: "); puthex(r_delay);
				puts(" WR_result: "); puthex(WR_Result);
				puts("\r");*/

			_memctl_result_to_DQ_RW_Array(WR_Result, w_delay, r_delay, DQ_RW_Array);
                }
        }
        /* All scan result is in DQ_RW_Array, choose a proper delay tap setting. */

		ret = _memctl_find_proper_RW_dealy( resolution, w_start, r_start, DQ_RW_Array);
        if(ret == 2){
 			goto scan_start;	//Read group delay Auto calibration loop
		}else if(ret == 0){
			return 0;/* Pass */
        }else{
			return 1;/* Fali */
        }
}

#else /* _REDUCE_CALIBRATION_ */

uint32 DDR_Calibration_Full_Scan_Reduce(uint32 target_addr, uint32 len, uint32 resolution, uint32 w_start, uint32 r_start)
{


    uint8 max_r_seq_start_array[32], max_r_len_array[32];
    uint8 max_w_seq_start_array[32], max_w_len_array[32];
    uint8 seq_start_array[32];
    uint8 search_seq_start_array[32];
    uint8 w_delay, r_delay, bit_loc;
    uint8 is_this_bit_correct;
    uint8 mode_16bit;
    uint8 max_r_seq_start = 0, max_r_len = 0;
    uint8 max_w_seq_start, max_w_len;
    uint8 r_seq_start, r_len;
    uint8 w_seq_start, w_len;
    uint8 search_seq_start;
    uint32 WR_Result;
	uint32 r_start_shift_L=0, r_start_shift_U=0, last_value_L=0, last_value_U=0;

	puts("AK: Reduce DDR calibration\n\r");

    if(REG32(DCR) & 0x0F000000){
        mode_16bit = 1;
    }else{
        mode_16bit = 0;
    }

	scan_start:

    /**************************************************************
     ******* Searching for the max. sequetial read window.*********
     **************************************************************/
    for(bit_loc = 0; bit_loc < 32; bit_loc++){
        max_r_seq_start_array[bit_loc] = 0;
        max_r_len_array[bit_loc]       = 0;
    }

    for(w_delay = w_start; w_delay < 32; w_delay += resolution){
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
                    if((bit_loc > 7) && (bit_loc < 16))
                        continue;
                    if((bit_loc > 23) && (bit_loc < 32))
                        continue;
                }

                max_r_seq_start  = max_r_seq_start_array[bit_loc];
                max_r_len        = max_r_len_array[bit_loc];
                r_seq_start      = seq_start_array[bit_loc];
                search_seq_start = search_seq_start_array[bit_loc];
                is_this_bit_correct = ((WR_Result>>bit_loc) & 0x1);
																		// if correct bit 0->0,    path is "Step A"
																		// if correct bit 0->1->0, path is "Step 1 -> Step 2 -> Step A"
																		// if correct bit 1->0,    path is "Step 1 -> Step 2 -> Step A"
																		// if correct bit 1->1,    path is "Step 1 -> Step B"
                if(search_seq_start == 1){								//Step 1: Search start
                    if(is_this_bit_correct == 1){						//if bit is correct
                        r_seq_start = r_delay;							//this is "the start"!
                        search_seq_start = 0;							//stop search start
                    }
                    if( (r_delay+resolution) >= 31 ){					//Step A: Step 2 Jemp to here OR there is not correct bit
                        r_len = 1;										//No more correct bit, set len=1
                        if(r_len > max_r_len){							//If jump here from Step 2, this "if" always "false".
                            max_r_len = r_len;							//If there is not correct bit, max len set 1.
                            max_r_seq_start = r_seq_start;				//at the same thme, update the max start. So the max start is selected when max len is occured.
                            r_len = 0;
                            r_seq_start = r_delay + resolution;
                        }
                    }
                }else{													//Step 2: Search end
                    if(is_this_bit_correct == 0){						//if bit is not correct
                        r_len = r_delay - r_seq_start - resolution + 1;	//this is "the end", calculate len
                        if(r_len > max_r_len){							//if the len is larger than previous len
                            max_r_len = r_len;							//update the max len
                            max_r_seq_start = r_seq_start;				//at the same thme, update the max start. So the max start is selected when max len is occured.
                            r_len = 0;
                            r_seq_start = r_delay + resolution;
                        }
		                    search_seq_start = 1;						//Jump to Step A
                    }else{
                        if((r_delay+resolution) >= 31){					//Step B: if the whole bit is correct
                            r_len = r_delay - r_seq_start + 1;			//this is "the end", calculate len
                            if(r_len > max_r_len){						//if the len is larger than previous len
                                max_r_len = r_len;						//update the max len
                                max_r_seq_start = r_seq_start;			//at the same thme, update the max start. So the max start is selected when max len is occured.
                                r_len = 0;
                                r_seq_start = r_delay + resolution;
                            }
                        }
                    }
                }

               if(max_r_len > max_r_len_array[bit_loc]){
                    max_r_seq_start_array[bit_loc] = max_r_seq_start;
                    max_r_len_array[bit_loc]       = max_r_len;
                }
                seq_start_array[bit_loc]     = r_seq_start;
                search_seq_start_array[bit_loc]= search_seq_start;

			}
        }
    }


    /*********************************************************************************
         Searching for the max. write delay window basing on max. read delay window.
     *********************************************************************************/
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
                    if((bit_loc > 7) && (bit_loc < 16))
                        continue;
                    if((bit_loc > 23) && (bit_loc < 32))
                        continue;
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

                if(max_w_len > max_w_len_array[bit_loc]){
                    max_w_seq_start_array[bit_loc] = max_w_seq_start;
                    max_w_len_array[bit_loc]       = max_w_len;
                }
                seq_start_array[bit_loc]     = w_seq_start;
                search_seq_start_array[bit_loc]= search_seq_start;

			}
        }
    }

    for(bit_loc = 0; bit_loc < 32; bit_loc++){
		puts("AK: Bit:"); puthex(bit_loc); puts(" ");
		puts("max_r_s:"); puthex(max_r_seq_start_array[bit_loc]); puts(" ");
		puts("max_r_l:"); puthex(max_r_len_array[bit_loc]); puts(" ");
		puts("max_w_s:"); puthex(max_w_seq_start_array[bit_loc]); puts(" ");
		puts("max_w_l:"); puthex(max_w_len_array[bit_loc]); puts(" \n\r");

		if((max_r_len_array[bit_loc]<8) | (max_w_len_array[bit_loc]<8)){
			puts("AK: Window is too small, watchdog enable\n");
			sys_watchdog_enable(0,0);
			while(1);
		}

		_memctl_set_phy_delay_dqrf(bit_loc, max_w_seq_start_array[bit_loc], max_w_len_array[bit_loc], max_r_seq_start_array[bit_loc], max_r_len_array[bit_loc]);

		// Read DQS group delay auto K: Search the max_r_start of all bit_loc

        if((bit_loc < 8) || ((bit_loc >= 16)&&(bit_loc < 24))){
			if(max_r_seq_start_array[bit_loc] > r_start_shift_L){		//search the MAX "max_r_seq_start_array" in LDQ
				r_start_shift_L = max_r_seq_start_array[bit_loc] + last_value_L;	//group delay is equal to start value add lest group delay
			}
        }
        if(((bit_loc >= 8)&&(bit_loc < 16)) || (bit_loc >= 24)){
			if(max_r_seq_start_array[bit_loc] > r_start_shift_U){		//search the MAX "max_r_seq_start_array" in HDQ
				r_start_shift_U = max_r_seq_start_array[bit_loc] + last_value_U;	//group delay is equal to start value add lest group delay
			}
        }

    }

	//Read DQS group delay auto K: Save the DQS group value
	if(r_start_shift_L == 0){		//if r_start_shift_L =0 means the group delay is good now.
		r_start_shift_L = last_value_L;		//so the group delay is last group delay
	}else{							//if r_start_shift_L !=0 means the group delay is not enough.
		last_value_L = r_start_shift_L;		//so save the value in "last_value," write the "r_start_shift" and do the best again.
	}
	if(r_start_shift_U == 0){
		r_start_shift_U = last_value_U;
	}else{
		last_value_U = r_start_shift_U;
	}

	//Read DQS group delay auto K: Write the group delay
	if((((REG32(DACCR) >> 16) & 0x1F) != r_start_shift_L) || (((REG32(DACCR) >> 8) & 0x1F) != r_start_shift_U)) {
		REG32(DACCR) &= ~((0x1F << 16) | (0x1F << 8));		//clear DQS group delay
		puts("\n\rAK: Let's calibrate DRAM again, it will be better!\n\r");
		puts("AK: Set DQS0 group delay: "); puthex(r_start_shift_L); puts("\n\r");
		puts("AK: Set DQS1 group delay: "); puthex(r_start_shift_U); puts("\n\r");
		REG32(DACCR) = REG32(DACCR) | ((r_start_shift_L & 0x1F) << 16) | ((r_start_shift_U & 0x1F) << 8);
		REG32(DMCR) = REG32(DMCR);
		goto scan_start;	//Read group delay Auto calibration loop
	}

	puts("ENABLE: DQ output enable delay SYNC\r\n");	//code locat in "_memctl_set_phy_delay_dqrf"

    return 0;
}

#endif /* _REDUCE_CALIBRATION_ */

#if CONFIG_ADDR_CHECK
void DDR_Addr_Check(uint32 dram_size){

	unsigned int addr_range, addr_range_idx=0, row_bank, chk, i=0, j=0;
	unsigned char A, flag=0;
	unsigned int *addr=0;

	if(dram_size==0)
		return;

	while(dram_size>>addr_range_idx != 1){		//Assume 0x10000000 is 2Gb(A0~A13), addr_range_idx is 28
		addr_range_idx++;
	}
	puts("DAC: addr_range_idx = ");puthex(addr_range_idx);puts("\n\r");
	addr_range = addr_range_idx - 1;	//addr_range is 28-1=27(1 address,2 byte)
	row_bank = addr_range - 10;				//row_bank is 27-10=17 (A0~A13 + BA0~BA2)
	puts("DAC: row+bank = ");puthex(row_bank);puts("\n\r");

	for (A=0; A<row_bank; A++){	//A=0 means address "A0"
		REG32(0xa0000000) = 0;	//Clear
		i = A + 11;				//if only A0 of row is high,
		addr = 0xa0000000 | (1 << i);			//address is 0x800(2^11), addr=2^i
		//puts("addr = ");puthex(addr);puts("\n\r");
		*addr = addr;		//write pattern
		//puts("*addr = ");puthex(*addr);puts("\n\r");

		//puts("0xa0000000 = ");puthex(REG32(0xa0000000));puts("\n\r");

		if(REG32(0xa0000000) == addr){
			puts("DAC:A");puthex(A);puts(" is BAD!\n\r");
			flag=1;
		}
	}
	if(flag==0)
		puts("DAC: Address of DRAM connection is correct!\n\r");
	return;
}
#endif

uint32 DDR_Calibration(unsigned char full_scan)
{
     uint32 target_addr, len, resolution, w_start, r_start;
	 uint32 retry_limit;
        target_addr 	= MEMCTL_CALI_TARGET_ADDR;
        len         		= MEMCTL_CALI_TARGET_LEN;
        resolution  	= MEMCTL_CALI_FULL_SCAN_RESOLUTION;
        w_start     	= MEMCTL_CALI_WRITE_DELAY_START_TAP;
        r_start     	= MEMCTL_CALI_READ_DELAY_START_TAP;
	retry_limit = 0;

		/* Enable dynamic PHY FIFO Reset */
		REG32(DACCR)= REG32(DACCR) | 0x20;

        /* Do a fully scan to choose a proper point. */
	if(full_scan){
#ifdef _REDUCE_CALIBRATION_
		 while( 0 != DDR_Calibration_Full_Scan_Reduce(target_addr, len, resolution, w_start, r_start)){
#else
	     while( 0 != _DDR_Calibration_Full_Scan(target_addr, len, resolution, w_start, r_start)){
#endif

	        /* Base on the proper point, we do a one dimension scan for the reliabilties. */
			retry_limit++;
			if(retry_limit > MEMCTL_CALI_RETRY_LIMILT){
	               	return 1; /* Fail, need to define failure status. */
			}
		}
	}
	return 0;
}

