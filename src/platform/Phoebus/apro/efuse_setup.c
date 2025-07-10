#include <util.h>

void wait_efuse_ready(void)
{
    u32_t cnt=0;
    udelay(1);
    while((REG32(0xbb000020)>>16&0x1)){
        if(++cnt>=500) puts("EE: wait_efuse_ready fail\n");
    }
}

u16_t apro_efuse_read_entry(unsigned char entry)
{
	REG32(0xbb00001c)= (1<<16)|entry;
	wait_efuse_ready();
	return (REG32(0xbb000020) & 0xFFFF);
}

u32_t Read_efuse(u16_t addr, u32_t msb, u32_t lsb)
{
    REG32(0xbb00001c) = (1<<16)|(addr<<0);
    wait_efuse_ready();
    u32_t mask_bit = (1<<(msb-lsb+1))-1;
    return (REG32(0xbb000020)>>lsb)&mask_bit;
}

u16_t Read_GPHY(u32_t phyid, u16_t OCPAddr)
{
    REG32(0xbb000004) = (1<<21)|((phyid&0x1F)<<16)|(OCPAddr<<0);
    wait_efuse_ready();
    return (REG32(0xbb000008)&0xFFFF);
}
void Write_GPHY(u32_t phyid, u16_t OCPAddr, u32_t data)
{
    REG32(0xbb000000) = (REG32(0xbb000000)&0xFFFF0000)|data;
    REG32(0xbb000004) = (3<<21)|((phyid&0x1F)<<16)|(OCPAddr<<0);
    wait_efuse_ready();
}
void pwr_adj_en (u32_t ctrl)
{
    if(ctrl){
        REG32(0xbb000190) = (REG32(0xbb000190)&0xFFFF)|(0x2379<<16); //Step 1  -Set 0xbb000190[31:16]  0x2379
    }
    REG32(0xbb00018c) = (REG32(0xbb00018c)&(~(1<<28)))|((ctrl&1)<<28); //Step 2  -Set 0xbb00018c[28]  ctrl
    REG32(0xbb00018c) = (REG32(0xbb00018c)&(~(1<<27))); //Step 4  -Set 0xbb00018c[27]  0x0
    REG32(0xbb00018c) = (REG32(0xbb00018c)|(1<<27));     //Step 3  -Set 0xbb00018c[27]  0x1
}
void SWR_freq_patch(void)
{
    u32_t val = Read_efuse(8,4,0);

    REG32(0xbb00018c) = 0x3d410000;  //3-1	-Set 0xbb00018c 0x3d400000
    u16_t temp = ((REG32(0xbb000194)&(~0x1F))&0xFFFF)|val;
    REG32(0xbb00018c) = (0x3d41<<16)|temp;
    REG32(0xbb000190) = 0x23790001;
}
void table10(void)
{
    u32_t val = Read_efuse(7,4,0);
    pwr_adj_en(1);
    REG32(0xbb00018c) = 0x3d400000;  //3-1	-Set 0xbb00018c 0x3d400000
    u16_t temp = (REG32(0xbb000194)&0x80FC)|((val<<8)|(2<<0));
    REG32(0xbb00018c) = (0x3d40<<16)|temp;
    REG32(0xbb000190) = 0x23790001;
    SWR_freq_patch();
    pwr_adj_en(0);
}
void table11(void)
{
    pwr_adj_en(1);
    REG32(0xbb00018c) = 0x3d400956;
    REG32(0xbb000190) = 0x23790001;
    pwr_adj_en(0);

}
void table12(void)
{
    u32_t val = Read_efuse(7,9,5);
    pwr_adj_en(1);
    REG32(0xbb00018c) = 0x3d400000;  //3-1	-Set 0xbb00018c 0x3d400000
    u16_t temp = (REG32(0xbb000194)&0x80FC)|((1<<13)|(val<<8)|(2<<0));
    REG32(0xbb00018c) = (0x3d40<<16)|temp;
    REG32(0xbb000190) = 0x23790001;
    SWR_freq_patch();
    pwr_adj_en(0);

}
void table13(void)
{
    pwr_adj_en(1);
    REG32(0xbb00018c) = 0x3d402956;
    REG32(0xbb000190) = 0x23790001;
    pwr_adj_en(0);
}
void table14(void)
{
    u32_t target = Read_efuse(7,14,10);
    pwr_adj_en(1);
    REG32(0xbb00018c) = 0x3d400000;  //3-1	-Set 0xbb00018c 0x3d400000
    u16_t reg194 = (REG32(0xbb000194)&0xFFFF);
    u32_t cur = (reg194>>8)&0x1F;

    if(target > cur){
        do{
            cur++;
            REG32(0xbb00018c) = (0x3d40<<16)|((reg194&0x80FC)|((3<<13)|(cur<<8)));
            REG32(0xbb000190) = 0x23790001;
        }while(target > cur);
    }else if(target < cur){
        do{
            cur--;
            REG32(0xbb00018c) = (0x3d40<<16)|((reg194&0x80FC)|((3<<13)|(cur<<8)));
            REG32(0xbb000190) = 0x23790001;
        }while(target < cur);
    }
    SWR_freq_patch();
    pwr_adj_en(0);
    REG32(0xbb00018c) = 0x2d400000;
}
void table26(void)
{
    u32_t val = Read_efuse(8,9,5);
    pwr_adj_en(1);
    REG32(0xbb00018c) = 0x1d400000;  //3-1  -Set 0xbb00018c 0x1d400000
    REG32(0xbb00018c) = (0x1d400000)|((REG32(0xbb000194)&0x00EC)|((val<<8)|(1<<0))); //20171214: 0xFC to 0xEC for bit4=0
    REG32(0xbb000190) = 0x23790001;
    pwr_adj_en(0);
}
void table27(void)
{
    pwr_adj_en(1);
//    REG32(0xbb00018c) = 0x1d400955;
    REG32(0xbb00018c) = 0x1d400945; //20171214: bit4=0
    REG32(0xbb000190) = 0x23790001;
    pwr_adj_en(0);
}
void table28(void)
{
    u32_t val = Read_efuse(8,14,10);
    pwr_adj_en(1);
    REG32(0xbb00018c) = 0x1d400000;  //3-1  -Set 0xbb00018c 0x1d400000
    REG32(0xbb00018c) = (0x1d400000)|((REG32(0xbb000194)&0x00EC)|((1<<13)|(val<<8)|(1<<0))); //20171214: 0xFC to 0xEC for bit4=0
    REG32(0xbb000190) = 0x23790001;
    pwr_adj_en(0);
}
void table29(void)
{
    pwr_adj_en(1);
//    REG32(0xbb00018c) = 0x1d402955;
    REG32(0xbb00018c) = 0x1d402945; //20171214: bit4=0
    REG32(0xbb000190) = 0x23790001;
    pwr_adj_en(0);
}

