#ifndef _REGISTER_MAP_H_
#define _REGISTER_MAP_H_
#include <reg_skc35.h>
#include <dram/memcntlr_reg.h>


/*-----------------------------------------------------
    CHIP INFORMATION:
 ----------------------------------------------------*/
typedef union {
	struct {
		unsigned int en:4;
		unsigned int no_use:23;
		unsigned int sct:5;
	} f;
	unsigned int v;
} SCT_T;
#define SCTrv (*((regval)0xbb010008))
#define RMOD_SCT(...) rset(SCT, SCTrv, __VA_ARGS__)
#define RFLD_SCT(fld) (*((const volatile SCT_T *)0xbb010008)).f.fld

/*-----------------------------------------------------
    Extraced from file_IP_ENABLE.xml
    -----------------------------------------------------*/
typedef union {
	struct {
		u32_t en_usb2phy_p1:1;
		u32_t en_usb2phy_p0:1;
		u32_t en_usb3phy:1;
		u32_t reserved:1;
		u32_t pcm_spd_mode:1;
		u32_t voip_int_sel:2;
		u32_t dbg_grpsel:5;
		u32_t dbg_sgrpsel:4;
		u32_t en_nandfsh_ctrl:1;
		u32_t en_nfbi:1;
		u32_t en_p2usbhost:1;
		u32_t en_voipfft:1;
		u32_t en_voipacc:1;
		u32_t en_ipsec:1;
		u32_t en_gdma1:1;
		u32_t en_gdma0:1;
		u32_t en_pcie0:1;
		u32_t en_pcie1:1;
		u32_t en_p0_usbhost:2;
		u32_t en_p1_usbhost:1;
		u32_t en_pcm:1;
		u32_t en_gmac:1;
		u32_t en_prei_voip:1;
	} f;
	u32_t v;
} IP_EN_CTRL_T;
#define IP_EN_CTRLrv (*((regval)0xb8000600))
#define RFLD_IP_EN_CTRL(fld) (*((const volatile IP_EN_CTRL_T *)0xb8000600)).f.fld


typedef union {
	struct {
		u32_t usb3_sel_axi:1;
		u32_t usb2host_mux_sel:2;
		u32_t mbz_0:2;
		u32_t en_usb3_axi:1;
		u32_t en_pbo_es:1;
		u32_t pcie_11ac_rdlh_link_up:1;
		u32_t pcie_11n_rdlh_link_up:1;
		u32_t en_gdma8:1;
		u32_t flowbase_bus_sel:1;
		u32_t en_gmac2:1;
		u32_t en_gmac1:1;
		u32_t en_gdma7:1;
		u32_t ocp_extdbg_sel:7;
		u32_t en_gdma6:1;
		u32_t en_gdma5:1;
		u32_t en_gdma4:1;
		u32_t en_gdma3:1;
		u32_t en_gdma2:1;
		u32_t en_wrap_pbuf:1;
		u32_t en_spi_nand:1;
		u32_t dbg_timeout_sel:4;
	} f;
	u32_t v;
} NEW_IP_EN_CTRL_T;
#define NEW_IP_EN_CTRLrv (*((regval)0xb800063C))
#define RFLD_NEW_IP_EN_CTRL(fld) (*((const volatile NEW_IP_EN_CTRL_T *)0xb800063C)).f.fld



/*-----------------------------------------------------
	Extraced from file_UART.xml
	-----------------------------------------------------*/
typedef union {
	struct {
		unsigned int rbr_thr_dll:8; //0
		unsigned int mbz_0:24; //0
	} f;
	unsigned int v;
} UART_PBR_THR_DLL_T;
#define UART_PBR_THR_DLLrv (*((regval)0xb8002000))
#define UART_PBR_THR_DLLdv (0x00000000)
#define RMOD_UART_PBR_THR_DLL(...) rset(UART_PBR_THR_DLL, UART_PBR_THR_DLLrv, __VA_ARGS__)
#define RIZS_UART_PBR_THR_DLL(...) rset(UART_PBR_THR_DLL, 0, __VA_ARGS__)
#define RFLD_UART_PBR_THR_DLL(fld) (*((const volatile UART_PBR_THR_DLL_T *)0xb8002000)).f.fld

typedef union {
	struct {
		unsigned int ier_dlm:8; //0
		unsigned int mbz_0:24; //0
	} f;
	unsigned int v;
} UART_IER_DLM_T;
#define UART_IER_DLMrv (*((regval)0xb8002004))
#define UART_IER_DLMdv (0x00000000)
#define RMOD_UART_IER_DLM(...) rset(UART_IER_DLM, UART_IER_DLMrv, __VA_ARGS__)
#define RIZS_UART_IER_DLM(...) rset(UART_IER_DLM, 0, __VA_ARGS__)
#define RFLD_UART_IER_DLM(fld) (*((const volatile UART_IER_DLM_T *)0xb8002004)).f.fld

typedef union {
	struct {
		unsigned int iir_fcr_1:2; //3
		unsigned int mbz_0:2; //0
		unsigned int iir_fcr_0:4; //1
		unsigned int mbz_1:24; //0
	} f;
	unsigned int v;
} UART_IIR_FCR_T;
#define UART_IIR_FCRrv (*((regval)0xb8002008))
#define UART_IIR_FCRdv (0xc1000000)
#define RMOD_UART_IIR_FCR(...) rset(UART_IIR_FCR, UART_IIR_FCRrv, __VA_ARGS__)
#define RIZS_UART_IIR_FCR(...) rset(UART_IIR_FCR, 0, __VA_ARGS__)
#define RFLD_UART_IIR_FCR(fld) (*((const volatile UART_IIR_FCR_T *)0xb8002008)).f.fld

