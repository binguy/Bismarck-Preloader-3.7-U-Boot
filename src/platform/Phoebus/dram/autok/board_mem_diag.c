#include "autoconf.h"

#include "./bspchip.h"
#include "./memctl.h"
#include "board.h"

void memctlc_clk_rev_check(void);

/*====================================================================================== */
/* Merge from board_mem_diag.c */
/*=======================================================================================*/
unsigned int dramtype_freq_25oc[4][8]__attribute__ ((section(".text"))) = {
		{100, 384, 225, 192, 100, 384, 225, 100},
		{150, 350, 225, 200, 150, 350, 225, 125},
		{192, 300, 250, 175, 192, 300, 250, 150},
		{200, 400, 275, 166, 200, 400, 275, 200},
};

#ifdef CONFIG_RTL8685S_DYNAMIC_FREQ
char param_buf[80]__attribute__ ((section(".text")));

/* Boot tried to get parameters */
void boot_get_param(void)
{
	char id_buf[8];
	board_param_p buf = (board_param_p)param_buf;

	/* (0) Get flash parameters*/
	memcpy(id_buf, _PARM_START, 8);
	if(!memcmp(id_buf, BOARD_PARAM_ID, sizeof(8))) {
		/* (1) Load value from flash setting */
		memcpy(buf, _PARM_START, BOARD_PARAM_SZ );
	}else{
		/* (2) Reset to default value */
		memset(buf, 0, BOARD_PARAM_SZ);

		memcpy(buf->id, BOARD_PARAM_ID, 8);
		memcpy(buf->bootline, BOARD_PARAM_BOOT,8);
		memcpy(buf->mac[0], BOARD_PARAM_MAC, 6);
		buf->entry = BOARD_PARAM_ENTRY;
		buf->load = BOARD_PARAM_LOAD;
		buf->app = BOARD_PARAM_APP;
		buf->sysclk_ctrl = CONFIG_SYS_PLL_CTRL;
		buf->mckg_freq_div = CONFIG_MCKG_FREQ_DIV;
		buf->lx_pll_sel = CONFIG_LX_PLL_SEL;
		buf->cmu_ctrl = CONFIG_SYS_CMU_CTRL;
	}

	return;

}
#endif  /* CONFIG_RTL8685S_DYNAMIC_FREQ */

void SetSWRCtrlReg(unsigned int swr_val)
{
	unsigned int cnt = 0x1000;

	REG32(SYSREG_SWR_CTRL1) = (0xa8000000 | swr_val);
	while( (REG32(SYSREG_SWR_CTRL2) & SYSREG_SWR_ANAREG_RDY) && (cnt--));

	return;
}

void SetSWR(unsigned int  swr_data)
{
	unsigned int cnt = 0x1000;

	do{
		SetSWRCtrlReg(0x350000);

		/* anareg_rdy & anareg_valid equ high, and then unlock SWR*/
		SetSWRCtrlReg(0x600000);

		SetSWRCtrlReg(swr_data);

	}while( ((REG32(SYSREG_SWR_CTRL2)&0xFFFF)!=(swr_data & 0xFFFF)) && (cnt--)) ;


	return;
}
void voltage_setup(void){

	unsigned int swr_data = 0x0;

	/* Setup correct Voltage for SWR output */
	swr_data = 0xb55625;
	SetSWR(swr_data);

	/* Set OCP(over current protection) */
	swr_data = 0xb701cf;
	SetSWR(swr_data);

	/* Select page to default */
	SetSWRCtrlReg(0x0);



	/* Setup correct Voltage for DDRLDO output */
	do{
		/* DDRLDO output ~ 1.8V*/
		REG32(SYSREG_ANA1_CTL_REG) = 0xE3;
	}while((REG32(SYSREG_ANA1_CTL_REG)&0xFF)!=0xE3);

	return;
}

void spi_setup(void){
	unsigned int regval;

	//puts("setup SPI PLL and SPI CLK DIV");
	//puts("SPI PLL: 0x"); puthex(REG32(0xb8000228)); puts("\n\r");
	regval = REG32(0xb8000228);
	regval &= ~(0xf << 8);
	regval |= (0x8 << 8);
	REG32(0xb8000228) = regval;
	//puts("SPI PLL after setup: 0x"); puthex(REG32(0xb8000228)); puts("\n\r");


	//puts("SPI CLK DIV: 0x"); puthex(REG32(0xb8001200)); puts("\n\r");
	regval = REG32(0xb8001200);
	regval &= ~(0x7 << 29);
	regval |= (0x1 << 29);
	REG32(0xb8001200) = regval;
	//puts("SPI CLK DIV: 0x"); puthex(REG32(0xb8001200)); puts("\n\r");

}