void __gphy_sub(u32_t addr, u32_t *efuse)
{
#if 0 //Marked at 2017-10-05
    u16_t val = Read_efuse(efuse[0], 15, 0);
    Write_GPHY (addr, 0xbcdc, val);
#endif
    u32_t model_name = (REG32(0xBB010000)>>4)&0x3; //0=MP, 1=ES
    u16_t val;
    if(0 == model_name){
        val = Read_efuse (efuse[0], 15, 0);
        //Added at 2017-11-13, refered to the E-mail from Arshian SD2
        Write_GPHY (addr, 0xbcde, val);
        Write_GPHY (addr, 0xbcdc, val);
    }else if(1 == model_name){
        val = Read_efuse (efuse[1], 15, 0);
        //Added at 2017-10-06, refered to the E-mail from SD2
        u8_t off0 = (val>>0)&0xF;
        u8_t off1 = (val>>4)&0xF;
        u8_t off2 = (val>>8)&0xF;
        u8_t off3 = (val>>12)&0xF;
        off0 = (off0>=8)?0xF:(off0+7);
        off1 = (off1>=8)?0xF:(off1+7);
        off2 = (off2>=8)?0xF:(off2+7);
        off3 = (off3>=8)?0xF:(off3+7);
        val = (off3<<12) | (off2<<8) | (off1<<4) | off0;
        Write_GPHY (addr, 0xbcde, val);
        Write_GPHY (addr, 0xbcdc, val);
    }

    val = Read_efuse (efuse[2], 15, 0);
    Write_GPHY(addr, 0xbce0, val);
    Write_GPHY(addr, 0xbce2, val);

    val = Read_efuse (efuse[3], 15, 0);
    Write_GPHY (addr, 0xbcac, val);
}