typedef union {
	struct {
		unsigned int dlab:1; //0
		unsigned int brk:1; //0
		unsigned int eps:2; //0
		unsigned int pen:1; //0
		unsigned int stb:1; //0
		unsigned int wls_1:1; //1
		unsigned int wls_0:1; //1
		unsigned int mbz_0:24; //0
	} f;
	unsigned int v;
} UART_LCR_T;
#define UART_LCRrv (*((regval)0xb800200c))
#define UART_LCRdv (0x03000000)
#define RMOD_UART_LCR(...) rset(UART_LCR, UART_LCRrv, __VA_ARGS__)
#define RIZS_UART_LCR(...) rset(UART_LCR, 0, __VA_ARGS__)
#define RFLD_UART_LCR(fld) (*((const volatile UART_LCR_T *)0xb800200c)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:1; //0
		unsigned int lxclk_sel:1; //0
		unsigned int afe:1; //0
		unsigned int loop:1; //0
		unsigned int out2:1; //0
		unsigned int out1:1; //0
		unsigned int rts:1; //0
		unsigned int dtr:1; //0
		unsigned int mbz_1:24; //0
	} f;
	unsigned int v;
} UART_MCR_T;
#define UART_MCRrv (*((regval)0xb8002010))
#define UART_MCRdv (0x00000000)
#define RMOD_UART_MCR(...) rset(UART_MCR, UART_MCRrv, __VA_ARGS__)
#define RIZS_UART_MCR(...) rset(UART_MCR, 0, __VA_ARGS__)
#define RFLD_UART_MCR(fld) (*((const volatile UART_MCR_T *)0xb8002010)).f.fld

typedef union {
	struct {
		unsigned int rfe:1; //0
		unsigned int temt:1; //1
		unsigned int thre:1; //1
		unsigned int bi:1; //0
		unsigned int fe:1; //0
		unsigned int pe:1; //0
		unsigned int oe:1; //0
		unsigned int dr:1; //0
		unsigned int mbz_0:24; //0
	} f;
	unsigned int v;
} UART_LSR_T;
#define UART_LSRrv (*((regval)0xb8002014))
#define UART_LSRdv (0x60000000)
#define RMOD_UART_LSR(...) rset(UART_LSR, UART_LSRrv, __VA_ARGS__)
#define RIZS_UART_LSR(...) rset(UART_LSR, 0, __VA_ARGS__)
#define RFLD_UART_LSR(fld) (*((const volatile UART_LSR_T *)0xb8002014)).f.fld

typedef union {
	struct {
		unsigned int dcts:1; //0
		unsigned int ddsr:1; //0
		unsigned int teri:1; //0
		unsigned int ddcd:1; //1
		unsigned int cts:1; //0
		unsigned int dsr:1; //0
		unsigned int ri:1; //0
		unsigned int dcd:1; //0
		unsigned int mbz_0:24; //0
	} f;
	unsigned int v;
} UART_MSR_T;
#define UART_MSRrv (*((regval)0xb8002018))
#define UART_MSRdv (0x10000000)
#define RMOD_UART_MSR(...) rset(UART_MSR, UART_MSRrv, __VA_ARGS__)
#define RIZS_UART_MSR(...) rset(UART_MSR, 0, __VA_ARGS__)
#define RFLD_UART_MSR(fld) (*((const volatile UART_MSR_T *)0xb8002018)).f.fld

/*-----------------------------------------------------
	WDOG TIMER.xml
	-----------------------------------------------------*/
typedef union {
	struct {
		unsigned int wdt_kick:1; //0
		unsigned int mbz_0:31; //0
	} f;
	unsigned int v;
} WDTCNTRR_T;
#define WDTCNTRRrv (*((regval)0xb8003260))
#define WDTCNTRRdv (0x00000000)
#define RMOD_WDTCNTRR(...) rset(WDTCNTRR, WDTCNTRRrv, __VA_ARGS__)
#define RIZS_WDTCNTRR(...) rset(WDTCNTRR, 0, __VA_ARGS__)
#define RFLD_WDTCNTRR(fld) (*((const volatile WDTCNTRR_T *)0xb8003260)).f.fld

typedef union {
	struct {
		unsigned int ph1_ip:1; //0
		unsigned int ph2_ip:1; //0
		unsigned int mbz_0:30; //0
	} f;
	unsigned int v;
} WDTINTRR_T;
#define WDTINTRRrv (*((regval)0xb8003264))
#define WDTINTRRdv (0x00000000)
#define RMOD_WDTINTRR(...) rset(WDTINTRR, WDTINTRRrv, __VA_ARGS__)
#define RIZS_WDTINTRR(...) rset(WDTINTRR, 0, __VA_ARGS__)
#define RFLD_WDTINTRR(fld) (*((const volatile WDTINTRR_T *)0xb8003264)).f.fld

typedef union {
	struct {
		unsigned int wdt_e:1; //0
		unsigned int wdt_clk_sc:2; //0
		unsigned int mbz_0:2; //0
		unsigned int ph1_to:5; //0
		unsigned int mbz_1:2; //0
		unsigned int ph2_to:5; //0
		unsigned int mbz_2:13; //0
		unsigned int wdt_reset_mode:2; //0
	} f;
	unsigned int v;
} WDT_CTRL_T;
#define WDT_CTRLrv (*((regval)0xb8003268))
#define WDT_CTRLdv (0x00000000)
#define RMOD_WDT_CTRL(...) rset(WDT_CTRL, WDT_CTRLrv, __VA_ARGS__)
#define RIZS_WDT_CTRL(...) rset(WDT_CTRL, 0, __VA_ARGS__)
#define RFLD_WDT_CTRL(fld) (*((const volatile WDT_CTRL_T *)0xb8003268)).f.fld


/*-----------------------------------------------------
	OCP Bus Timeout Monitor
	-----------------------------------------------------*/
typedef union {
	struct {
		unsigned int to_ctrl_en:1; //1
		unsigned int err_indcat:1; //0
		unsigned int to_ctrl_thr:4; //7
		unsigned int mbz_0:26; //0
	} f;
	unsigned int v;
} OCP0_TO_CTRL_T;
#define OCP0_TO_CTRLrv (*((regval)0xb8005100))
#define RMOD_OCP0_TO_CTRL(...) rset(OCP0_TO_CTRL, OCP0_TO_CTRLrv, __VA_ARGS__)

typedef union {
	struct {
		unsigned int to_ip:2; //0
		unsigned int mbz_0:30; //0
	} f;
	unsigned int v;
} OCP0_TO_INTR_T;
#define OCP0_TO_INTRrv (*((regval)0xb8005104))
#define RMOD_OCP0_TO_INTR(...) rset(OCP0_TO_INTR, OCP0_TO_INTRrv, __VA_ARGS__)

