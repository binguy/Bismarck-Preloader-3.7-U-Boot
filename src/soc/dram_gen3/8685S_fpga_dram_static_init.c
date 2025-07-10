#include <init_define.h>
#include <uart/uart.h>
#include <dram/memcntlr_util.h>


void delay_for_setting(void){
    volatile u32_t i=10000;
    while(i>0) i--;
}
extern cg_info_t cg_info_query;
void ddr123_static_setup(void)
{
    if(RFLD_MCR(dram_type)==0){
        printf("II: FPGA DDR1 Init ...");
        
        REG32(0xb8001000)=0x00000060;
        REG32(0xb8001004)=0x11220000;
        REG32(0xb8001008)=0x03000800;
        REG32(0xb800100c)=0x00000000;
        REG32(0xb8001010)=0x00301000;
        REG32(0xb800101c)=0x00100061;
        delay_for_setting();
        REG32(0xb8001500)=0xc0000000;
        REG32(0xb8001500)=0xc0000010;
    }else if(RFLD_MCR(dram_type)==1){
        printf("II: FPGA(V6) DDR2 Init ...");   

        REG32(0xb8001064)=0x22220000;
        REG32(0xb8001000)=0x10205c60;
        delay_for_setting();
        REG32(0xb8001004)=0x11220000;
        REG32(0xb8001008)=0x41301804;
        REG32(0xb800100c)=0x00000002;
        REG32(0xb8001010)=0x00502000;

        REG32(0xb800101c)=0x00120000;
        delay_for_setting();
        REG32(0xb800101c)=0x00130000;
        delay_for_setting();
        REG32(0xb800101c)=0x00110001;
        delay_for_setting();
        REG32(0xb800101c)=0x00110000;  
        delay_for_setting();
        REG32(0xb800101c)=0x00100352;  
        delay_for_setting();
        REG32(0xb800101c)=0x00100252;  
        delay_for_setting();
        REG32(0xb8001500)=0x80000000;  
        delay_for_setting();
        REG32(0xb8001500)=0x80000010;  
        delay_for_setting();
    }else if(RFLD_MCR(dram_type)==2){
        printf("II: FPGA(V6) DDR3 Init ...");   
        /* configure DRAM (DDR3) */

        REG32(0xb8001064) = 0x22220000;

        REG32(0xb8001000) = 0x200001E0;      
        delay_for_setting();
        REG32(0xb8001004) = 0x21220000;
        REG32(0xb8001008) = 0x44433804;
        REG32(0xb800100c) = 0x02000301;
        REG32(0xb8001010) = 0x00401000;
        
        REG32(0xb800101c) = 0x00120000;
        delay_for_setting();
        REG32(0xb800101c) = 0x00130000;
        delay_for_setting();
        REG32(0xb800101c) = 0x00110043;
        delay_for_setting();
        REG32(0xb800101c) = 0x00110042;
        delay_for_setting();
        REG32(0xb800101c) = 0x00100320;
        delay_for_setting();

        REG32(0xb8001080) = 0x80000000;
        delay_for_setting();
        REG32(0xb8001500) = 0x80000000;
        REG32(0xb8001500) = 0x80000030;
    }

    if(cg_info_query.dev_freq.mem_mhz > cg_info_query.dev_freq.lx_mhz){
        u32_t slow_bit = REG32(0xB8000308);
        REG32(0xB8000308) = (slow_bit |0x7);
    }

#ifdef OTTO_PROJECT_FPGA
    ISTAT_SET(cal, MEM_CAL_OK);
#endif

    printf(" done\n");
}

REG_INIT_FUNC(ddr123_static_setup, 30);

