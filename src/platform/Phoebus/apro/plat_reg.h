#ifndef __PLAT_REG_H_
#define __PLAT_REG_H_

/*-----------------------------------------------------
    CHP INFORMATION:
 ----------------------------------------------------*/
typedef union {
    struct {
        unsigned int rtl_id:16; //0
        unsigned int model_char_1st:5; //0
        unsigned int model_char_2nd:5; //0
        unsigned int dummy:2; //0
        unsigned int rtl_vid:4; //0
    } f;
    unsigned int v;
} MODEL_NAME_INFO_T;
#define MODEL_NAME_INFOrv (*((regval)0xbb010000))
#define RMOD_MODEL_NAME_INFO(...) rset(MODEL_NAME_INFO, MODEL_NAME_INFOrv, __VA_ARGS__)
#define RFLD_MODEL_NAME_INFO(fld) (*((const volatile MODEL_NAME_INFO_T *)0xbb010000)).f.fld


typedef union {
    struct {
        unsigned int chip_info_en:4;
        unsigned int reserved:7;
        unsigned int chip_ver:5;
        unsigned int rl_id:16;
    } f;
    unsigned int v;
}CHIP_INFO_T;
#define CHIP_INFOrv (*((regval)0xbb010004))
#define RMOD_CHIP_INFO(...) rset(CHIP_INFO, CHIP_INFOrv, __VA_ARGS__)
#define RFLD_CHIP_INFO(fld) (*((const volatile CHIP_INFO_T *)0xbb010004)).f.fld


/*-----------------------------------------------------
    Hardware Interface:
 ----------------------------------------------------*/
typedef union {
	struct {
		unsigned int no_use31_23:9; //0
		unsigned int pps_sel:1; //0
		unsigned int sc_cd_en:1; //0
		unsigned int pps_en_0:1; //0
		unsigned int oem_en_0:1; //0
		unsigned int dying_en:1; //0
		unsigned int slic_pcm_en:1;
		unsigned int slic_zsi_en:1; //0
		unsigned int slic_isi_en:1; //0
		unsigned int i2c_en:2; //0
		unsigned int no_use12_10:3; //0
		unsigned int mdx_m_en:2; //0
		unsigned int mdx_s_en:1; //0
		unsigned int spi_en:1; //0
		unsigned int uart2_en:1; //0
		unsigned int uart0_en:1; //0
		unsigned int uart1_en:1; //0
		unsigned int uartfc_en:2; //0
		unsigned int uart3_en:1; //0
	} f;
	unsigned int v;
} IO_MODE_EN_T;
#define IO_MODE_ENrv (*((regval)0xbb023014))
#define RMOD_IO_MODE_EN(...) rset(IO_MODE_EN, IO_MODE_ENrv, __VA_ARGS__)
#endif