typedef union {
	struct {
		unsigned int to_addr:32; //0
	} f;
	unsigned int v;
} OCP0_TO_MONT_ADDR_T;
#define OCP0_TO_MONT_ADDRrv (*((regval)0xb8005108))
#define RMOD_OCP0_TO_MONT_ADDR(...) rset(OCP0_TO_MONT_ADDR, OCP0_TO_MONT_ADDRrv, __VA_ARGS__)


typedef union {
	struct {
		unsigned int to_ctrl_en:1; //1
		unsigned int err_indcat:1; //0
		unsigned int to_ctrl_thr:4; //7
		unsigned int mbz_0:26; //0
	} f;
	unsigned int v;
} OCP1_TO_CTRL_T;
#define OCP1_TO_CTRLrv (*((regval)0xb8005110))
#define RMOD_OCP1_TO_CTRL(...) rset(OCP1_TO_CTRL, OCP1_TO_CTRLrv, __VA_ARGS__)

typedef union {
	struct {
		unsigned int to_ip:2; //0
		unsigned int mbz_0:30; //0
	} f;
	unsigned int v;
} OCP1_TO_INTR_T;
#define OCP1_TO_INTRrv (*((regval)0xb8005114))
#define RMOD_OCP1_TO_INTR(...) rset(OCP1_TO_INTR, OCP1_TO_INTRrv, __VA_ARGS__)

typedef union {
	struct {
		unsigned int to_addr:32; //0
	} f;
	unsigned int v;
} OCP1_TO_MONT_ADDR_T;
#define OCP1_TO_MONT_ADDRrv (*((regval)0xb8005118))
#define RMOD_OCP1_TO_MONT_ADDR(...) rset(OCP1_TO_MONT_ADDR, OCP1_TO_MONT_ADDRrv, __VA_ARGS__)


/*-----------------------------------------------------
	LX_P Bus Timeout Monitor
	-----------------------------------------------------*/
typedef union {
	struct {
		unsigned int to_ctrl_en:1; //1
		unsigned int to_ctrl_thr:3; //7
		unsigned int mbz_0:28; //0
	} f;
	unsigned int v;
} LXP_TO_CTRL_T;
#define LXP_TO_CTRLrv (*((regval)0xb8005200))
#define LXP_TO_CTRLdv (0xF0000000)
#define RMOD_LXP_TO_CTRL(...) rset(LXP_TO_CTRL, LXP_TO_CTRLrv, __VA_ARGS__)
#define RFLD_LXP_TO_CTRL(fld) (*((const volatile LXP_TO_CTRL_T *)0xb8005200)).f.fld

typedef union {
	struct {
		unsigned int to_ip:2; //0
		unsigned int mbz_0:30; //0
	} f;
	unsigned int v;
} LXP_TO_INTR_T;
#define LXP_TO_INTRrv (*((regval)0xb8005204))
#define LXP_TO_INTRdv (0x00000000)
#define RMOD_LXP_TO_INTR(...) rset(LXP_TO_INTR, LXP_TO_INTRrv, __VA_ARGS__)
#define RFLD_LXP_TO_INTR(fld) (*((const volatile LXP_TO_INTR_T *)0xb8005204)).f.fld

typedef union {
	struct {
		unsigned int to_addr:32; //0
	} f;
	unsigned int v;
} LXP_TO_MONT_ADDR_T;
#define LXP_TO_MONT_ADDRrv (*((regval)0xb8005208))
#define LXP_TO_MONT_ADDRdv (0x00000000)
#define RMOD_LXP_TO_MONT_ADDR(...) rset(LXP_TO_MONT_ADDR, TO_MONT_ADDRrv, __VA_ARGS__)
#define RFLD_LXP_TO_MONT_ADDR(fld) (*((const volatile LXP_TO_MONT_ADDR_T *)0xb8005208)).f.fld


/*-----------------------------------------------------
	Mater LX_0 Bus Timeout Monitor
	-----------------------------------------------------*/
typedef union {
	struct {
		unsigned int to_ctrl_en:1; //1
		unsigned int to_ctrl_thr:3; //7
		unsigned int mbz_0:28; //0
	} f;
	unsigned int v;
} LX0_M_TO_CTRL_T;
#define LX0_M_TO_CTRLrv (*((regval)0xb8005210))
#define LX0_M_TO_CTRLdv (0xF0000000)
#define RMOD_LX0_M_TO_CTRL(...) rset(LX0_M_TO_CTRL, LX0_M_TO_CTRLrv, __VA_ARGS__)
#define RFLD_LX0_M_TO_CTRL(fld) (*((const volatile LX0_M_TO_CTRL_T *)0xb8005210)).f.fld

typedef union {
	struct {
		unsigned int to_ip:2; //0
		unsigned int mbz_0:30; //0
	} f;
	unsigned int v;
} LX0_M_TO_INTR_T;
#define LX0_M_TO_INTRrv (*((regval)0xb8005214))
#define LX0_M_TO_INTRdv (0x00000000)
#define RMOD_LX0_M_TO_INTR(...) rset(LX0_M_TO_INTR, LX0_M_TO_INTRrv, __VA_ARGS__)
#define RFLD_LX0_M_TO_INTR(fld) (*((const volatile LX0_M_TO_INTR_T *)0xb8005214)).f.fld

typedef union {
	struct {
		unsigned int to_addr:32; //0
	} f;
	unsigned int v;
} LX0_M_TO_MONT_ADDR_T;
#define LX0_M_TO_MONT_ADDRrv (*((regval)0xb8005218))
#define LX0_M_TO_MONT_ADDRdv (0x00000000)
#define RMOD_LX0_M_TO_MONT_ADDR(...) rset(LX0_M_TO_MONT_ADDR, LX0_M_TO_MONT_ADDRrv, __VA_ARGS__)
#define RFLD_LX0_M_TO_MONT_ADDR(fld) (*((const volatile LX0_M_TO_MONT_ADDR_T *)0xb8005218)).f.fld


/*-----------------------------------------------------
	Slave LX_0 Bus Timeout Monitor
	-----------------------------------------------------*/
