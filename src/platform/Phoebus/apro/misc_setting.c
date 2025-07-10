#include <util.h>
#include <apro/misc_setting.h>
#include <apro/cg.h>
#include <apro/plat_reg.h>


//0. stall_setup() comes from SNOF. loader boot0412, dev by TCTsai.
SECTION_RECYCLE void apro_stall_setup(void)
{
    if(REG32(0xb8000250) != 0x11){
        //setup wsstall/datastall = 1/1
        REG32(0xb8000250) = 0x11;
        //trigger watchdog reset
        WDT_CTRLrv = 0;
        WDT_CTRLrv = 0x80000001;
        while(1);
    }
}

SECTION_RECYCLE void
apro_chip_pre_init(void)
{
    MODEL_NAME_INFO_T model_info;

    model_info.v = MODEL_NAME_INFOrv;

    //1. Check which SoC it it.
    if(model_info.f.rtl_id == 0x9607){
        if (model_info.f.rtl_vid == 1){
            _soc.sid = PLR_SID_APRO_GEN2;
        } else {
            _soc.sid = PLR_SID_APRO;
        }
        RMOD_CHIP_INFO(chip_info_en, 0);

        //2. Comes from SNOF. loader boot0412, dev by TCTsai
        apro_stall_setup();

        //3. Enable Memory controller "outstanding ECO"
        RMOD_DCR(eco_rd_buf_mech_iA,1, eco_rd_buf_mech_iA, 1);
        DMCRrv = DMCRrv;

        //4. Set GPIO4 to be enable (GPI), reuquested by Scott Lin
        REG32(0xBB000038) |= (1<<4);

        //5. Enable UART IO PAD
        RMOD_IO_MODE_EN(uart0_en, 1);
    }

    return;
}

u32_t apro_xlat_cpu_freq = 0;
u32_t apro_xlat_ddr3_freq = 0;
u32_t apro_xlat_dram_size_num = 0;

SECTION_RO apro_series_mode_t series_tbl[] = {
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
    {.st = ST_RTL9607EP, .clk= 95, .size= SIZE_NA},
    {.st = NA,           .clk= NA, .size= SIZE_NA},
    {.st = ST_RTL9607CP, .clk= 95, .size= SIZE_NA},
    {.st = ST_RTL9607E,  .clk= 70, .size= SIZE_1Gb}, //idx_20
    {.st = ST_RTL9607C,  .clk= 90, .size= SIZE_1Gb},
    {.st = ST_RTL8198D,  .clk= 90, .size= SIZE_1Gb},
    {.st = ST_RTL9606C,  .clk= 90, .size= SIZE_1Gb},
    {.st = ST_RTL9607C,  .clk= 90, .size= SIZE_2Gb}, //idx_24
    {.st = ST_RTL9603CW, .clk= 90, .size= SIZE_2Gb},
    {.st = ST_RTL8198D,  .clk= 90, .size= SIZE_2Gb},
    {.st = ST_RTL9606C,  .clk= 90, .size= SIZE_2Gb},
    {.st = ST_RTL9607C,  .clk= 90, .size= SIZE_4Gb}, //idx_28
    {.st = ST_RTL9603CW, .clk= 90, .size= SIZE_4Gb},
    {.st = ST_RTL8198D,  .clk= 90, .size= SIZE_4Gb},
    {.st = ST_RTL9606C,  .clk= 90, .size= SIZE_4Gb},
};

