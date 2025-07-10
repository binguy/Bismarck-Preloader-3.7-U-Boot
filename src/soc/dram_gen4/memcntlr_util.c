#include <soc.h>
#include <dram/memcntlr.h>
#include <util.h>

extern mc_info_t meminfo;

MEMCNTLR_SECTION
void mc_pupd_detection(void) {
    if (0==meminfo.cntlr_opt->pupd_en) return;
    DDPDR0_T ddpdr0 = {.v=DDPDR0rv};
    DDPDR1_T ddpdr1 = {.v=DDPDR1rv};
    DTR0_T dtr0 = {.v=DTR0rv};
    
    puts("II: PUPD Detection... enabled\n");

    ddpdr0.f.dqs0_pupd_det_en=1;
    ddpdr1.f.dqs1_pupd_det_en=1;
    ddpdr0.f.dqs0_pupd_start_det=dtr0.f.t_cas;
    ddpdr1.f.dqs1_pupd_start_det=dtr0.f.t_cas;
    DDPDR0rv=ddpdr0.v;
    DDPDR1rv=ddpdr1.v;
}

u16_t ddr2_8bit_size[] =  { 0x1022/* 32MB */, 0x1032/* 64MB */, 0x2032/* 128MB */, 0x2042/* 256MB */, 0x2052/* 512MB */};
u16_t ddr3_8bit_size[] =  { 0x2022/* 64MB */, 0x2032/* 128MB */, 0x2042/* 256MB */, 0x2052/* 512MB */, 0x2053/* 1024MB */};
u16_t ddr2_16bit_size[] = { 0x1121/* 32MB */, 0x1122/* 64MB */, 0x2122/* 128MB */, 0x2132/* 256MB */, 0x2142/* 512MB */};
u16_t ddr3_16bit_size[] = { 0x2121/* 64MB */, 0x2122/* 128MB */, 0x2132/* 256MB */, 0x2142/* 512MB */, 0x2152/* 1024MB */};

// DDR physical address
u32_t ddr2_verify_addr[] = { 0x067f0000/* 32MB */, 0x06ff0000/*  64MB */, 0x0eff0000/* 128MB */, 0x0fff0000/* 256MB */};    //FIXME
u32_t ddr3_verify_addr[] = { 0x08800200/* 64MB */, 0x08800600/* 128MB */, 0x09800600/* 256MB */, 0x0b800600/* 512MB */, 0x0f800600/* 1024MB */};

MEMCNTLR_SECTION
u16_t *memctl_get_size_array(unsigned int type, unsigned int buswidth) {
    u16_t *array;
    if (8==buswidth){
        array=(DDR_TYPE_DDR3==type)?(ddr3_8bit_size):(ddr2_8bit_size);
    }else{
        array=(DDR_TYPE_DDR3==type)?(ddr3_16bit_size):(ddr2_16bit_size);
    }
    return array;
}

MEMCNTLR_SECTION
u32_t * memctl_get_verify_addr(unsigned int *size, unsigned int type, unsigned int buswidth) {
    #define _assign_array_and_size(arr) ({*size=sizeof(arr)/sizeof(u32_t); arr;})

    if (DDR_TYPE_DDR3==type) {
        return _assign_array_and_size(ddr3_verify_addr);
    } else {
        return _assign_array_and_size(ddr2_verify_addr);
    } 
}

//#define ZONE0_OFFSET_CONFIG(off)    ({ DOR0rv=(off)<<28; }) // basic unit is 256MB

MEMCNTLR_SECTION
unsigned int memctlc_dram_size_detect(void) {

#define DCR_MASK    (0xffff0000)
#define INIT_STR    (0xdeadbeef)
#define PATT_STR    (0xbeefcafe)

    u16_t *size_array;
    u32_t *addr;
    u32_t ddr_size, buswidth, size;
    volatile u32_t *vaddr;
    u32_t type=RFLD_MCR(dram_type);
    u32_t idx, var;
    DCR_T dcr={.v=DCRrv};

    #define DACDQR  (0xB8001510)
    #define DACDQF  (0xB8001550)
    for (idx=8, var=0; idx<16; idx++) {
        var|=(REG32(DACDQR+idx*4)|REG32(DACDQR+idx*4));
    }
    var&=0xffffff;
    buswidth=(0==var)?8:16;
    ddr_size=0x1000000<<(type); // base size DDR3-64MB, DD2-32MB, DD1-16MB    

    size_array=memctl_get_size_array(type, buswidth);
    addr=memctl_get_verify_addr(&size, type, buswidth);

    /* clean up */
    for (idx=0; idx<size; idx++) {
        DCRrv=(size_array[idx]<<16);
        vaddr=(u32_t *)((addr[idx]&0xFFFFFFF)|0xA0000000);
        *vaddr=INIT_STR;
    }    
    
    DCRrv = (size_array[size-1]<<16);    // important step    
    /* verify size*/
    vaddr=(u32_t *)((addr[size-1]&0xFFFFFFF)|0xA0000000);
    *vaddr=PATT_STR;

    for (idx=0; idx<size; idx++) {
        vaddr=(u32_t *)((addr[idx]&0xFFFFFFF)|0xA0000000);
        if (PATT_STR==*vaddr) {
            ddr_size=ddr_size<<idx;
            break;
        }
    }

    if (idx==size && ddr_size==(0x1000000<<type)) {
        printf("EE: auto size detection failed.\n");
        DCRrv =dcr.v;    // apply the original size
        return 0;
    }

    printf("II: Auto Size Detect... x%d %dMB\n", buswidth, ddr_size>>20);

    //restore correct size 
    DCRrv = (dcr.v&(~DCR_MASK))|(ddr3_16bit_size[idx]<<16);
    
    return 0;
}