typedef union {
	struct {
		unsigned int to_ctrl_en:1; //1
		unsigned int to_ctrl_thr:3; //7
		unsigned int mbz_0:28; //0
	} f;
	unsigned int v;
} LX0_S_TO_CTRL_T;
#define LX0_S_TO_CTRLrv (*((regval)0xb8005220))
#define LX0_S_TO_CTRLdv (0xF0000000)
#define RMOD_LX0_S_TO_CTRL(...) rset(LX0_S_TO_CTRL, LX0_S_TO_CTRLrv, __VA_ARGS__)
#define RFLD_LX0_S_TO_CTRL(fld) (*((const volatile LX0_S_TO_CTRL_T *)0xb8005220)).f.fld

typedef union {
	struct {
		unsigned int to_ip:2; //0
		unsigned int mbz_0:30; //0
	} f;
	unsigned int v;
} LX0_S_TO_INTR_T;
#define LX0_S_TO_INTRrv (*((regval)0xb8005224))
#define LX0_S_TO_INTRdv (0x00000000)
#define RMOD_LX0_S_TO_INTR(...) rset(LX0_S_TO_INTR, LX0_S_TO_INTRrv, __VA_ARGS__)
#define RFLD_LX0_S_TO_INTR(fld) (*((const volatile LX0_S_TO_INTR_T *)0xb8005224)).f.fld

typedef union {
	struct {
		unsigned int to_addr:32; //0
	} f;
	unsigned int v;
} LX0_S_TO_MONT_ADDR_T;
#define LX0_S_TO_MONT_ADDRrv (*((regval)0xb8005228))
#define LX0_S_TO_MONT_ADDRdv (0x00000000)
#define RMOD_LX0_S_TO_MONT_ADDR(...) rset(LX0_S_TO_MONT_ADDR, LX0_S_TO_MONT_ADDRrv, __VA_ARGS__)
#define RFLD_LX0_S_TO_MONT_ADDR(fld) (*((const volatile LX0_S_TO_MONT_ADDR_T *)0xb8005228)).f.fld


/*-----------------------------------------------------
	Mater LX_1 Bus Timeout Monitor
	-----------------------------------------------------*/
typedef union {
	struct {
		unsigned int to_ctrl_en:1; //1
		unsigned int to_ctrl_thr:3; //7
		unsigned int mbz_0:28; //0
	} f;
	unsigned int v;
} LX1_M_TO_CTRL_T;
#define LX1_M_TO_CTRLrv (*((regval)0xb8005230))
#define LX1_M_TO_CTRLdv (0xF0000000)
#define RMOD_LX1_M_TO_CTRL(...) rset(LX1_M_TO_CTRL, LX1_M_TO_CTRLrv, __VA_ARGS__)
#define RFLD_LX1_M_TO_CTRL(fld) (*((const volatile LX1_M_TO_CTRL_T *)0xb8005230)).f.fld

typedef union {
	struct {
		unsigned int to_ip:2; //0
		unsigned int mbz_0:30; //0
	} f;
	unsigned int v;
} LX1_M_TO_INTR_T;
#define LX1_M_TO_INTRrv (*((regval)0xb8005234))
#define LX1_M_TO_INTRdv (0x00000000)
#define RMOD_LX1_M_TO_INTR(...) rset(LX1_M_TO_INTR, LX1_M_TO_INTRrv, __VA_ARGS__)
#define RFLD_LX1_M_TO_INTR(fld) (*((const volatile LX1_M_TO_INTR_T *)0xb8005234)).f.fld

typedef union {
	struct {
		unsigned int to_addr:32; //0
	} f;
	unsigned int v;
} LX1_M_TO_MONT_ADDR_T;
#define LX1_M_TO_MONT_ADDRrv (*((regval)0xb8005238))
#define LX1_M_TO_MONT_ADDRdv (0x00000000)
#define RMOD_LX1_M_TO_MONT_ADDR(...) rset(LX1_M_TO_MONT_ADDR, LX1_M_TO_MONT_ADDRrv, __VA_ARGS__)
#define RFLD_LX1_M_TO_MONT_ADDR(fld) (*((const volatile LX1_M_TO_MONT_ADDR_T *)0xb8005238)).f.fld


/*-----------------------------------------------------
	Slave LX_1 Bus Timeout Monitor
	-----------------------------------------------------*/
typedef union {
	struct {
		unsigned int to_ctrl_en:1; //1
		unsigned int to_ctrl_thr:3; //7
		unsigned int mbz_0:28; //0
	} f;
	unsigned int v;
} LX1_S_TO_CTRL_T;
#define LX1_S_TO_CTRLrv (*((regval)0xb8005240))
#define LX1_S_TO_CTRLdv (0xF0000000)
#define RMOD_LX1_S_TO_CTRL(...) rset(LX1_S_TO_CTRL, LX1_S_TO_CTRLrv, __VA_ARGS__)
#define RFLD_LX1_S_TO_CTRL(fld) (*((const volatile LX1_S_TO_CTRL_T *)0xb8005240)).f.fld

typedef union {
	struct {
		unsigned int to_ip:2; //0
		unsigned int mbz_0:30; //0
	} f;
	unsigned int v;
} LX1_S_TO_INTR_T;
#define LX1_S_TO_INTRrv (*((regval)0xb8005244))
#define LX1_S_TO_INTRdv (0x00000000)
#define RMOD_LX1_S_TO_INTR(...) rset(LX1_S_TO_INTR, LX1_S_TO_INTRrv, __VA_ARGS__)
#define RFLD_LX1_S_TO_INTR(fld) (*((const volatile LX1_S_TO_INTR_T *)0xb8005244)).f.fld

typedef union {
	struct {
		unsigned int to_addr:32; //0
	} f;
	unsigned int v;
} LX1_S_TO_MONT_ADDR_T;
#define LX1_S_TO_MONT_ADDRrv (*((regval)0xb8005248))
#define LX1_S_TO_MONT_ADDRdv (0x00000000)
#define RMOD_LX1_S_TO_MONT_ADDR(...) rset(LX1_S_TO_MONT_ADDR, LX1_S_TO_MONT_ADDRrv, __VA_ARGS__)
#define RFLD_LX1_S_TO_MONT_ADDR(fld) (*((const volatile LX1_S_TO_MONT_ADDR_T *)0xb8005248)).f.fld