void swr_setup(void){

	if(REG32(0xb8a80020) != 0x2d000001){
		//setup SWR
		REG32(0xb8000218) = 0x394;

		//DDR2 1.8V
		if (memctlc_DDR_Type() == 2) {
			REG32(0xb8000218) |= (1 << 16);
		}
	}else{
		REG32(0xb8000218) &= (~(1 << 8));
	}
}

void pll_setup(void){
#ifndef CONFIG_SOFTWARE_OVERWRITE_FREQ

	/* Don't do anything, just use default value */

#else
	unsigned int wait_loop_cnt = 0;
	unsigned char ocp0;

#ifdef CONFIG_OCP0_FREQ_OVER_850
	//reg_en_DIV2_cpu0 = 0
	REG32(SYSREG_PLL_CTL32_63_REG) &= ~(1 << 18);
#else
	ocp0 = ((CONFIG_SYS_PLL_CTRL>>16)&0x3f);
	if (ocp0>=0x1e) {
		REG32(SYSREG_PLL_CTL0_31_REG)  |= (3<<20);
	}
#endif
	/* (1) CPU CLOCK select LX PLL clock */
	wait_loop_cnt = 700 * 10000;
	while(wait_loop_cnt--){
		if(wait_loop_cnt == 3500000){
			REG32(SYSREG_SYSTEM_STATUS_REG) &= (~(SYSREG_SYSTEM_STATUS_CF_CKSE_OCP0_MASK|SYSREG_SYSTEM_STATUS_CF_CKSE_OCP1_MASK));
			//REG32(SYSREG_SYSTEM_STATUS_REG) &= (~SYSREG_SYSTEM_STATUS_CF_CKSE_OCP0_MASK);
		}
	}

	/* (2) Invoke the PLL change. */
	wait_loop_cnt = 500 * 1000;
	while(wait_loop_cnt--);

	REG32(SYSREG_SYSCLK_CONTROL_REG) = CONFIG_SYS_PLL_CTRL;

	wait_loop_cnt = 500 * 1000;
	while(wait_loop_cnt--);

	/* (5) CPU CLOCK select OCP0 PLL clock */
	wait_loop_cnt = 700 * 10000;
	while(wait_loop_cnt--){
		if(wait_loop_cnt == 3500000){
			REG32(SYSREG_SYSTEM_STATUS_REG) |=  SYSREG_SYSTEM_STATUS_CF_CKSE_OCP0_MASK|SYSREG_SYSTEM_STATUS_CF_CKSE_OCP1_MASK;
			//REG32(SYSREG_SYSTEM_STATUS_REG) |=  SYSREG_SYSTEM_STATUS_CF_CKSE_OCP0_MASK;
		}
	}
#endif

	return;

}

unsigned int sys_get_dramtype_freq_strapping(void)
{
	volatile unsigned int *bonding_reg;
	volatile unsigned int *strapping_reg;
	unsigned int b_sel, s_sel;

	/* !!!!!!!!!!!!!!!! FIX ME !!!!!!!!!!!!!!!!!! */

	bonding_reg = (volatile unsigned int *)BONDING_REG;
	strapping_reg = (volatile unsigned int *)STRAPING_REG;
	b_sel = (*bonding_reg & BONDING_REG_DRAMTYPE_FREQ43_MASK) >> BONDING_REG_DRAMTYPE_FREQ43_FD_S;
	s_sel = (*strapping_reg & STRAPING_REG_DRAMTYPE_FREQ210_MASK) >> STRAPING_REG_DRAMTYPE_FREQ210_FD_S;

	return dramtype_freq_25oc[b_sel][s_sel];
}


/* Function Name:
 * 	board_CPU_freq_mhz
 * Descripton:
 *	Get the current CPU frequency in MHz.
 * Input:
 *	None
 * Output:
 * 	None
 * Return:
 *  	CPU frequncy in MHz.
 */