SECTION_RO apro_series_mode_gen2_t series_gen2[] = {
    {.st = NA,           .cclk = NA,  .mclk = NA,  .size = SIZE_NA}, //idx_0
    {.st = NA,           .cclk = NA,  .mclk = NA,  .size = SIZE_NA},
    {.st = NA,           .cclk = NA,  .mclk = NA,  .size = SIZE_NA},
    {.st = NA,           .cclk = NA,  .mclk = NA,  .size = SIZE_NA},
    {.st = NA,           .cclk = NA,  .mclk = NA,  .size = SIZE_NA}, //idx_4
    {.st = NA,           .cclk = NA,  .mclk = NA,  .size = SIZE_NA},
    {.st = NA,           .cclk = NA,  .mclk = NA,  .size = SIZE_NA},
    {.st = NA,           .cclk = NA,  .mclk = NA,  .size = SIZE_NA},
    {.st = ST_RTL9606C,  .cclk = 115, .mclk = NA,  .size = SIZE_2Gb}, //idx_8
    {.st = NA,           .cclk = NA,  .mclk = NA,  .size = SIZE_NA},
    {.st = NA,           .cclk = NA,  .mclk = NA,  .size = SIZE_NA},
    {.st = ST_RTL8198DE, .cclk = 125, .mclk = NA,  .size = SIZE_2Gb},
    {.st = NA,           .cclk = NA,  .mclk = NA,  .size = SIZE_NA}, //idx_12
    {.st = NA,           .cclk = NA,  .mclk = NA,  .size = SIZE_NA},
    {.st = NA,           .cclk = NA,  .mclk = NA,  .size = SIZE_NA},
    {.st = NA,           .cclk = NA,  .mclk = NA,  .size = SIZE_NA},
    {.st = ST_RTL8198DE, .cclk = 90,  .mclk = NA,  .size = SIZE_1Gb}, //idx_16
    {.st = ST_RTL9607C,  .cclk = 115, .mclk = NA,  .size = SIZE_1Gb},
    {.st = NA,           .cclk = NA,  .mclk = NA,  .size = SIZE_NA},
    {.st = NA,           .cclk = NA,  .mclk = NA,  .size = SIZE_NA},
    {.st = ST_RTL8198DE, .cclk = 90,  .mclk = 525, .size = SIZE_1Gb}, //idx_20
    {.st = ST_RTL9607C,  .cclk = 115, .mclk = NA,  .size = SIZE_1Gb},
    {.st = ST_RTL8198DE, .cclk = 115, .mclk = NA,  .size = SIZE_1Gb},
    {.st = ST_RTL9606C,  .cclk = 115, .mclk = NA,  .size = SIZE_1Gb},
    {.st = ST_RTL9607C,  .cclk = 115, .mclk = NA,  .size = SIZE_2Gb}, //idx_24
    {.st = ST_RTL8198DE, .cclk = 90,  .mclk = NA,  .size = SIZE_2Gb},
    {.st = ST_RTL8198DE, .cclk = 115, .mclk = NA,  .size = SIZE_2Gb},
    {.st = ST_RTL8198DE, .cclk = 125, .mclk = 800, .size = SIZE_2Gb},
    {.st = ST_RTL9607C,  .cclk = 115, .mclk = NA,  .size = SIZE_4Gb}, //idx_28
    {.st = NA,           .cclk = NA,  .mclk = NA,  .size = SIZE_NA},
    {.st = NA,           .cclk = NA,  .mclk = NA,  .size = SIZE_NA},
    {.st = ST_RTL9606C,  .cclk = 115, .mclk = NA,  .size = SIZE_4Gb},
};

u32_t acquire_series(void)
{
    if (_soc.sid == PLR_SID_APRO) {
        _soc.cid = ((_soc.cid&0xFFFF0000)|(series_tbl[_soc_cid_sct].st));
        apro_xlat_cpu_freq = ((series_tbl[_soc_cid_sct].clk) == NA) ? CPU0_RESET_MHZ:(((series_tbl[_soc_cid_sct].clk)<<1)*5);
        apro_xlat_dram_size_num = series_tbl[_soc_cid_sct].size;
    } else if (_soc.sid == PLR_SID_APRO_GEN2) {
        _soc.cid = ((_soc.cid&0xFFFF0000)|(series_gen2[_soc_cid_sct].st));
        apro_xlat_cpu_freq = ((series_gen2[_soc_cid_sct].cclk) == NA) ? CPU0_RESET_MHZ:(((series_gen2[_soc_cid_sct].cclk)<<1)*5);
        apro_xlat_ddr3_freq = ((series_gen2[_soc_cid_sct].mclk) == NA) ? 0:(series_gen2[_soc_cid_sct].mclk);
        apro_xlat_dram_size_num = series_gen2[_soc_cid_sct].size;
    }
    return _soc_cid_series;
}

u32_t xlat_chip_mode(void)
{
    u32_t st = 0;

    if (_soc.sid == PLR_SID_APRO) {
        st = series_tbl[_soc_cid_sct].st;
    } else if (_soc.sid == PLR_SID_APRO_GEN2) {
        st = series_gen2[_soc_cid_sct].st;
    }

    return st;
}


REG_INIT_FUNC(acquire_series, 9);