/*-----------------------------------------------------
	Mater LX2 Bus Timeout Monitor
	-----------------------------------------------------*/
typedef union {
	struct {
		unsigned int to_ctrl_en:1; //1
		unsigned int to_ctrl_thr:3; //7
		unsigned int mbz_0:28; //0
	} f;
	unsigned int v;
} LX2_M_TO_CTRL_T;
#define LX2_M_TO_CTRLrv (*((regval)0xb8005250))
#define LX2_M_TO_CTRLdv (0xF0000000)
#define RMOD_LX2_M_TO_CTRL(...) rset(LX2_M_TO_CTRL, LX2_M_TO_CTRLrv, __VA_ARGS__)
#define RFLD_LX2_M_TO_CTRL(fld) (*((const volatile LX2_M_TO_CTRL_T *)0xb8005250)).f.fld

typedef union {
	struct {
		unsigned int to_ip:2; //0
		unsigned int mbz_0:30; //0
	} f;
	unsigned int v;
} LX2_M_TO_INTR_T;
#define LX2_M_TO_INTRrv (*((regval)0xb8005254))
#define LX2_M_TO_INTRdv (0x00000000)
#define RMOD_LX2_M_TO_INTR(...) rset(LX2_M_TO_INTR, LX2_M_TO_INTRrv, __VA_ARGS__)
#define RFLD_LX2_M_TO_INTR(fld) (*((const volatile LX2_M_TO_INTR_T *)0xb8005254)).f.fld

typedef union {
	struct {
		unsigned int to_addr:32; //0
	} f;
	unsigned int v;
} LX2_M_TO_MONT_ADDR_T;
#define LX2_M_TO_MONT_ADDRrv (*((regval)0xb8005258))
#define LX2_M_TO_MONT_ADDRdv (0x00000000)
#define RMOD_LX2_M_TO_MONT_ADDR(...) rset(LX2_M_TO_MONT_ADDR, LX2_M_TO_MONT_ADDRrv, __VA_ARGS__)
#define RFLD_LX2_M_TO_MONT_ADDR(fld) (*((const volatile LX2_M_TO_MONT_ADDR_T *)0xb8005258)).f.fld


/*-----------------------------------------------------
	Slave LX2 Bus Timeout Monitor
	-----------------------------------------------------*/
typedef union {
	struct {
		unsigned int to_ctrl_en:1; //1
		unsigned int to_ctrl_thr:3; //7
		unsigned int mbz_0:28; //0
	} f;
	unsigned int v;
} LX2_S_TO_CTRL_T;
#define LX2_S_TO_CTRLrv (*((regval)0xb8005260))
#define LX2_S_TO_CTRLdv (0xF0000000)
#define RMOD_LX2_S_TO_CTRL(...) rset(LX2_S_TO_CTRL, LX2_S_TO_CTRLrv, __VA_ARGS__)
#define RFLD_LX2_S_TO_CTRL(fld) (*((const volatile LX2_S_TO_CTRL_T *)0xb8005260)).f.fld

typedef union {
	struct {
		unsigned int to_ip:2; //0
		unsigned int mbz_0:30; //0
	} f;
	unsigned int v;
} LX2_S_TO_INTR_T;
#define LX2_S_TO_INTRrv (*((regval)0xb8005264))
#define LX2_S_TO_INTRdv (0x00000000)
#define RMOD_LX2_S_TO_INTR(...) rset(LX2_S_TO_INTR, LX2_S_TO_INTRrv, __VA_ARGS__)
#define RFLD_LX2_S_TO_INTR(fld) (*((const volatile LX2_S_TO_INTR_T *)0xb8005264)).f.fld

typedef union {
	struct {
		unsigned int to_addr:32; //0
	} f;
	unsigned int v;
} LX2_S_TO_MONT_ADDR_T;
#define LX2_S_TO_MONT_ADDRrv (*((regval)0xb8005268))
#define LX2_S_TO_MONT_ADDRdv (0x00000000)
#define RMOD_LX2_S_TO_MONT_ADDR(...) rset(LX2_S_TO_MONT_ADDR, LX2_S_TO_MONT_ADDRrv, __VA_ARGS__)
#define RFLD_LX2_S_TO_MONT_ADDR(fld) (*((const volatile LX2_S_TO_MONT_ADDR_T *)0xb8005268)).f.fld


/*-----------------------------------------------------
	Mater LX3 Bus Timeout Monitor
	-----------------------------------------------------*/
typedef union {
	struct {
		unsigned int to_ctrl_en:1; //1
		unsigned int to_ctrl_thr:3; //7
		unsigned int mbz_0:28; //0
	} f;
	unsigned int v;
} LX3_M_TO_CTRL_T;
#define LX3_M_TO_CTRLrv (*((regval)0xb8005270))
#define RMOD_LX3_M_TO_CTRL(...) rset(LX3_M_TO_CTRL, LX3_M_TO_CTRLrv, __VA_ARGS__)

typedef union {
	struct {
		unsigned int to_ip:2; //0
		unsigned int mbz_0:30; //0
	} f;
	unsigned int v;
} LX3_M_TO_INTR_T;
#define LX3_M_TO_INTRrv (*((regval)0xb8005274))
#define RMOD_LX3_M_TO_INTR(...) rset(LX3_M_TO_INTR, LX3_M_TO_INTRrv, __VA_ARGS__)

typedef union {
	struct {
		unsigned int to_addr:32; //0
	} f;
	unsigned int v;
} LX3_M_TO_MONT_ADDR_T;
#define LX3_M_TO_MONT_ADDRrv (*((regval)0xb8005278))
#define RMOD_LX3_M_TO_MONT_ADDR(...) rset(LX3_M_TO_MONT_ADDR, LX3_M_TO_MONT_ADDRrv, __VA_ARGS__)


/*-----------------------------------------------------
	LX_PBO_USW Bus Timeout Monitor
	-----------------------------------------------------*/
