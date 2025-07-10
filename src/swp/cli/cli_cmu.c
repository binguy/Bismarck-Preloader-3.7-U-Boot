#include <soc.h>
#include <cli/cli_access.h>
//#include <util.h>
#include <cg/cg.h>
#include <cg/cmu.h>
#include <lib/misc/timer.h>

#ifndef SECTION_CLI_UTIL
    #define SECTION_CLI_UTIL
#endif

void cmu_std_dynamic_test(void)
{
    printf("II: %s\n",__FUNCTION__);
    printf("    CMU init state: GCR=0x%08x,  SDCR=0x%08x, CR=0x%08x, SCR=0x%08x\n", OC0_CMUGCRrv, OC0_CMUSDCRrv, OC0_CMUCRrv, OC0_CMUSCRrv);

    OC0_CMUCR_T cr, fake_cr;
    u32_t divisor, base, mul, sleep_clk;
    u32_t timer_mode = (1<<24);
    u32_t timer_en   = (1<<28);
    u32_t tc_ip_clear= (1<<16);
    u32_t tc_ie_en   = (1<<20);

    /*Init CMU Wake-up Interrupt Selection*/
    REG32(0xb80003B0) = 0xFFFFFFFF;
    REG32(0xb80003B8) = 0xFFFFFFFF;


    for(divisor=0; divisor<=7; divisor++){
        cr.v = OC0_CMUCRrv;
        //Set auto_bz & interrupt type
        cr.f.auto_bz = 1;
        cr.f.int_cxn_type = 1;

        //Calculate slow bit for wk & slp
        cr.f.se_sram_rom_wk = cr.f.se_sram_rom;
        cr.f.se_sram_rom_slp= 1;
        cr.f.se_sram_rom_hs = 1;
        
        sleep_clk = cg_cur_query.cpu0_mhz/(1<<divisor);
        cr.f.se_dram_wk = cr.f.se_dram;
        cr.f.se_dram_slp= (sleep_clk < cg_cur_query.mem_mhz*2)?1:0;
        cr.f.se_dram_hs = 1;
        
        /*Due to the CMU bug, when using handshake mode, the wk_slow_bit should be set to the error value first*/
        fake_cr.v = cr.v;
        fake_cr.f.se_sram_rom_wk = ~cr.f.se_sram_rom_wk;
        fake_cr.f.se_sram_rom_hs = 0;
        fake_cr.f.se_dram_wk = ~cr.f.se_dram_wk;
        fake_cr.f.se_dram_hs = 0;
        

        //Change to FIX mode and set fake value
        RMOD_OC0_CMUGCR(freq_div,divisor, cmu_mode, DISABLE_CMU);
        OC0_CMUCRrv = fake_cr.v;
//        printf("    Fake     state: GCR=0x%08x,  SDCR=0x%08x, CR=0x%08x, SCR=0x%08x\n", OC0_CMUGCRrv, OC0_CMUSDCRrv, OC0_CMUCRrv, OC0_CMUSCRrv);
        
        for(base=0; base<=6; base++){
            RMOD_OC0_CMUSDCR(dly_base, base); 
            for(mul=0; mul<=15; mul++){
                /*Init CMU to dynamic mode & Update [mul]*/
                RMOD_OC0_CMUCR(dly_mul, mul);
                RMOD_OC0_CMUGCR(busy, 0, cmu_mode, ENABLE_CMU_DYNAMIC_MODE);
//                printf("    Check    state: GCR=0x%08x,  SDCR=0x%08x, CR=0x%08x, SCR=0x%08x\n", OC0_CMUGCRrv, OC0_CMUSDCRrv, OC0_CMUCRrv, OC0_CMUSCRrv);
                
                udelay(200);
 
                /*Clear & Set Timer 2 to timer mode*/
                REG32(0xB8003220) = 0x0fffffff;
                REG32(0xB800322C) = tc_ie_en|tc_ip_clear;
                REG32(0xB8003228) = timer_en|timer_mode|50;             

                while(!RFLD_OC0_CMUGCR(busy));                
//                printf("    Test     state: GCR=0x%08x,  SDCR=0x%08x, CR=0x%08x, SCR=0x%08x\n", OC0_CMUGCRrv, OC0_CMUSDCRrv, OC0_CMUCRrv, OC0_CMUSCRrv);
                printf("    CMU wakeup: GCR=0x%08x,  SDCR=0x%08x, CR=0x%08x, SCR=0x%08x\n", OC0_CMUGCRrv, OC0_CMUSDCRrv, OC0_CMUCRrv, OC0_CMUSCRrv);
            }        
        }        
    }
}


/*Cases dependent parameters*/
#define INIT_VALUE (0x5A5AA5A5)
#define BACKGROUND_VALUE (0xDEADDEAD)
#define GET_SEED 1
#define SET_SEED 0
#define RANDOM_TEST
#define TEST_TIMES (0x1)
//#define DIFF_ROWS
//#define USE_BYTESET /* exclusive with DIFF_ROWS */
/*
  get_or_set = GET_SEED: get seed
  get_or_set = SET_SEED: set seed
*/
SECTION_CLI_UTIL static void __srandom32(unsigned int *a1, unsigned int *a2, unsigned int *a3, unsigned int get_or_set)
{
	static int s1, s2, s3;
	if(GET_SEED==get_or_set){
		*a1=s1;
		*a2=s2;
		*a3=s3;
	}else{
		s1 = *a1;
		s2 = *a2;
		s3 = *a3;
	}
}


