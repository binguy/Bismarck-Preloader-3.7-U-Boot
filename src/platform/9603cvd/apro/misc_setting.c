#include <util.h>
#include <apro/misc_setting.h>
#include <apro/cg.h>

//0. stall_setup() comes from SNOF. loader boot0412, dev by TCTsai.
SECTION_RECYCLE void stall_setup(void)
{
    if(REG32(0xb8000250) != 0x11){
        //setup wsstall/datastall = 1/1
        REG32(0xb8000250) = 0x11;
        //trigger watchdog reset
        WDT_CTRLrv = 0;
        if(_soc.sid) WDT_CTRLrv = 0x80000001;
        else WDT_CTRLrv = 0x80000002;
        while(1);
    }
}

SECTION_RECYCLE void
apro_chip_pre_init(void)
{
    //0. Comes from SNOF. loader boot0412, dev by TCTsai
    stall_setup();

    //1. Enable Memory controller "outstanding ECO"
    RMOD_DCR(eco_rd_buf_mech_iA,1, eco_rd_buf_mech_iA, 1);
    DMCRrv = DMCRrv;

    //2. Set GPIO4 to be enable (GPI), reuquested by Scott Lin
    REG32(0xBB000038) |= (1<<4);
}

SECTION_RO series_mode_t series_tbl[] = {
    {.st = ST_RTL9603CT, .clk= 60, .size= SIZE_NA},  //idx_0
    {.st = ST_RTL9603C,  .clk= 95, .size= SIZE_512Mb},
    {.st = NA,           .clk= NA, .size= SIZE_NA},
    {.st = NA,           .clk= NA, .size= SIZE_NA},
    {.st = ST_RTL9603C,  .clk= 95, .size= SIZE_1Gb}, //idx_4
    {.st = NA,           .clk= NA, .size= SIZE_NA},
    {.st = NA,           .clk= NA, .size= SIZE_NA},
    {.st = NA,           .clk= NA, .size= SIZE_NA},
    {.st = ST_RTL9603C,  .clk= 95, .size= SIZE_2Gb}, //idx_8
    {.st = ST_RTL9603CE, .clk= 95, .size= SIZE_NA},
    {.st = NA,           .clk= NA, .size= SIZE_NA},
    {.st = NA,           .clk= NA, .size= SIZE_NA},
    {.st = NA,           .clk= NA, .size= SIZE_NA}, //idx_12
    {.st = NA,           .clk= NA, .size= SIZE_NA},
    {.st = NA,           .clk= NA, .size= SIZE_NA},
    {.st = NA,           .clk= NA, .size= SIZE_NA},
    {.st = ST_RTL9603CP, .clk= 95, .size= SIZE_NA}, //idx_16
    {.st = NA,           .clk= NA, .size= SIZE_NA},
    {.st = NA,           .clk= NA, .size= SIZE_NA},
    {.st = NA,           .clk= NA, .size= SIZE_NA},
    {.st = NA,           .clk= NA, .size= SIZE_NA}, //idx_20
    {.st = NA,           .clk= NA, .size= SIZE_NA},
    {.st = NA,           .clk= NA, .size= SIZE_NA},
    {.st = NA,           .clk= NA, .size= SIZE_NA},
    {.st = NA,           .clk= NA, .size= SIZE_NA}, //idx_24
    {.st = ST_RTL9603CW, .clk= 90, .size= SIZE_2Gb},
    {.st = NA,           .clk= NA, .size= SIZE_NA},
    {.st = NA,           .clk= NA, .size= SIZE_NA},
    {.st = NA,           .clk= NA, .size= SIZE_NA}, //idx_28
    {.st = ST_RTL9603CW, .clk= 90, .size= SIZE_4Gb},
    {.st = NA,           .clk= NA, .size= SIZE_NA},
    {.st = NA,           .clk= NA, .size= SIZE_NA},
};

u32_t acquire_series(void)
{
    _soc.cid = ((_soc.cid&(0xFFFF<<16))|(series_tbl[_soc_cid_sct].st));
#if 0
    printf("DD: The SCT is 0x%x",_soc_cid_sct);
    switch(_soc_cid_series){
        case ST_RTL9603C:
             puts(" ST_RTL9603C\n");
             break;
        case ST_RTL9603CP:
            puts(" ST_RTL9603CP\n");
            break;
        case ST_RTL9607C:
            puts(" ST_RTL9607C\n");
            break;
        case ST_RTL9607CP:
            puts(" ST_RTL9607CP\n");
            break;
        case ST_RTL9607E:
            puts(" ST_RTL9607E\n");
            break;
        case ST_RTL9607EP:
            puts(" ST_RTL9607EP\n");
            break;
        default:
            puts(" N.A.\n");
            break;
    }
#endif
    return _soc_cid_series;
}

u32_t apro_xlat_cpu_freq(void)
{
    if((series_tbl[_soc_cid_sct].clk) == NA){
        return CPU_CLK_DEFAULT;
    }else{
        return (((series_tbl[_soc_cid_sct].clk)<<1)*5);
    }
}

u32_t apro_xlat_dram_size_num(void)
{
    acquire_series();
    return series_tbl[_soc_cid_sct].size;
}