typedef union {
	struct {
		unsigned int to_ctrl_en:1; //1
		unsigned int to_ctrl_thr:3; //7
		unsigned int mbz_0:28; //0
	} f;
	unsigned int v;
} LX_PBO_USW_TO_CTRL_T;
#define LX_PBO_USW_TO_CTRLrv (*((regval)0xb8005280))
#define RMOD_LX_PBO_USW_TO_CTRL(...) rset(LX_PBO_USW_TO_CTRL, LX_PBO_USW_TO_CTRLrv, __VA_ARGS__)

typedef union {
	struct {
		unsigned int to_ip:2; //0
		unsigned int mbz_0:30; //0
	} f;
	unsigned int v;
} LX_PBO_USW_TO_INTR_T;
#define LX_PBO_USW_TO_INTRrv (*((regval)0xb8005284))
#define RMOD_LX_PBO_USW_TO_INTR(...) rset(LX_PBO_USW_TO_INTR, LX_PBO_USW_TO_INTRrv, __VA_ARGS__)

typedef union {
	struct {
		unsigned int to_addr:32; //0
	} f;
	unsigned int v;
} LX_PBO_USW_TO_MONT_ADDR_T;
#define LX_PBO_USW_TO_MONT_ADDRrv (*((regval)0xb8005288))
#define RMOD_LX_PBO_USW_TO_MONT_ADDR(...) rset(LX_PBO_USW_TO_MONT_ADDR, TO_MONT_ADDRrv, __VA_ARGS__)


/*-----------------------------------------------------
	LX_PBO_USR Bus Timeout Monitor
	-----------------------------------------------------*/
typedef union {
	struct {
		unsigned int to_ctrl_en:1; //1
		unsigned int to_ctrl_thr:3; //7
		unsigned int mbz_0:28; //0
	} f;
	unsigned int v;
} LX_PBO_USR_TO_CTRL_T;
#define LX_PBO_USR_TO_CTRLrv (*((regval)0xb8005290))
#define RMOD_LX_PBO_USR_TO_CTRL(...) rset(LX_PBO_USR_TO_CTRL, LX_PBO_USR_TO_CTRLrv, __VA_ARGS__)

typedef union {
	struct {
		unsigned int to_ip:2; //0
		unsigned int mbz_0:30; //0
	} f;
	unsigned int v;
} LX_PBO_USR_TO_INTR_T;
#define LX_PBO_USR_TO_INTRrv (*((regval)0xb8005294))
#define RMOD_LX_PBO_USR_TO_INTR(...) rset(LX_PBO_USR_TO_INTR, LX_PBO_USR_TO_INTRrv, __VA_ARGS__)

typedef union {
	struct {
		unsigned int to_addr:32; //0
	} f;
	unsigned int v;
} LX_PBO_USR_TO_MONT_ADDR_T;
#define LX_PBO_USR_TO_MONT_ADDRrv (*((regval)0xb8005298))
#define RMOD_LX_PBO_USR_TO_MONT_ADDR(...) rset(LX_PBO_USR_TO_MONT_ADDR, LX_PBO_USR_TO_MONT_ADDRrv, __VA_ARGS__)


/*-----------------------------------------------------
	LX_PBO_DSW Bus Timeout Monitor
	-----------------------------------------------------*/
typedef union {
	struct {
		unsigned int to_ctrl_en:1; //1
		unsigned int to_ctrl_thr:3; //7
		unsigned int mbz_0:28; //0
	} f;
	unsigned int v;
} LX_PBO_DSW_TO_CTRL_T;
#define LX_PBO_DSW_TO_CTRLrv (*((regval)0xb80052a0))
#define RMOD_LX_PBO_DSW_TO_CTRL(...) rset(LX_PBO_DSW_TO_CTRL, LX_PBO_DSW_TO_CTRLrv, __VA_ARGS__)

typedef union {
	struct {
		unsigned int to_ip:2; //0
		unsigned int mbz_0:30; //0
	} f;
	unsigned int v;
} LX_PBO_DSW_TO_INTR_T;
#define LX_PBO_DSW_TO_INTRrv (*((regval)0xb80052a4))
#define RMOD_LX_PBO_DSW_TO_INTR(...) rset(LX_PBO_DSW_TO_INTR, LX_PBO_DSW_TO_INTRrv, __VA_ARGS__)

typedef union {
	struct {
		unsigned int to_addr:32; //0
	} f;
	unsigned int v;
} LX_PBO_DSW_TO_MONT_ADDR_T;
#define LX_PBO_DSW_TO_MONT_ADDRrv (*((regval)0xb80052a8))
#define RMOD_LX_PBO_DSW_TO_MONT_ADDR(...) rset(LX_PBO_DSW_TO_MONT_ADDR, TO_MONT_ADDRrv, __VA_ARGS__)



/*-----------------------------------------------------
	LX_PBO_DSR Bus Timeout Monitor
	-----------------------------------------------------*/
typedef union {
	struct {
		unsigned int to_ctrl_en:1; //1
		unsigned int to_ctrl_thr:3; //7
		unsigned int mbz_0:28; //0
	} f;
	unsigned int v;
} LX_PBO_DSR_TO_CTRL_T;
#define LX_PBO_DSR_TO_CTRLrv (*((regval)0xb80052b0))
#define RMOD_LX_PBO_DSR_TO_CTRL(...) rset(LX_PBO_DSR_TO_CTRL, LX_PBO_DSR_TO_CTRLrv, __VA_ARGS__)

typedef union {
	struct {
		unsigned int to_ip:2; //0
		unsigned int mbz_0:30; //0
	} f;
	unsigned int v;
} LX_PBO_DSR_TO_INTR_T;
#define LX_PBO_DSR_TO_INTRrv (*((regval)0xb80052b4))
#define RMOD_LX_PBO_DSR_TO_INTR(...) rset(LX_PBO_DSR_TO_INTR, LX_PBO_DSR_TO_INTRrv, __VA_ARGS__)

typedef union {
	struct {
		unsigned int to_addr:32; //0
	} f;
	unsigned int v;
} LX_PBO_DSR_TO_MONT_ADDR_T;
#define LX_PBO_DSR_TO_MONT_ADDRrv (*((regval)0xb80052b8))
#define RMOD_LX_PBO_DSR_TO_MONT_ADDR(...) rset(LX_PBO_DSR_TO_MONT_ADDR, TO_MONT_ADDRrv, __VA_ARGS__)