SECTION_CLI_UTIL static unsigned int __random32(void)
{
#define TAUSWORTHE(s,a,b,c,d) ((s&c)<<d) ^ (((s <<a) ^ s)>>b)
	unsigned int s1, s2, s3;
	__srandom32(&s1, &s2, &s3, GET_SEED);

	s1 = TAUSWORTHE(s1, 13, 19, 4294967294UL, 12);
	s2 = TAUSWORTHE(s2, 2, 25, 4294967288UL, 4);
	s3 = TAUSWORTHE(s3, 3, 11, 4294967280UL, 17);

	__srandom32(&s1, &s2, &s3, SET_SEED);

	return (s1 ^ s2 ^ s3);
}


void cmu_random_wakeup_time(void)
{
    OC0_CMUCR_T cr, fake_cr;
    u32_t divisor=7, delay_base=4, delay_mul=5, sleep_clk;
    u32_t timer_mode = (1<<24);
    u32_t timer_en   = (1<<28);
    u32_t tc_ip_clear= (1<<16);
    u32_t tc_ie_en   = (1<<20);

    /*Init CMU Wake-up Interrupt Selection*/
    REG32(0xb80003B0) = 0xFFFFFFFF;
    REG32(0xb80003B8) = 0xFFFFFFFF;

    /*Set Timer 2~5 to timer mode with different divisor*/
    REG32(0xB8003228) = timer_en|timer_mode|100;             
    REG32(0xB8003238) = timer_en|timer_mode|50;             
    REG32(0xB8003248) = timer_en|timer_mode|40;             
    REG32(0xB8003258) = timer_en|timer_mode|20;             

    cr.v = OC0_CMUCRrv;
    //Set auto_bz / interrupt type / delay_mul
    cr.f.auto_bz = 1;
    cr.f.int_cxn_type = 1;
    cr.f.dly_mul = delay_mul;

    //Calculate slow bit for wk & slp
    cr.f.se_sram_rom_wk = cr.f.se_sram_rom;
    cr.f.se_sram_rom_slp= 1;
    cr.f.se_sram_rom_hs = 1;
    
    sleep_clk = cg_cur_query.cpu0_mhz/(1<<divisor);
    cr.f.se_dram_wk = cr.f.se_dram;
    cr.f.se_dram_slp= (sleep_clk < cg_cur_query.mem_mhz*2)?1:0;
    cr.f.se_dram_hs = 1;
    
    /*Due to the CMU bug, when using handshake mode, the wk_slow_bit should be set to the error value first*/
    fake_cr.v = cr.v;
    fake_cr.f.se_sram_rom_wk = ~cr.f.se_sram_rom_wk;
    fake_cr.f.se_sram_rom_hs = 0;
    fake_cr.f.se_dram_wk = ~cr.f.se_dram_wk;
    fake_cr.f.se_dram_hs = 0;
    

    //Change to FIX mode and set fake value
    //Set Sleep Delay Control Register
    RMOD_OC0_CMUGCR(busy, 1, freq_div,divisor, cmu_mode, DISABLE_CMU);
    OC0_CMUSDCRrv = delay_base;
    OC0_CMUCRrv = fake_cr.v;
    OC0_CMUCRrv = cr.v;

    u32_t ra, rb, rc;
//#define SEED1   ((0x13243*(b+1))&0xffffff)
//#define SEED2   (0xaaa0bdd+b)
//#define SEED3   (0xfffbda0-b)

    #define SEED1   (0x1356243)
    #define SEED2   (0xaaa0bdd)
    #define SEED3   (0xfffbda0)
    ra=SEED1;rb=SEED2;rc=SEED3;
    __srandom32(&ra, &rb, &rc, SET_SEED);


    printf("II: %s\n",__FUNCTION__);
    printf("    CMU init state: GCR=0x%08x,  SDCR=0x%08x, CR=0x%08x, SCR=0x%08x\n", OC0_CMUGCRrv, OC0_CMUSDCRrv, OC0_CMUCRrv, OC0_CMUSCRrv);

    while(1){
        /*Init CMU to dynamic mode*/
        RMOD_OC0_CMUGCR(busy, 0, cmu_mode, ENABLE_CMU_DYNAMIC_MODE);
        
        /*Clear & Set Timer 2 to timer mode*/
        REG32(0xB8003220) = 0x0fffffff & __random32();
        REG32(0xB8003230) = 0x0fffffff & __random32();
        REG32(0xB8003240) = 0x0fffffff & __random32();
        REG32(0xB8003250) = 0x0fffffff & __random32();

        REG32(0xB800322C) = tc_ie_en|tc_ip_clear;
        REG32(0xB800323C) = tc_ie_en|tc_ip_clear;
        REG32(0xB800324C) = tc_ie_en|tc_ip_clear;
        REG32(0xB800325C) = tc_ie_en|tc_ip_clear;

        while(!RFLD_OC0_CMUGCR(busy));                
        printf("    Wakeup: SCR=0x%08x\n", OC0_CMUSCRrv);
    }        
}


SECTION_CLI_UTIL cli_cmd_ret_t
cli_cmu_dynamic_test(const void *user, u32_t argc, const char *argv[])
{
    // "cmu dynamic [-tcs/-timers]"
    
    
    if(argc<2) return CCR_INCOMPLETE_CMD;
    else if(argc==2){
        //Run all CMU setting case, only use Timer2 
        cmu_std_dynamic_test();
    }else{
        if((cli_strcmp(argv[2], "-tcs", ' ') == 0) || (cli_strcmp(argv[2], "-timers", ' ') == 0)){
            //Run all CMU setting case, only use Timer2 
            cmu_random_wakeup_time();
        }else return CCR_INCOMPLETE_CMD;
    }
    return CCR_OK;
}

cli_top_node(cmu, VZERO);
    cli_add_node(dynamic, cmu, (cli_cmd_func_t *)cli_cmu_dynamic_test);
    cli_add_help(dynamic, "cmu dynamic");