#if 0 //currently not used on RTL8685G
unsigned int board_CPU_freq_mhz(const unsigned int cmu_ctrl_reg)
{
#ifdef CONFIG_SOFTWARE_OVERWRITE_FREQ
	unsigned int cpu_clk;
	unsigned int cmu_div;

	cmu_div = 1;
	if(cmu_ctrl_reg & SYSREG_CMUCTLR_CMU_MD_MASK){
		cmu_div = 1 << ((cmu_ctrl_reg & SYSREG_CMUCTLR_CMU_CPU0_FREQ_DIV_MASK) >>\
			   SYSREG_CMUCTLR_CMU_CPU0_FREQ_DIV_FD_S);
	}

	cpu_clk = (REG32(SYSREG_SYSCLK_CONTROL_REG) & SYSREG_SYSCLK_CONTROL_OCP0PLL_MASK) \
		>> SYSREG_SYSCLK_CONTROL_OCP0PLL_FD_S;

	cpu_clk = (cpu_clk + 2) * 25;
	/*puts("CPU Clock "); puthex((cpu_clk/cmu_div)); puts("\n");*/

	return (cpu_clk/cmu_div);
#else
	return CONFIG_CPUCLK_MHZ;
#endif
}
#endif

/* Function Name:
 * 	board_DRAM_freq_mhz
 * Descripton:
 *	Get the current DRAM frequency in MHz.
 * Input:
 *	None
 * Output:
 * 	None
 * Return:
 *  	DRAM Frequncy in MHz.
 */
unsigned int board_DRAM_freq_mhz(void)
{
	/* Dram clock must be divided by 2 in default
		due to the memory controller's design */

	unsigned char pdiv;
	pdiv=1<<((REG32(MEMPLL95_64)>>14)&0x3);			//pdiv 0: /1, 1: /2, 2: /4, 3: /8
	return ((((((REG32(MEMPLL127_96)>>24)&0x0FF) + 3) * 25)>>1)/pdiv);		//(n+3)*25
}

unsigned int board_SPIF_freq_mhz(int sel)
{
	unsigned int spif_clk_div;

	spif_clk_div = (REG32(SYSREG_LX_PLL_SEL_REG) & SYSREG_LX_PLL_CTRL_SPIFPLL_FD_MASK) \
		>> SYSREG_LX_PLL_CTRL_SPIFPLL_FD_S ;
	spif_clk_div += 2;

	/*
		sel =0, means internal SPIF clock frequency.
	     	sel =1, means output SPI clock frequency
	  */
	if(sel ==1)
		spif_clk_div *= (((REG32(0xB8001200) & 0xE0000000) >> 29) +1)*2 ;

	return (1000/spif_clk_div);
}

/*
 * DRAM parameters APIs for initializing DDR1/2/3 SDRAM
 * We current define environment variables listed below
 * for DRAM ODT/OCD/ZQ/Delay parameters. These fuctions are called when initializing
 * DDR1/2/3 SDRAM memory.
 * zq_cali_value: ZQ auto-calibration value, we apply zq_cali_value if it exists.
 * dram_odt	: DRAM side ODT value of DDR2 SDRAM(0, 50, 75, 150)
 * dram_ocd	: DRAM side OCD value of DDR3 SDRAM(ZQ/6, ZQ/7)
 * ddrkodl	: DDRKODL register value. (Assign CLKM/TX/PHASE 90 delay.)
 *
 */
unsigned int get_memory_delay_parameters(unsigned int *para_array)
{
	para_array[0] = CONFIG_DRAM_PREFERED_DDRKODL;
	return 1;
}

#ifdef CONFIG_DDR2_USAGE
unsigned int get_memory_ddr2_dram_odt_parameters(unsigned int *para_array)

{
	para_array[0] = (unsigned int)CONFIG_DDR2_DRAM_ODT_VALUE;
	return 1; /*fail */
}
#endif

#ifdef CONFIG_DDR3_USAGE
unsigned int get_memory_ddr3_dram_rtt_nom_parameters(unsigned int *para_array)

{
	para_array[0] = (unsigned int)CONFIG_DDR3_DRAM_RTT_NOM_VALUE;
	return 1; /*fail */
}

unsigned int get_memory_ddr3_dram_rtt_wr_parameters(unsigned int *para_array)

{
	para_array[0] = (unsigned int)CONFIG_DDR3_DRAM_RTT_WR_VALUE;
	return 1; /*fail */
}
#endif

unsigned int get_memory_dram_reduce_drv_parameters(unsigned int *para_array)
{
	if(CONFIG_PREFERED_DRAM_DRIV_STRENGTH){
		para_array[0] = 0; /*full*/
	}else{
		para_array[0] = 1;/*reduce*/
	}
	return 1;
}