/*-----------------------------------------------------
	LX_PBO_EGW Bus Timeout Monitor
	-----------------------------------------------------*/
typedef union {
	struct {
		unsigned int to_ctrl_en:1; //1
		unsigned int to_ctrl_thr:3; //7
		unsigned int mbz_0:28; //0
	} f;
	unsigned int v;
} LX_PBO_EGW_TO_CTRL_T;
#define LX_PBO_EGW_TO_CTRLrv (*((regval)0xb80052c0))
#define RMOD_LX_PBO_EGW_TO_CTRL(...) rset(LX_PBO_EGW_TO_CTRL, LX_PBO_EGW_TO_CTRLrv, __VA_ARGS__)


/*-----------------------------------------------------
	Extraced from file_MEM_SRAM.xml
	-----------------------------------------------------*/
typedef union {
	struct {
		unsigned int lx_sa:24; //0
		unsigned int mbz_0:7; //0
		unsigned int ensram:1; //0
	} f;
	unsigned int v;
} CPU_SRAM_SEG0_ADDR_T;
#define CPU_SRAM_SEG0_ADDRrv (*((regval)0xb8004000))
#define CPU_SRAM_SEG0_ADDRdv (0x00000000)
#define RMOD_CPU_SRAM_SEG0_ADDR(...) rset(CPU_SRAM_SEG0_ADDR, CPU_SRAM_SEG0_ADDRrv, __VA_ARGS__)
#define RIZS_CPU_SRAM_SEG0_ADDR(...) rset(CPU_SRAM_SEG0_ADDR, 0, __VA_ARGS__)
#define RFLD_CPU_SRAM_SEG0_ADDR(fld) (*((const volatile CPU_SRAM_SEG0_ADDR_T *)0xb8004000)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:28; //0
		unsigned int size:4; //8
	} f;
	unsigned int v;
} CPU_SRAM_SEG0_SIZE_T;
#define CPU_SRAM_SEG0_SIZErv (*((regval)0xb8004004))
#define CPU_SRAM_SEG0_SIZEdv (0x00000008)
#define RMOD_CPU_SRAM_SEG0_SIZE(...) rset(CPU_SRAM_SEG0_SIZE, CPU_SRAM_SEG0_SIZErv, __VA_ARGS__)
#define RIZS_CPU_SRAM_SEG0_SIZE(...) rset(CPU_SRAM_SEG0_SIZE, 0, __VA_ARGS__)
#define RFLD_CPU_SRAM_SEG0_SIZE(fld) (*((const volatile CPU_SRAM_SEG0_SIZE_T *)0xb8004004)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:16; //0
		unsigned int base:8; //0
		unsigned int mbz_1:8; //0
	} f;
	unsigned int v;
} CPU_SRAM_SEG0_BASE_T;
#define CPU_SRAM_SEG0_BASErv (*((regval)0xb8004008))
#define CPU_SRAM_SEG0_BASEdv (0x00000000)
#define RMOD_CPU_SRAM_SEG0_BASE(...) rset(CPU_SRAM_SEG0_BASE, CPU_SRAM_SEG0_BASErv, __VA_ARGS__)
#define RIZS_CPU_SRAM_SEG0_BASE(...) rset(CPU_SRAM_SEG0_BASE, 0, __VA_ARGS__)
#define RFLD_CPU_SRAM_SEG0_BASE(fld) (*((const volatile CPU_SRAM_SEG0_BASE_T *)0xb8004008)).f.fld

typedef union {
	struct {
		unsigned int lx_sa:24; //0
		unsigned int mbz_0:7; //0
		unsigned int ensram:1; //0
	} f;
	unsigned int v;
} CPU_SRAM_SEG1_ADDR_T;
#define CPU_SRAM_SEG1_ADDRrv (*((regval)0xb8004010))
#define CPU_SRAM_SEG1_ADDRdv (0x00000000)
#define RMOD_CPU_SRAM_SEG1_ADDR(...) rset(CPU_SRAM_SEG1_ADDR, CPU_SRAM_SEG1_ADDRrv, __VA_ARGS__)
#define RIZS_CPU_SRAM_SEG1_ADDR(...) rset(CPU_SRAM_SEG1_ADDR, 0, __VA_ARGS__)
#define RFLD_CPU_SRAM_SEG1_ADDR(fld) (*((const volatile CPU_SRAM_SEG1_ADDR_T *)0xb8004010)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:28; //0
		unsigned int size:4; //0
	} f;
	unsigned int v;
} CPU_SRAM_SEG1_SIZE_T;
#define CPU_SRAM_SEG1_SIZErv (*((regval)0xb8004014))
#define CPU_SRAM_SEG1_SIZEdv (0x00000000)
#define RMOD_CPU_SRAM_SEG1_SIZE(...) rset(CPU_SRAM_SEG1_SIZE, CPU_SRAM_SEG1_SIZErv, __VA_ARGS__)
#define RIZS_CPU_SRAM_SEG1_SIZE(...) rset(CPU_SRAM_SEG1_SIZE, 0, __VA_ARGS__)
#define RFLD_CPU_SRAM_SEG1_SIZE(fld) (*((const volatile CPU_SRAM_SEG1_SIZE_T *)0xb8004014)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:16; //0
		unsigned int base:8; //0
		unsigned int mbz_1:8; //0
	} f;
	unsigned int v;
} CPU_SRAM_SEG1_BASE_T;
#define CPU_SRAM_SEG1_BASErv (*((regval)0xb8004018))
#define CPU_SRAM_SEG1_BASEdv (0x00000000)
#define RMOD_CPU_SRAM_SEG1_BASE(...) rset(CPU_SRAM_SEG1_BASE, CPU_SRAM_SEG1_BASErv, __VA_ARGS__)
#define RIZS_CPU_SRAM_SEG1_BASE(...) rset(CPU_SRAM_SEG1_BASE, 0, __VA_ARGS__)
#define RFLD_CPU_SRAM_SEG1_BASE(fld) (*((const volatile CPU_SRAM_SEG1_BASE_T *)0xb8004018)).f.fld