void table15(void)
{
    u32_t efuse_addr[4] = {23,18,13,28};
    __gphy_sub(0x0, efuse_addr);
}
void table16(void)
{
    u32_t efuse_addr[4] = {22,17,12,27};
    __gphy_sub(0x1, efuse_addr);
}
void table17(void)
{
    u32_t efuse_addr[4] = {21,16,11,26};
    __gphy_sub(0x2, efuse_addr);
}

void table18(void)
{
    u32_t efuse_addr[4] = {20,15,10,25};
    __gphy_sub(0x3, efuse_addr);
}
void table19(void)
{
    u32_t efuse_addr[4] = {19,14,9,24};
    __gphy_sub(0x4, efuse_addr);
}
void table20(void)
{
    Write_GPHY(0x0, 0xbcfc, Read_efuse(33, 15, 0));
}
void table21(void)
{
    Write_GPHY(0x1, 0xbcfc, Read_efuse(32, 15, 0));
}
void table22(void)
{
    Write_GPHY(0x2, 0xbcfc, Read_efuse(31, 15, 0));
}
void table23(void)
{
    Write_GPHY(0x3, 0xbcfc, Read_efuse(30, 15, 0));
}
void table24(void)
{
    Write_GPHY(0x4, 0xbcfc, Read_efuse(29, 15, 0));
}
void table25(void)
{
    u16_t temp = Read_GPHY(0x0, 0xa4a2)&(~(1<<8));
    Write_GPHY(0x0, 0xa4a2, temp);

    temp = Read_GPHY(0x1, 0xa4a2)&(~(1<<8));
    Write_GPHY(0x1, 0xa4a2, temp);

    temp = Read_GPHY(0x2, 0xa4a2)&(~(1<<8));
    Write_GPHY(0x2, 0xa4a2, temp);

    temp = Read_GPHY(0x3, 0xa4a2)&(~(1<<8));
    Write_GPHY(0x3, 0xa4a2, temp);

    temp = Read_GPHY(0x4, 0xa4a2)&(~(1<<8));
    Write_GPHY(0x4, 0xa4a2, temp);
}


void apro_efuse_setup_fcn(void)
{
    u32_t SWLDO_Select = 0;  // 1: SWR mode, 0: LDO mode

    u32_t ea6 = Read_efuse(6, 15, 0);   // 讀取E-Fuse中Address 6的值並存入變數EA6中
    #define EA6_BIT(loc) ((ea6 >>loc)&0x1)
    u32_t ddr_strap = (REG32(0xbb0002a4)>>2)&0x3;

    if(REG32(0xbb000198)&1){ // DDR
        if(ddr_strap == 1){
            if(SWLDO_Select){ //SWR mode
                if(EA6_BIT(1)) table12();
                else table13();
            }else{ //LDO mode
                if(EA6_BIT(15)) table28();
                else table29();
            }
        }else{
            if(SWLDO_Select){ //SWR mode
                if(EA6_BIT(0)) table10();
                else table11();
            }else{ //LDO mode
                if(EA6_BIT(14)) table26();
                else table27();
            }
        }
    }else{ // Core power, SWR
        if(EA6_BIT(2)) table14();
    }

    // ------Stop procefure B------

    // ------Start procedure C------
    if(EA6_BIT(3)) table15();
    if(EA6_BIT(4)) table16();
    if(EA6_BIT(5)) table17();
    if(EA6_BIT(6)) table18();
    if(EA6_BIT(7)) table19();
    if(EA6_BIT(8)) table20();
    if(EA6_BIT(9)) table21();
    if(EA6_BIT(10)) table22();
    if(EA6_BIT(11)) table23();
    if(EA6_BIT(12)) table24();
    if(EA6_BIT(13)) table25();

    // ------Stop procedure C------

}
