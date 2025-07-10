#include <util.h>

void wait_otp_ready(void)
{
    u32_t cnt=0;
    //hw busy status response need 2us
    udelay(2);
    while((REG32(0xbb000020)>>16&0x1)){
        if(++cnt>=800) puts("EE: wait_otp_ready fail\n");
        udelay(1);
        //for write otp busy time need 400us. wait 2x time
    }
}
unsigned short apro_otp_read_entry(unsigned char entry)
{
    u32_t otp_addr=entry;
    u32_t otp_val=0;

    otp_addr = otp_addr<<1;
    REG32(0xbb00001c)= (1<<16)|otp_addr;
    wait_otp_ready();
    otp_val = (REG32(0xbb000020) & 0xFF);

    otp_addr = otp_addr + 1;
    REG32(0xbb00001c)= (1<<16)|otp_addr;
    wait_otp_ready();
    otp_val |= ((REG32(0xbb000020) & 0xFF) << 8);

    return (unsigned short) otp_val;
}

#if 0
//write function.
void __otp_write_entry(u8_t entry, u16_t value)
{
    u32_t opt_addr=entry;

    otp_addr = otp_addr<<1;
    REG32(0xbb000018)= (value & 0xff);
    REG32(0xbb00001c)= (1<<21)|(1<<17)|(1<<16)|otp_addr;
    wait_otp_ready();

    otp_addr = otp_addr+1;
    REG32(0xbb000018)= ((value>>8) & 0xff);
    REG32(0xbb00001c)= (1<<21)|(1<<17)|(1<<16)|otp_addr;
    wait_otp_ready();



    return;
}

#endif

u32_t Read_otp(u16_t addr, u32_t msb, u32_t lsb)
{
    u32_t otp_val;

    otp_val = apro_otp_read_entry(addr);

    u32_t mask_bit = (1<<(msb-lsb+1))-1;

    return (otp_val>>lsb)&mask_bit;

}

void pwr_adj_en_gen2(u32_t ctrl)
{
    if(ctrl){
        REG32(0xbb000190) = (REG32(0xbb000190)&0xFFFF)|(0x2379<<16); //Step 1  -Set 0xbb000190[31:16]  0x2379
    }
    REG32(0xbb00018c) = (REG32(0xbb00018c)&(~(1<<28)))|((ctrl&1)<<28); //Step 2  -Set 0xbb00018c[28]  ctrl
    REG32(0xbb00018c) = (REG32(0xbb00018c)|(1<<27));     //Step 3  -Set 0xbb00018c[27]  0x1
    REG32(0xbb00018c) = (REG32(0xbb00018c)&(~(1<<27))); //Step 4  -Set 0xbb00018c[27]  0x0
}
void table26_gen2(void)
{
    //u32_t val = Read_efuse(8,9,5);
    u32_t val = Read_otp(8,9,5);
    pwr_adj_en_gen2(1);
    REG32(0xbb00018c) = 0x15400000;  //3-1  -Set 0xbb00018c 0x15400000
    REG32(0xbb00018c) = (0x15400000)|((REG32(0xbb000194)&0x80FE)|((val<<8)|(1<<0))); //20211130: 0xFC to 0xEC for bit4=0, to 0x80fe for 98d2
    REG32(0xbb000190) = 0x23790001;
    REG32(0xbb000190) = 0x23790000;
    pwr_adj_en_gen2(0);
}
void table27_gen2(void)
{
    pwr_adj_en_gen2(1);
    REG32(0xbb00018c) = 0x15409055;
    REG32(0xbb000190) = 0x23790001;
    REG32(0xbb000190) = 0x23790000;
    pwr_adj_en_gen2(0);
}
void table28_gen2(void)
{
    //u32_t val = Read_efuse(8,14,10);
    u32_t val = Read_otp(8,14,10);
    pwr_adj_en_gen2(1);
    REG32(0xbb00018c) = 0x15400000;  //3-1  -Set 0xbb00018c 0x15400000
    REG32(0xbb00018c) = (0x15400000)|((REG32(0xbb000194)&0x80fe)|((1<<13)|(val<<8)|(1<<0))); //20211130: 0xFC to 0xEC for bit4=0, to 0x80fe f0r 98d2
    REG32(0xbb000190) = 0x23790001;
    REG32(0xbb000190) = 0x23790000;
    pwr_adj_en_gen2(0);
}

void table29_gen2(void)
{
    pwr_adj_en_gen2(1);
    REG32(0xbb00018c) = 0x1540b055;
    REG32(0xbb000190) = 0x23790001;
    REG32(0xbb000190) = 0x23790000;
    pwr_adj_en_gen2(0);

}

void apro_otp_setup_fcn(void)
{
    u32_t SWLDO_Select = 0;  // 1: SWR mode, 0: LDO mode

    //u32_t ea6 = Read_efuse(6, 15, 0);   // Åª¨úE-Fuse¤¤Address 6ªº­È¨Ã¦s¤JÅÜ¼ÆEA6¤¤
    u32_t ea6 = Read_otp(6, 15, 0); //read OTP address 6 value and save to EA6
    #define EA6_BIT(loc) ((ea6 >>loc)&0x1)
    //#define EA6_BIT(loc) (0)
    u32_t ddr_strap = (REG32(0xbb0002a4)>>2)&0x3;

    printf("II: apro_otp_setup_fcn(%x)\r", REG32(0xbb000198));

    //if(REG32(0xbb000198)&1){ // DDR
    if(1){ // DDR
        if(ddr_strap == 1){
            if(SWLDO_Select){ //SWR mode
                //if(EA6_BIT(1)) table12();
                //else table13();
            }else{ //LDO mode
                if(EA6_BIT(15)) table28_gen2();
                else table29_gen2();
            }
        }else{
            if(SWLDO_Select){ //SWR mode
                //if(EA6_BIT(0)) table10();
                //else table11();
            }else{ //LDO mode
                if(EA6_BIT(14)) table26_gen2();
                else table27_gen2();
            }
        }
    }
    return;
}