typedef union {
	struct {
		unsigned int lx_sa:24; //0
		unsigned int mbz_0:7; //0
		unsigned int ensram:1; //0
	} f;
	unsigned int v;
} CPU_SRAM_SEG2_ADDR_T;
#define CPU_SRAM_SEG2_ADDRrv (*((regval)0xb8004020))
#define CPU_SRAM_SEG2_ADDRdv (0x00000000)
#define RMOD_CPU_SRAM_SEG2_ADDR(...) rset(CPU_SRAM_SEG2_ADDR, CPU_SRAM_SEG2_ADDRrv, __VA_ARGS__)
#define RIZS_CPU_SRAM_SEG2_ADDR(...) rset(CPU_SRAM_SEG2_ADDR, 0, __VA_ARGS__)
#define RFLD_CPU_SRAM_SEG2_ADDR(fld) (*((const volatile CPU_SRAM_SEG2_ADDR_T *)0xb8004020)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:28; //0
		unsigned int size:4; //0
	} f;
	unsigned int v;
} CPU_SRAM_SEG2_SIZE_T;
#define CPU_SRAM_SEG2_SIZErv (*((regval)0xb8004024))
#define CPU_SRAM_SEG2_SIZEdv (0x00000000)
#define RMOD_CPU_SRAM_SEG2_SIZE(...) rset(CPU_SRAM_SEG2_SIZE, CPU_SRAM_SEG2_SIZErv, __VA_ARGS__)
#define RIZS_CPU_SRAM_SEG2_SIZE(...) rset(CPU_SRAM_SEG2_SIZE, 0, __VA_ARGS__)
#define RFLD_CPU_SRAM_SEG2_SIZE(fld) (*((const volatile CPU_SRAM_SEG2_SIZE_T *)0xb8004024)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:16; //0
		unsigned int base:8; //0
		unsigned int mbz_1:8; //0
	} f;
	unsigned int v;
} CPU_SRAM_SEG2_BASE_T;
#define CPU_SRAM_SEG2_BASErv (*((regval)0xb8004028))
#define CPU_SRAM_SEG2_BASEdv (0x00000000)
#define RMOD_CPU_SRAM_SEG2_BASE(...) rset(CPU_SRAM_SEG2_BASE, CPU_SRAM_SEG2_BASErv, __VA_ARGS__)
#define RIZS_CPU_SRAM_SEG2_BASE(...) rset(CPU_SRAM_SEG2_BASE, 0, __VA_ARGS__)
#define RFLD_CPU_SRAM_SEG2_BASE(fld) (*((const volatile CPU_SRAM_SEG2_BASE_T *)0xb8004028)).f.fld

typedef union {
	struct {
		unsigned int lx_sa:24; //0
		unsigned int mbz_0:7; //0
		unsigned int ensram:1; //0
	} f;
	unsigned int v;
} CPU_SRAM_SEG3_ADDR_T;
#define CPU_SRAM_SEG3_ADDRrv (*((regval)0xb8004030))
#define CPU_SRAM_SEG3_ADDRdv (0x00000000)
#define RMOD_CPU_SRAM_SEG3_ADDR(...) rset(CPU_SRAM_SEG3_ADDR, CPU_SRAM_SEG3_ADDRrv, __VA_ARGS__)
#define RIZS_CPU_SRAM_SEG3_ADDR(...) rset(CPU_SRAM_SEG3_ADDR, 0, __VA_ARGS__)
#define RFLD_CPU_SRAM_SEG3_ADDR(fld) (*((const volatile CPU_SRAM_SEG3_ADDR_T *)0xb8004030)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:28; //0
		unsigned int size:4; //0
	} f;
	unsigned int v;
} CPU_SRAM_SEG3_SIZE_T;
#define CPU_SRAM_SEG3_SIZErv (*((regval)0xb8004034))
#define CPU_SRAM_SEG3_SIZEdv (0x00000000)
#define RMOD_CPU_SRAM_SEG3_SIZE(...) rset(CPU_SRAM_SEG3_SIZE, CPU_SRAM_SEG3_SIZErv, __VA_ARGS__)
#define RIZS_CPU_SRAM_SEG3_SIZE(...) rset(CPU_SRAM_SEG3_SIZE, 0, __VA_ARGS__)
#define RFLD_CPU_SRAM_SEG3_SIZE(fld) (*((const volatile CPU_SRAM_SEG3_SIZE_T *)0xb8004034)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:16; //0
		unsigned int base:8; //0
		unsigned int mbz_1:8; //0
	} f;
	unsigned int v;
} CPU_SRAM_SEG3_BASE_T;
#define CPU_SRAM_SEG3_BASErv (*((regval)0xb8004038))
#define CPU_SRAM_SEG3_BASEdv (0x00000000)
#define RMOD_CPU_SRAM_SEG3_BASE(...) rset(CPU_SRAM_SEG3_BASE, CPU_SRAM_SEG3_BASErv, __VA_ARGS__)
#define RIZS_CPU_SRAM_SEG3_BASE(...) rset(CPU_SRAM_SEG3_BASE, 0, __VA_ARGS__)
#define RFLD_CPU_SRAM_SEG3_BASE(fld) (*((const volatile CPU_SRAM_SEG3_BASE_T *)0xb8004038)).f.fld


typedef union {
    struct {
        unsigned int mbz_31_9:23;
        unsigned int sram_to_lx:1;  // 0
        unsigned int sram_to_oc1:1; // 1
        unsigned int sram_to_oc0:1; // 1
        unsigned int oc1_to_rom:1;  // 0
        unsigned int oc0_to_rom:1;  // 0
        unsigned int oc1_to_sram:2; // 1
        unsigned int oc0_to_sram:2; // 1
    } f;
    unsigned int v;
} CPU_SRAM_SCATS_T;
#define CPU_SRAM_SCATSrv (*((regval)0xb80040F8))
#define RMOD_SCATS(...) rset(CPU_SRAM_SCATS, CPU_SRAM_SCATSrv, __VA_ARGS__)
#define RFLD_SCATS(fld) (*((const volatile CPU_SRAM_SCATS_T *)0xb80040F8)).f.fld

#endif //#ifndef _REGISTER_MAP_H_