MEMCNTLR_SECTION
void mc_result_show_dram_config(void) {
    if (0==meminfo.cntlr_opt->dbg_en) return;
    unsigned int i, j;
    volatile unsigned int *phy_reg;
    const char *dq_edge_str[2] = {"Rising", "Falling"};

    printf("II: MCR(%08x)    ZQPSR(%08x)\n", MCRrv, DDZQPSRrv);
    printf("II: DCR(%08x)    DTR0(%08x)    DTR1(%08x)    DTR2(%08x)\n",
               DCRrv, DTR0rv, DTR1rv, DTR2rv);
    printf("II: DACCR(%08x)  DACSPCR(%08x) DACSPAR(%08x) DACSPSR(%08x)\n",
               DACCRrv, DACSPCRrv, DACSPARrv, DACSPSRrv);

    phy_reg = (unsigned int *)(0xb8001510);

    puts("II: Write Delay (taps):");
    for (i=0; i<16; i++) {
        if ((i%8) == 0) {
            printf("\n    DQ%02d:", i);
        }
        printf("    %02d", (*(phy_reg + i) >> 24) & 0x1f);
    }

    for (j=0; j<2; j++) {
        unsigned int rdly;
        phy_reg = (unsigned int *)(0xb8001510 + j*0x40);
        printf("\nII: Read %s Edge Delay (Max./Current/Min. taps):", dq_edge_str[j]);
        for (i=0; i<16; i++) {
            rdly = (*(phy_reg + i)) & 0x1fffff;
            if ((i%4) == 0) {
                printf("\n    DQ%2d:", i);
            }
            printf("    %02d/%02d/%02d", (rdly >> 16) & 0x1f,
                                          (rdly >>  8) & 0x1f,
                                          (rdly >>  0) & 0x1f);
        }
    }
    puts("\n");

    return;
}

#if 0
MEMCNTLR_SECTION    
void mc_wr_lev(void) {
    if (meminfo.dram_param->dram_type<2) return;   //DDR3 only
    if (meminfo.cntlr_opt->write_lvl_en==0) return;

    DMCR_T dmcr = {.v=DMCRrv};
    DTR0_T dtr0 = {.v=DTR0rv};
    u32_t _dtr = dtr0.v;
    u32_t v=0, rd, _rd=0, r=0;

    puts("II: Write Leveling... ");
    
    // setting CL, CWL and CL_PHY be the same
    dtr0.f.t_cas=dtr0.f.t_cwl=dtr0.f.t_cas_phy=5;
    DTR0rv=dtr0.v;

    // enable write leveling
    dmcr.f.en_wr_leveling=1;
    DMCRrv=dmcr.v;
    
    for (v=0; v<0x1f; v++) 
    {
        CG_MEM_PLL_OE_DIS();
        cg_mem_pll_config_dqs_pi(v);
        CG_MEM_PLL_OE_EN();
        udelay(100);
        REG32(0xA0000000)=0x5A5AA5A5;
        rd=DCFDRR0rv&0xF;
        if (0==_rd && 0!=rd) {
            r=1;  break;
        }            
        _rd=rd;
    }

    // update DQ0~7 and DQ8~15
    CG_MEM_PLL_OE_DIS();
    if(0x1f<=v) {
        r=0; v=0;
        cg_mem_pll_config_dqs_pi(v);
    }
    cg_mem_pll_config_dq_pi((v>=0x10)?0x1f:(v+0xf));
    CG_MEM_PLL_OE_EN();
    udelay(100);
    printf("%s\n", r?"done":"skipped");

    // disable write leveling
    dmcr.f.en_wr_leveling=0;
    DMCRrv=dmcr.v;    

    // restore DTR0
    DTR0rv=_dtr;  
}
#endif

//REG_INIT_FUNC(mc_wr_lev,               27);
REG_INIT_FUNC(mc_pupd_detection,       27);
//REG_INIT_FUNC(memctlc_dram_size_detect, 29);
