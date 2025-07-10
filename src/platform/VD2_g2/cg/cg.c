#include <soc.h>
#include <uart/uart.h>
#include <cg/cg.h>
#include <spi_nand/spi_nand_ctrl.h>

#ifndef SECTION_CG
#define SECTION_CG
#endif
#ifndef SECTION_CG_INFO
#define SECTION_CG_INFO
#endif


static void reg_to_mhz(void);
cg_info_t cg_info_query SECTION_CG_INFO;


/****** Common Part Between FPGA & ASIC *********/
SECTION_CG
u32_t cg_query_freq(u32_t dev_type) {
	if (0 == cg_info_query.dev_freq.cpu0_mhz) {
		reg_to_mhz();
	}
	return CG_QUERY_FREQUENCY(dev_type,(&cg_info_query.dev_freq));
}

SECTION_CG
void cg_copy_info_to_sram(void) {
	inline_memcpy(&cg_info_query, &cg_info_proj, sizeof(cg_info_t));
}

/******************* FPGA *******************/
#ifdef OTTO_PROJECT_FPGA
SECTION_CG void cg_init(void)
{
	uart_init(uart_baud_rate, cg_info_query.dev_freq.lx_mhz);
}

SECTION_CG void reg_to_mhz(void)
{
	inline_memcpy(&cg_info_query, &cg_info_proj, sizeof(cg_info_t));
}

SECTION_CG void cg_result_decode(void) {
	u32_t val = REG32(0xB8000308);
	if(cg_info_query.dev_freq.cpu0_mhz > cg_info_query.dev_freq.mem_mhz) {
		val = val & 0xFFFFFFEF;
	} else {
		val = val | 0x10;
	}
	if(cg_info_query.dev_freq.lx_mhz > cg_info_query.dev_freq.mem_mhz) {
		val = val & 0xFFFFFFF8;
	} else {
		val = val | 0x7;
	}
	REG32(0xB8000308) = val;

	printf("II: OCP %dMHz, MEM %dMHz, LX %dMHz, SPIF %dMHz\n",
				 cg_info_query.dev_freq.cpu0_mhz,
				 cg_info_query.dev_freq.mem_mhz,
				 cg_info_query.dev_freq.lx_mhz,
				 cg_info_query.dev_freq.spif_mhz);
	return;
}

#else //OTTO_PROJECT_FPGA
/* Impl. in board_mem_diag_8685s.c */
unsigned int board_CPU_freq_mhz(const unsigned int cmu_ctrl_reg);
unsigned int board_DRAM_freq_mhz(void);
unsigned int board_LX_freq_mhz(void);
unsigned int board_SPIF_freq_mhz(int sel);

typedef union {
	struct {
		unsigned int mbz_0:10;
		unsigned int ocp0_pll_div:6;
		unsigned int mbz_1:2;
		unsigned int ocp1_pll_div:6;
		unsigned int mbz_2:2;
		unsigned int sdpll_div:6;
	} f;
	unsigned int v;
} SYSPLLCTR_T;
#define SYSPLLCTRrv (*((regval)0xb8000200))
#define CMUGCRrv (*((regval)0xb8000380))

static void reg_to_mhz(void) {
	SYSPLLCTR_T syspllctr;

	cg_info_query.dev_freq.cpu0_mhz = board_CPU_freq_mhz(CMUGCRrv);
	cg_info_query.dev_freq.mem_mhz = board_DRAM_freq_mhz();
	cg_info_query.dev_freq.lx_mhz = board_LX_freq_mhz();

#ifdef OTTO_FLASH_SPI_NAND
    cg_info_query.dev_freq.spif_mhz = (cg_info_query.dev_freq.lx_mhz) / ((get_spi_ctrl_divisor() + 1)*2);
#endif

	syspllctr.v = SYSPLLCTRrv;
	cg_info_query.dev_freq.cpu1_mhz = (syspllctr.f.ocp1_pll_div + 2) * 25;
	return;
}

extern void pll_setup(void);
extern void stall_setup(void);
extern void get_efuse_info(void);

#ifdef OTTO_FLASH_SPI_NAND
extern clk_div_sel_info_t sclk_divisor[];

void __cal_spi_ctrl_divisor(cg_sclk_info_t *info)
{
    u32_t target_sclk = cg_info_query.dev_freq.spif_mhz;
    u32_t input_pll   = board_LX_freq_mhz();
    u32_t cur_sclk    = info->cal_sclk_mhz;
    int i = -1;
    u32_t cur_divisor;
    u32_t calc_sclk;

    while(1){
        cur_divisor = sclk_divisor[++i].divisor;
        
        if(END_OF_INFO == cur_divisor) break;

        calc_sclk = input_pll/cur_divisor;
        if((target_sclk >= calc_sclk) && (calc_sclk > cur_sclk)){
            cur_sclk = calc_sclk;
            info->cal_sclk_mhz  = calc_sclk;
            info->sclk_div2ctrl = sclk_divisor[i].div_to_ctrl;
        }
    }
    return;
}

void _cg_calc_spif(cg_sclk_info_t *info)
{
    //Check the special limitation of sclk first
    cg_info_query.dev_freq.spif_mhz = nsu_sclk_limit(cg_info_query.dev_freq.spif_mhz);

    __cal_spi_ctrl_divisor(info);

    if(info->cal_sclk_mhz >= 100){
        info->spif_rx_delay = 2;
    }else if(info->cal_sclk_mhz >= 50){
        info->spif_rx_delay = 1;
    }else if(info->cal_sclk_mhz == 0){
        info->sclk_div2ctrl = get_default_spi_ctrl_divisor();
    }
    return;
}

SECTION_CG_CORE_INIT 
void cg_spif_clk_init(cg_sclk_info_t *info)
{
    set_spi_ctrl_divisor(info->sclk_div2ctrl, cg_info_query.dev_freq.spif_mhz);
    set_spi_ctrl_latency(info->spif_rx_delay);
    return;
}
#endif //#elif defined (OTTO_FLASH_SPI_NAND)

void cg_init(void) {
	/* stall_setup() comes from SNOF. loader boot0412, dev by TCTsai. */
	stall_setup();

	get_efuse_info();

	/* Increase SRAM to 500MHz */
	REG32(0xb8000200) = (REG32(0xb8000200) | 0x80000000);

	/* pll_setup() comes from SNOF. loader boot0412, dev. by TCTsai. */
	pll_setup();
	uart_init(uart_baud_rate, cg_info_query.dev_freq.lx_mhz);

#ifdef OTTO_FLASH_SPI_NAND
    cg_sclk_info_t final = {0, 0, 0};
    _cg_calc_spif(&final);
    cg_spif_clk_init(&final);
	printf("II: SPIF %dMHz\n",cg_info_query.dev_freq.spif_mhz);
#endif
	return;
}

void cg_result_decode(void) {
	reg_to_mhz();
	/* printf("II: CPU0 %dMHz, MEM %dMHz, LX %dMHz, SPIF %dMHz, CPU1 %dMHz\n", */
	/* 			 cg_info_query.dev_freq.cpu0_mhz, */
	/* 			 cg_info_query.dev_freq.mem_mhz, */
	/* 			 cg_info_query.dev_freq.lx_mhz, */
	/* 			 cg_info_query.dev_freq.spif_mhz, */
	/* 			 cg_info_query.dev_freq.cpu1_mhz); */
	return;
}
#endif

REG_INIT_FUNC(cg_copy_info_to_sram, 11);
REG_INIT_FUNC(cg_init, 13);
REG_INIT_FUNC(cg_result_decode, 31);
