#ifndef __BPSA_REG_MAP_H__
#define __BPSA_REG_MAP_H__


/*-----------------------------------------------------
 Bus Priority Switch Arbiter Register
 -----------------------------------------------------*/
typedef union {
    struct {
        unsigned int mbz_31_1:31;      // 0
        unsigned int pri_swt_en:1;     // 1
    } f;
    unsigned int v;
} BPSA_CTRL_T;
#define BPSA_CTRLrv (*((regval)0xb8001A00))


typedef union {
    struct {
        unsigned int mbz_31_1:31;      // 0
        unsigned int pri_err:1;     // 1
    } f;
    unsigned int v;
} BPSA_DBG_T;
#define BPSA_DBGrv (*((regval)0xb8001A04))


typedef union {
    struct {
        unsigned int bus7_pri:4; // 0x7
        unsigned int bus6_pri:4; // 0x9
        unsigned int bus5_pri:4; // 0x2
        unsigned int bus4_pri:4; // 0x3
        unsigned int bus3_pri:4; // 0x4
        unsigned int bus2_pri:4; // 0x5
        unsigned int bus1_pri:4; // 0xB
        unsigned int bus0_pri:4; // 0xA
    } f;
    unsigned int v;
} BPSA_DFT_PRI0_T;
#define BPSA_DFT_PRI0_REG 0xb8001A08
#define BPSA_DFT_PRI0rv (*((regval)BPSA_DFT_PRI0_REG))
#define BPSA_DFT_PRI0dv (0x792345BA)
#define RMOD_BPSA_DFT_PRI0(...) rset(BPSA_DFT_PRI0,BPSA_DFT_PRI0rv, __VA_ARGS__)
#define RFLD_BPSA_DFT_PRI0(fld) (*((const volatile BPSA_DFT_PRI0_T *)BPSA_DFT_PRI0_REG)).f.fld


typedef union {
    struct {
        unsigned int mbz_31_16:16; // 0x0
        unsigned int bus11_pri:4; // 0x1
        unsigned int bus10_pri:4; // 0x8
        unsigned int bus9_pri:4;  // 0xC
        unsigned int bus8_pri:4;  // 0x6
    } f;
    unsigned int v;
} BPSA_DFT_PRI1_T;
#define BPSA_DFT_PRI1_REG 0xb8001A0C
#define BPSA_DFT_PRI1rv (*((regval)BPSA_DFT_PRI1_REG))
#define BPSA_DFT_PRI1dv (0x000018C6)
#define RMOD_BPSA_DFT_PRI1(...) rset(BPSA_DFT_PRI1,BPSA_DFT_PRI1rv, __VA_ARGS__)
#define RFLD_BPSA_DFT_PRI1(fld) (*((const volatile BPSA_DFT_PRI1_T *)BPSA_DFT_PRI1_REG)).f.fld


#define B0_PSAR0rv (*((regval)0xb8001A10))
#define B0_PSAR1rv (*((regval)0xb8001A14))
#define B1_PSAR0rv (*((regval)0xb8001A18))   
#define B1_PSAR1rv (*((regval)0xb8001A1C))
#define B2_PSAR0rv (*((regval)0xb8001A20))
#define B2_PSAR1rv (*((regval)0xb8001A24))
#define B3_PSAR0rv (*((regval)0xb8001A28))   
#define B3_PSAR1rv (*((regval)0xb8001A2C))
#define B4_PSAR0rv (*((regval)0xb8001A30))
#define B4_PSAR1rv (*((regval)0xb8001A34))
#define B5_PSAR0rv (*((regval)0xb8001A38))   
#define B5_PSAR1rv (*((regval)0xb8001A3C))
#define B6_PSAR0rv (*((regval)0xb8001A40))
#define B6_PSAR1rv (*((regval)0xb8001A44))
#define B7_PSAR0rv (*((regval)0xb8001A48))   
#define B7_PSAR1rv (*((regval)0xb8001A4C))
#define B8_PSAR0rv (*((regval)0xb8001A50))
#define B8_PSAR1rv (*((regval)0xb8001A54))
#define B9_PSAR0rv (*((regval)0xb8001A58))   
#define B9_PSAR1rv (*((regval)0xb8001A5C))
#define B10_PSAR0rv (*((regval)0xb8001A60))
#define B10_PSAR1rv (*((regval)0xb8001A64))
#define B11_PSAR0rv (*((regval)0xb8001A68))   
#define B11_PSAR1rv (*((regval)0xb8001A6C))


#endif //__BPSA_REG_MAP_H__


