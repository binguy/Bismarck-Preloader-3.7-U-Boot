/**test 2**/

#include <soc.h>
#include "autoconf.h"
#include "./bspchip.h"
#include "./memctl.h"
#include "./memctl_func.h"

#include <extra.h>
#include <mem_plat_setting.h>

#ifdef CONFIG_DDR2_USAGE
#define DDR2_USAGE
#else
#undef DDR2_USAGE
#endif

#ifdef CONFIG_DDR3_USAGE
#define DDR3_USAGE
#else
#undef DDR3_USAGE
#endif


/* Prototype*/

#ifdef DDR2_USAGE
void memctlc_ddr2_dll_reset(void);
extern void _DTR_DDR2_MRS_setting(unsigned int *mr);
#endif


#ifdef DDR3_USAGE
void memctlc_ddr3_dll_reset(void);
extern void _DTR_DDR3_MRS_setting(unsigned int *sug_dtr, unsigned int *mr);
void dram_ZQCS_ZQCL_enable(void);
#endif
int memctlc_ZQ_cali_value(unsigned int Z_prog_ODT,unsigned int Z_prog_OCD);

unsigned int _DCR_get_buswidth(void);
void memctlc_dram_phy_reset(void);
void memctlc_clk_rev_check(void);

/* Definitions */
#ifndef printf
#define printf puts
#endif

/* global variable for FT */
unsigned int ft_result=0; //bit0: ZQ PASS=0 FAIL=1,  bit1: window Normal=0 Small=1

/* global variable for DRAM Efuse patch*/
unsigned int EFPH_patch_num=0;	//0:no patch  1:use efuse patch 1  2:use efuse patch 2

unsigned int EFPH_CLK_OCD=0;
unsigned int EFPH_Addr_OCD=0;
unsigned int EFPH_DQS_OCD=0;
unsigned int EFPH_DQ_OCD=0;

unsigned int EFPH_Ctrl_ODT=0;
unsigned int EFPH_DRAM_ODT=0;
unsigned int EFPH_DRAM_driving=0;
unsigned int EFPH_HCLK=0;
unsigned int EFPH_DQS_delay=0;
unsigned int EFPH_DQSEN=0;

unsigned int EFPH_Addr_delay=0;
unsigned int EFPH_DQR_delay=0;
unsigned int EFPH_DQW_delay=0;
unsigned int EFPH_DM_delay=0;

unsigned int EFPH_ZQ_en=0;
unsigned int EFPH_DRAM_ZQ_en=0;
unsigned int EFPH_DQSEN_en=0;
unsigned int EFPH_DQS_delay_en=0;
unsigned int EFPH_Addr_delay_en=0;
unsigned int EFPH_DQR_delay_en=0;
unsigned int EFPH_DQW_delay_en=0;
unsigned int EFPH_DM_delay_en=0;
unsigned int EFPH_Patch1_en=0;
unsigned int EFPH_Patch2_en=0;

unsigned int EFVD_Capacity=0;
unsigned int EFVD_Vendor=0;
unsigned int EFVD_type=0;
unsigned int EFVD_other=0;

void sys_watchdog_enable(unsigned int ph1, unsigned int ph2)
{
	REG32(SYSREG_WDCNTRR) |= SYSREG_WDT_KICK;
	REG32(SYSREG_WDTCTRLR) = 0;
	REG32(SYSREG_WDTCTRLR) = ((SYSREG_WDT_E) |\
								((ph1 << SYSREG_PH1_TO_S) & SYSREG_PH1_TO_MASK) |\
								((ph2 << SYSREG_PH2_TO_S) & SYSREG_PH2_TO_MASK));
	return;
}

void sys_watchdog_disable(void)
{
	REG32(SYSREG_WDCNTRR) = 0x0;
	REG32(SYSREG_WDTCTRLR) = 0x0;

	return;
}

void _memctl_delay_clkm_cycles(unsigned int delay_cycles)
{
	volatile unsigned int *mcr;

	mcr = (unsigned int *)MCR;

	while(delay_cycles--){
		volatile unsigned int read_tmp __attribute__((unused)) = *mcr;
	}

	return;
}

void _DRAM_PLL_CLK_power_switch(unsigned char power_on)
{
	volatile unsigned int *dpcpw;
	unsigned int	delay_tmp;
	dpcpw = (unsigned int *)0xB8000204;

	if(power_on)
		*dpcpw &=  ~(1<<4);
	else
		*dpcpw |=  (1<<4);

	delay_tmp=0x1fff;
	while(delay_tmp--);
}

void _periodic_DRAM_refresh(unsigned char enable)
{
	volatile unsigned int *dmcr;
	dmcr = (unsigned int *)DMCR;

	if(enable){
		/* Enable DRAM periodic DRAM refresh operation. */
		*dmcr &=  ~(1<<24);
	}else{
		/* Disable DRAM periodic DRAM refresh operation */
		*dmcr |=  (1<<24);
	}

	_memctl_delay_clkm_cycles(10);
	while((*dmcr & ((unsigned int)DMCR_MRS_BUSY)) != 0);
}


void _memctl_update_phy_param(void)
{
	volatile unsigned int *dmcr;
	volatile unsigned int *dacr;
	volatile unsigned int dacr_tmp1, dacr_tmp2;
	volatile unsigned int dmcr_tmp;

	dmcr = (unsigned int *)DMCR;
	dacr = (unsigned int *)DACCR;

	/* Write DMCR register to sync the parameters to phy control. */
	dmcr_tmp = *dmcr;
	*dmcr = dmcr_tmp;
	_memctl_delay_clkm_cycles(10);
	/* Waiting for the completion of the update procedure. */
	while((*dmcr & ((unsigned int)DMCR_MRS_BUSY)) != 0);

	__asm__ __volatile__("": : :"memory");

	/* reset phy buffer pointer */
	dacr_tmp1 = *dacr;
	dacr_tmp1 = dacr_tmp1 & (0xFFFFFFEF);
	dacr_tmp2 = dacr_tmp1 | (0x10);
	*dacr = dacr_tmp1 ;

	udelay(300);
	__asm__ __volatile__("": : :"memory");
	*dacr = dacr_tmp2 ;
	udelay(10);

	return;
}

void memctlc_reset_procedure(void){
	// Reset Procedure
	// 1. RESET# needs to be maintained for minimum 200 us with stable power. CKE is pulled "Low" anytime before RESET# being de-asserted (min. time 10 ns).
	// 2. After RESET# is de-asserted, wait for another 500 us until CKE becomes active.
	volatile unsigned int work_around __attribute__((unused));

	if(memctlc_DDR_Type()==IS_DDR3_SDRAM){
		REG32(0xb8001040) |=  (1 << 29);		//PM mode(bit 28.29) = 10: Enable self refresh (CKE low)
		udelay(100);						//100us
		REG32(0xb800022c)=0x0;				//Reset low
		udelay(10000);						//10000us
		REG32(0xb800022c)=0x1;				//Reset high
		udelay(800);						//800us
		work_around = REG32(0xa0000000);	//Read DRAM, and PM mode becomes to 00 (CKE high )
	}else{
		printf("AK: Start DDR2 Reset Procedure\n");
	}
}

static unsigned short efuse_read(unsigned    char entry){
	if (otto_is_apro()) {
		REG32(0xbb00001c)= (1<<16)|entry;
		udelay(1);
		while(REG32(0xbb000020) & 0x10000);
		return REG32(0xbb000020);
	}else{
		REG32(0xbb000020)= (1<<16)|entry;
		udelay(1);
		while(REG32(0xbb000024) & 0x10000);
		return REG32(0xbb000024);
	}
}


uint32 Efuse_DRAM_patch(void){

	uint32 pc_a=0,pc_b=0,pc_c=0,pc_d=0,pc_num=0;

	EFPH_Patch1_en = (efuse_read(251)>>14) & 0x3;
	EFPH_Patch2_en = (efuse_read(247)>>14) & 0x3;
	EFVD_type = (efuse_read(243)>>12) & 0x7;
	EFVD_Vendor = (efuse_read(243)>>8) & 0xf;
	EFVD_Capacity = (efuse_read(243)>>4) & 0x7;
	EFVD_other = (efuse_read(243)>>0) & 0xf;

	printf("AK: TP=%d, VD=%d, CC=%d, OT=%d,",
		EFVD_type,EFVD_Vendor,EFVD_Capacity,EFVD_other);

	if(EFPH_Patch1_en==1){
		pc_a=248;
		pc_b=249;
		pc_c=250;
		pc_d=251;
		pc_num=1;
	}else if(EFPH_Patch2_en==1){
		pc_a=244;
		pc_b=245;
		pc_c=246;
		pc_d=247;
		pc_num=2;
	}else{
		return pc_num;
	}
	EFPH_CLK_OCD = (efuse_read(pc_a)>>0) & 0xf;
	EFPH_Addr_OCD = (efuse_read(pc_a)>>4) & 0xf;
	EFPH_DQ_OCD = (efuse_read(pc_a)>>8) & 0xf;
	EFPH_DQS_OCD = (efuse_read(pc_a)>>12) & 0xf;
	
	EFPH_Ctrl_ODT = (efuse_read(pc_b)>>0) & 0xf;
	EFPH_DRAM_ODT = (efuse_read(pc_b)>>4) & 0x3;
	EFPH_DRAM_driving = (efuse_read(pc_b)>>6) & 0x1;
	EFPH_HCLK = (efuse_read(pc_b)>>7) & 0x1;
	EFPH_DQSEN = (efuse_read(pc_b)>>8) & 0xf;
	EFPH_DQS_delay = (efuse_read(pc_b)>>12) & 0xf;
	
	EFPH_Addr_delay = (efuse_read(pc_c)>>0) & 0xf; 
	EFPH_DQR_delay = (efuse_read(pc_c)>>4) & 0xf; 
	EFPH_DQW_delay = (efuse_read(pc_c)>>8) & 0xf; 
	EFPH_DM_delay = (efuse_read(pc_c)>>12) & 0xf;

	EFPH_ZQ_en = (efuse_read(pc_d)>>6) & 0x1;
	EFPH_DRAM_ZQ_en = (efuse_read(pc_d)>>7) & 0x1;	
	EFPH_DQSEN_en = (efuse_read(pc_d)>>8) & 0x1;
	EFPH_DQS_delay_en = (efuse_read(pc_d)>>9) & 0x1;
	EFPH_Addr_delay_en = (efuse_read(pc_d)>>10) & 0x1;
	EFPH_DQR_delay_en = (efuse_read(pc_d)>>11) & 0x1;
	EFPH_DQW_delay_en = (efuse_read(pc_d)>>12) & 0x1;
	EFPH_DM_delay_en = (efuse_read(pc_d)>>13) & 0x1;

#if 0
	printf("EFPH: Entry A, CLK_OCD=0x%x Addr_OCD=0x%x DQ_OCD=0x%x DQS_OCD=0x%x\n",
		EFPH_CLK_OCD,EFPH_Addr_OCD,EFPH_DQ_OCD,EFPH_DQS_OCD);
	
	printf("EFPH: Entry B, Ctrl_ODT=0x%x DRAM_ODT=0x%x DRAM_driving=0x%x HCLK=0x%x DQS_EN=0x%x DQS_delay=0x%x\n",
		EFPH_Ctrl_ODT,EFPH_DRAM_ODT,EFPH_DRAM_driving,EFPH_HCLK,EFPH_DQSEN,EFPH_DQS_delay);
	
	printf("EFPH: Entry C, Addr_delay=0x%x DQR_delay=0x%x DQW_delay=0x%x DM_delay=0x%x\n",
		EFPH_Addr_delay,EFPH_DQR_delay,EFPH_DQW_delay,EFPH_DM_delay);

	printf("EFPH: Entry D, ZQ_en=%d DRAM_ZQ_en=%d DQSEN_en=%d DQS_delay_en=%d\n",
		EFPH_ZQ_en,EFPH_DRAM_ZQ_en,EFPH_DQSEN_en,EFPH_DQS_delay_en);

	printf("               Addr_delay_en=%d DQR_delay_en=%d DQW_delay_en=%d DM_delay_en=%d\n",
		EFPH_Addr_delay_en,EFPH_DQR_delay_en,EFPH_DQW_delay_en,EFPH_DM_delay_en);
#endif
	return pc_num;
}


struct ZQ_Value {
	char* group;
	unsigned int ODT;
	unsigned int OCD;
};

//For Patch
struct ZQ_Value PATCH_VALUE[] = {
	{.group = "  Clock", 	.ODT = 0, .OCD = 0},
	{.group = "Address", 	.ODT = 0, .OCD = 0},
	{.group = "     DQ", 	.ODT = 0, .OCD = 0},
	{.group = "    DQS",	.ODT = 0, .OCD = 0},
};


void memctlc_ZQ_seperate_calibration(struct           ZQ_Value ODT_OCD_VALUE[4])
{
	volatile unsigned int *dmcr, *zq_zctrl_prog, *zq_zctrl_status, *zq_rzctrl_status;
	volatile unsigned int *zq_pad_ctrl_dq_odt, *zq_pad_ctrl_dq_ocd, *zq_pad_ctrl_dqsp_odt, *zq_pad_ctrl_dqsn_odt, *zq_pad_ctrl_dqs_ocd, *zq_pad_ctrl_adr_cmd_ocd, *zq_pad_ctrl_ck_ocd;
	volatile unsigned int zq_zctrl_prog_pre;

	dmcr = (volatile unsigned int *)DMCR;
	zq_zctrl_prog = (volatile unsigned int *)ZQ_ZCTRL_PROG;
	zq_zctrl_status = (volatile unsigned int *)ZQ_ZCTRL_STATUS;
	zq_rzctrl_status = (volatile unsigned int *)ZQ_RZCTRL_STATUS;

	zq_pad_ctrl_dq_odt = (volatile unsigned int *)ZQ_PAD_CTRL_DQ_ODT;
	zq_pad_ctrl_dq_ocd = (volatile unsigned int *)ZQ_PAD_CTRL_DQ_OCD;
	zq_pad_ctrl_dqsp_odt = (volatile unsigned int *)ZQ_PAD_CTRL_DQSP_ODT;
	zq_pad_ctrl_dqsn_odt = (volatile unsigned int *)ZQ_PAD_CTRL_DQSN_ODT;
	zq_pad_ctrl_dqs_ocd = (volatile unsigned int *)ZQ_PAD_CTRL_DQS_OCD;
	zq_pad_ctrl_adr_cmd_ocd = (volatile unsigned int *)ZQ_PAD_CTRL_ADR_CMD_OCD;
	zq_pad_ctrl_ck_ocd = (volatile unsigned int *)ZQ_PAD_CTRL_CK_OCD;

	unsigned int polling_limit,delay_loop;
	unsigned short RZQ_RUN_FLAG=0,sel,err_flag=0,en_R480=0;

	/* Disable DRAM refresh operation */
	*dmcr = ((*dmcr | DMCR_DIS_DRAM_REF_MASK) & (~DMCR_MR_MODE_EN_MASK));

	/*	Auto detect R240 resistor & initial ZQ parameters	*/
ENABLE_R480:
	if(en_R480 == 1){
		zq_zctrl_prog_pre = DZQ_AUTO_UP | (0<<ZCTRL_CLK_SEL_FD_S) | (2<<ZCTRL_CLK_SEL_FD_S);		//CTRL_PROG init, dqz_auto_up=1, zctrl_clk_sel=2'b10(default zclk/32), external resistor=1
		RZQ_RUN_FLAG = 1;
	}else{	//en_R240
		/*	initial ZQ parameters	*/
		zq_zctrl_prog_pre = DZQ_AUTO_UP | (0<<ZCTRL_CLK_SEL_FD_S) | (2<<ZCTRL_CLK_SEL_FD_S) | RZQ_EXT_R240;		//CTRL_PROG init, dqz_auto_up=1, zctrl_clk_sel=2'b10(default zclk/32), external resistor=1
	}

	/*  ZQ power up  */
	REG32(0xb8142100) = 0x0f;
	delay_loop = 0x1000;
	while(delay_loop--);

	/*	R480 Calibration */
	if(RZQ_RUN_FLAG){
		// initial ZQ calibration, rzq_cal_en = 1 ---> wait rzq_cal_done = 1 ---> rzq_cal_en = 0
		*zq_zctrl_prog = zq_zctrl_prog_pre;
		*zq_zctrl_prog |= RZQ_CAL_EN;				//rzq_cal_en = 1

		/* Polling to rzq_cal_done */
		polling_limit = 0x1000;
		while((*zq_rzctrl_status) & RZQ_CAL_DONE){
			polling_limit--;
			if(polling_limit == 0){
				//				printf("%s, %d: Error, R480 calibration ready polling timeout!\n", __FUNCTION__, __LINE__);
				break;
			}
		}
		puts("ZQ: ZQ_RZCTRL_STATUS=0x");puthex(*zq_rzctrl_status);puts("\n\r");
		*zq_zctrl_prog &= ~RZQ_CAL_EN;				//rzq_cal_en = 0
	}

	/*	ZQ Calibration	*/
	for(sel=0;sel<4;sel++){	//Set "sel" to determine how many ZQ value to be calibrated
		puts("ZQ: "); puts(ODT_OCD_VALUE[sel].group);puts(": ");
		*zq_zctrl_prog = zq_zctrl_prog_pre;		//clear zprog[13:0]
		*zq_zctrl_prog |= (sel << DZQ_UP_SEL_FD_S) | memctlc_ZQ_cali_value(ODT_OCD_VALUE[sel].ODT,ODT_OCD_VALUE[sel].OCD);
		printf("zq_zctrl_prog=0x%8x", *zq_zctrl_prog);
		//puts("OCD = ");puts(ODT_OCD_VALUE[sel][1]);puts("\n\r");

		//ZQ calibration start
		*zq_zctrl_prog |= ZCTRL_START;			//zctrl_start = 1
		//Polling to zq_cal_done
		polling_limit = 0x1000;
		while(!(*zq_zctrl_status & ZQ_CAL_DONE)){
			polling_limit--;
			if(polling_limit == 0){
				printf("ZQ: calibration ready polling timeout!\n");
				break;
			}
		}
		*zq_zctrl_prog &= ~ZCTRL_START; 		//zctrl_start = 0

		puts(", zq_zctrl_status=0x");puthex(*zq_zctrl_status);puts("\n\r");
		if((*zq_zctrl_status & 0x1fffff) == 0) {
			puts("ZQ: Doesn't detect R240, uses R480\n");
			en_R480 = 1;
			goto ENABLE_R480;
		}else if(((*zq_zctrl_status >> 22) & 0xf) != 0){
			puts("ZQ: "); puts(ODT_OCD_VALUE[sel].group); puts(" FAIL!\n\r");
			err_flag = 1;
		}
	}

	/*  ZQ power down  */
	REG32(0xb8142100) = 0x1f;
	delay_loop = 0x1000;
	while(delay_loop--);

	/* Write each ZQ calibraton to each group PAD */
	*zq_pad_ctrl_ck_ocd = DZQ_UP_SEL_0 << 0 | DZQ_UP_SEL_0 << 8 | DZQ_UP_SEL_0 << 16 | DZQ_UP_SEL_0 << 24;
	*zq_pad_ctrl_adr_cmd_ocd = DZQ_UP_SEL_1 << 0 | DZQ_UP_SEL_1 << 4 | DZQ_UP_SEL_1 << 8 | DZQ_UP_SEL_1 << 12 | DZQ_UP_SEL_1 << 16 | DZQ_UP_SEL_1 << 20;
	*zq_pad_ctrl_dq_ocd = DZQ_UP_SEL_2 << 0 | DZQ_UP_SEL_2 << 8;
	*zq_pad_ctrl_dq_odt = DZQ_UP_SEL_2 << 0 | DZQ_UP_SEL_2 << 8 | DZQ_UP_SEL_2 << 16 | DZQ_UP_SEL_2 << 24;
	*zq_pad_ctrl_dqs_ocd = DZQ_UP_SEL_3 << 0 | DZQ_UP_SEL_3 << 8 | DZQ_UP_SEL_3 << 16 | DZQ_UP_SEL_3 << 24;
	*zq_pad_ctrl_dqsp_odt = DZQ_UP_SEL_3 << 0 | DZQ_UP_SEL_3 << 8 | DZQ_UP_SEL_3 << 16 | DZQ_UP_SEL_3 << 24;
	*zq_pad_ctrl_dqsn_odt = DZQ_UP_SEL_3 << 0 | DZQ_UP_SEL_3 << 8 | DZQ_UP_SEL_3 << 16 | DZQ_UP_SEL_3 << 24;
	//		puts("Debug: zq_pad_ctrl_ck_ocd=0x");puthex(*zq_pad_ctrl_ck_ocd);puts("\n\r");
	//		puts("Debug: zq_pad_ctrl_adr_cmd_ocd=0x");puthex(*zq_pad_ctrl_adr_cmd_ocd);puts("\n\r");
	//		puts("Debug: zq_pad_ctrl_dq_ocd=0x");puthex(*zq_pad_ctrl_dq_ocd);puts("\n\r");
	//		puts("Debug: zq_pad_ctrl_dq_odt=0x");puthex(*zq_pad_ctrl_dq_odt);puts("\n\r");
	//		puts("Debug: zq_pad_ctrl_dqs_ocd=0x");puthex(*zq_pad_ctrl_dqs_ocd);puts("\n\r");
	//		puts("Debug: zq_pad_ctrl_dqsp_odt=0x");puthex(*zq_pad_ctrl_dqsp_odt);puts("\n\r");
	//		puts("Debug: zq_pad_ctrl_dqsn_odt=0x");puthex(*zq_pad_ctrl_dqsn_odt);puts("\n\r");

	/* Enable DRAM refresh operation */
	*dmcr = *dmcr &  (~DMCR_DIS_DRAM_REF_MASK) ;
	if (err_flag==1){
		ft_result |= (1 << 0); 
		puts("ZQ: Calibration FAIL!\n\r");
	}
	return;

}

int memctlc_ZQ_cali_value(unsigned int Z_prog_ODT,unsigned int Z_prog_OCD)
{
	unsigned int OCD_TBL[16][2]={
		{34,  0x7F},
		{36,  0x7D},
		{40,  0x79},
		{44,  0x75},
		{48,  0x3E},
		{53,  0x70},
		{60,  0x38},
		{65,  0x36},
		{72,  0x34},
		{80,  0x32},
		{90,  0x30},
		{103, 0x1a},
		{120, 0x18},
		{160, 0x15},
		{240, 0x12},
		{360, 0x10},
	};

	unsigned int P_N_ODT_TBL[13][2]={	//P_N_ODT=ODT*2
		{40,  0x1E},
		{51,  0x1A},
		{60,  0x18},
		{72,  0x0A},
		{80,  0x09},
		{90,  0x08},
		{103, 0x07},
		{120, 0x06},
		{144, 0x05},
		{180, 0x04},
		{240, 0x03},
		{360, 0x02},
		{720, 0x01},
	};

	unsigned int idx, odt_val, ocd_val;

	printf("ODT/OCD=%d/%d, ",Z_prog_ODT,Z_prog_OCD);
//	puts("ODT/OCD=");puthex(Z_prog_ODT);puts("/");puthex(Z_prog_OCD);
	idx=0;
    while((Z_prog_ODT*2) > P_N_ODT_TBL[idx][0]){
        idx++;
    }
	odt_val = P_N_ODT_TBL[idx][1];
//	puts("Debug: odt_val=");puthex(odt_val);puts("\n\r");

	idx=0;
    while(Z_prog_OCD > OCD_TBL[idx][0]){
        idx++;
    }
	ocd_val = OCD_TBL[idx][1];
//	puts("Debug: ocd_val=");puthex(ocd_val);puts("\n\r");
	return (ocd_val | odt_val <<7);

}

void set_patch_value(void){
	PATCH_VALUE[0].OCD = 34 + EFPH_CLK_OCD * 5;
	PATCH_VALUE[1].OCD = 34 + EFPH_Addr_OCD * 5;
	PATCH_VALUE[2].OCD = 34 + EFPH_DQ_OCD * 5;
	PATCH_VALUE[3].OCD = 34 + EFPH_DQS_OCD * 5;
	PATCH_VALUE[0].ODT = 30 + EFPH_Ctrl_ODT * 5;
	PATCH_VALUE[1].ODT = 30 + EFPH_Ctrl_ODT * 5;
	PATCH_VALUE[2].ODT = 30 + EFPH_Ctrl_ODT * 5;
	PATCH_VALUE[3].ODT = 30 + EFPH_Ctrl_ODT * 5;
}


void memctlc_config_misc(void)
{
	volatile unsigned int *mcr,*pollsrr,*mcbsrr;

	/* 1. Enable triple SYNC ()
	   2. Turn-on read buff full mask (RBF_MASK_EN=1)
	   3. Depend on OCP0_FRQ_SLOWER or OCP0_RBF_MASK_EN filed. */

	mcr = (volatile unsigned int *)MCR;
	pollsrr = (volatile unsigned int *)PBOLSRR;
	mcbsrr = (volatile unsigned int *)MCBSRR;

	*mcr |= MCR_RBF_MAS | MCR_TRIPLE_SYNC;
	*pollsrr |= POLLSRR_TRIPLE_SYNC;
	*mcbsrr |= MCBSRR_TRIPLE_SYNC;
	
	return;
}

#ifdef CONFIG_DRAM_AUTO_SIZE_DETECTION

unsigned int _dram_MCR_setting[6][5] __attribute__ ((section(".text")))=
{ 	{	0x10110000/* 8MB_DDR1_08b */,
		0x10120000/* 16MB_DDR1_08b */,
		0x10220000/* 32MB_DDR1_08b */,
		0x10230000/* 64MB_DDR1_08b */,
		0x10330000/* 128MB_DDR1_08b */
	},
	{	0x11100000/* 8MB_DDR1_16b */,
		0x11110000/* 16MB_DDR1_16b */,
		0x11210000/* 32MB_DDR1_16b */,
		0x11220000/* 64MB_DDR1_16b */,
		0x11320000/* 128MB_DDR1_16b */
	},
	{	0x10120000/* 16MB_DDR2_08b */,
		0x10220000/* 32MB_DDR2_08b */,
		0x10320000/* 64MB_DDR2_08b */,
		0x20320000/* 128MB_DDR2_08b */,
		0x20420000/* 256MB_DDR2_08b */
	},
	{	0x11110000/* 16MB_DDR2_16b */,
		0x11210000/* 32MB_DDR2_16b */,
		0x11220000/* 64MB_DDR2_16b */,
		0x21220000/* 128MB_DDR2_16b */,
		0x21320000/* 256MB_DDR2_16b */
	},
	{	0x00000000/* 16MB_DDR3_08b */,
		0x00000000/* 32MB_DDR3_08b */,
		0x20220000/* 64MB_DDR3_08b */,
		0x20320000/* 128MB_DDR3_08b */,
		0x20420000/* 256MB_DDR3_08b */
	},
	{	0x00000000/* 16MB_DDR3_16b */,
		0x00000000/* 32MB_DDR3_16b */,
		0x21120000/* 64MB_DDR3_16b */,
		0x21220000/* 128MB_DDR3_16b */,
		0x21320000/* 256MB_DDR3_16b */}
};

unsigned int _dram_type_setting[6][5] __attribute__ ((section(".text")))=		//format: n15: 1=16bit, n8..n4=tRFC, n2..n0=DRAM_type, n31..n16=DRAM_size
{ 	{	0x008004B2/* 8MB_DDR1_08b */,
		0x010004B2/* 16MB_DDR1_08b */,
		0x02000692/* 32MB_DDR1_08b */,
		0x04000802/* 64MB_DDR1_08b */,
		0x08000C62/* 128MB_DDR1_08b */
	},
	{	0x008084B2/* 8MB_DDR1_16b */,
		0x010084B2/* 16MB_DDR1_16b */,
		0x02008692/* 32MB_DDR1_16b */,
		0x04008802/* 64MB_DDR1_16b */,
		0x08008C62/* 128MB_DDR1_16b */
	},
	{	0x010004B2/* 16MB_DDR2_08b */,
		0x020004B2/* 32MB_DDR2_08b */,
		0x04000692/* 64MB_DDR2_08b */,
		0x08000802/* 128MB_DDR2_08b */,
		0x10000C62/* 256MB_DDR2_08b */
	},
	{	0x010084B2/* 16MB_DDR2_16b */,
		0x020084B2/* 32MB_DDR2_16b */,
		0x04008692/* 64MB_DDR2_16b */,
		0x08008802/* 128MB_DDR2_16b */,
		0x10008C62/* 256MB_DDR2_16b */
	},
	{	0x010004B3/* 16MB_DDR3_08b */,
		0x020005A3/* 32MB_DDR3_08b */,
		0x040005A3/* 64MB_DDR3_08b */,
		0x080006E3/* 128MB_DDR3_08b */,
		0x10000A03/* 256MB_DDR3_08b */
	},
	{	0x010084B3/* 16MB_DDR3_16b */,
		0x020085A3/* 32MB_DDR3_16b */,
		0x040085A3/* 64MB_DDR3_16b */,
		0x080086E3/* 128MB_DDR3_16b */,
		0x10008A03/* 256MB_DDR3_16b */}
};

unsigned int _dram_detection_addr[6][5] __attribute__ ((section(".text")))=
{ 	{	0xA63809A4/* 8MB_DDR1_08b */,
		0xA6380BA4/* 16MB_DDR1_08b */,
		0xA6780BA4/* 32MB_DDR1_08b */,
		0xA0000000/* 64MB_DDR1_08b */,
		0xA0000000/* 128MB_DDR1_08b */
	},
	{	0xA6701148/* 8MB_DDR1_16b */,
		0xA6701348/* 16MB_DDR1_16b */,
		0xA6F01348/* 32MB_DDR1_16b */,
		0xA6F01748/* 64MB_DDR1_16b */,
		0xA7F01748/* 128MB_DDR1_16b */
	},
	{	0xA6380BA4/* 16MB_DDR2_08b */,
		0xA6780BA4/* 32MB_DDR2_08b */,
		0xA6F80BA4/* 64MB_DDR2_08b */,
		0xAEF80BA4/* 128MB_DDR2_08b */,
		0xAFF80BA4/* 256MB_DDR2_08b */
	},
	{	0xA6701348/* 16MB_DDR2_16b */,
		0xA6F01348/* 32MB_DDR2_16b */,
		0xA6F01748/* 64MB_DDR2_16b */,
		0xAEF01748/* 128MB_DDR2_16b */,
		0xAFF01748/* 256MB_DDR2_16b */
	},
	{	0xA0000000/* 16MB_DDR3_08b */,
		0xA0000000/* 32MB_DDR3_08b */,
		0xAE780BA4/* 64MB_DDR3_08b */,
		0xAEF80BA4/* 128MB_DDR3_08b */,
		0xAFF80BA4/* 256MB_DDR3_08b */
	},
	{	0xA0000000/* 16MB_DDR3_16b */,
		0xA0000000/* 32MB_DDR3_16b */,
		0xAE701548/* 64MB_DDR3_16b */,
		0xAEF01548/* 128MB_DDR3_16b */,
		0xAFF01548/* 256MB_DDR3_16b */
	}
};

#else
unsigned int ddr2_8bit_size[] __attribute__ ((section(".text")))=
	{  0x10120000/*16MB*/    , 0x10220000/* 32MB */, 0x10320000/* 64MB */,
	   0x20320000/* 128MB */, 0x20420000/* 256MB */, 0x20520000/* 512MB */};
unsigned int ddr2_16bit_size[] __attribute__ ((section(".text")))=
	{ 0x11110000/*16MB*/,     0x11210000/* 32MB */, 0x11220000/* 64MB */,
	  0x21220000/* 128MB */, 0x21320000/* 256MB */, 0x21420000/* 512MB */,
	  0x21520000/*1GB*/};
#ifdef CONFIG_DRAM_AUTO_SIZE_DETECTION
unsigned int dram_test_addr[] __attribute__((section(".text")))=
	{ 0xa7f01354/* 32MB*/, 0xa7f01754/* 64MB */, 0xaef01754/* 128MB */,
	   0xadf01754/* 256MB*/, 0xabf01754/* 512MB */, 0xaff01754/* 1GB */};
#endif
#endif
unsigned int memctlc_config_DRAM_size(void)
{
	volatile unsigned int *dcr;
	unsigned int *size_arry;
	unsigned int dcr_value=0, dram_size=0x2000000;
#ifdef CONFIG_DRAM_AUTO_SIZE_DETECTION
	volatile unsigned int *dram_addr;
	unsigned int i;
	unsigned int DDR_para_index=0, DDR_width=16, loc=0;
#endif

	dcr = (volatile unsigned int *)DCR;

#ifdef CONFIG_DRAM_AUTO_SIZE_DETECTION

	if(memctlc_DDR_Type()==IS_DDR3_SDRAM)
		DDR_para_index = 4;
	else if(memctlc_DDR_Type()==IS_DDR2_SDRAM)
		DDR_para_index = 2;
	else
		DDR_para_index = 0;

	DDR_width =  8 << ((REG32(DCR) & DCR_DBUSWID_MASK) >> DCR_DBUSWID_FD_S) ;

	loc=(DDR_width/8-1) + DDR_para_index;
	size_arry =  &_dram_MCR_setting[loc][0];

	*dcr = size_arry[4];
	_memctl_update_phy_param();

	dram_addr = (volatile unsigned int *)_dram_detection_addr[loc][4];
	*dram_addr = 0x5A0FF0A5;

	/* Assign 64MBytes DRAM parameters as default value */
	dcr_value = _dram_MCR_setting[loc][2];
	dram_size = _dram_type_setting[loc][2];

#if 0	/* Unknown issue : DRAM size detection must be set from small size to big size.*/

	for(i=(sizeof(_dram_detection_addr[loc])/sizeof(unsigned int)); i>0; i--){
		if( REG32(_dram_detection_addr[loc][i-1]) != 0x5A0FF0A5 ){
			dcr_value = _dram_MCR_setting[loc][i];
			#ifdef CONFIG_RTL8685
			/* Enable RTL8685 memory controller jitter tolerance*/
			dcr_value |= (1<<31);
			#endif /* CONFIG_RTL8685 */
			dram_size = ((_dram_type_setting[loc][i]) & 0xFFFF0000);
			break;
		}
	}
#else
	for(i=0; i<(sizeof(_dram_detection_addr[loc])/sizeof(unsigned int)); i++){
		if( REG32(_dram_detection_addr[loc][i]) == 0x5A0FF0A5 ){
			dcr_value = _dram_MCR_setting[loc][i];
			dram_size = ((_dram_type_setting[loc][i]) & 0xFFFF0000);
			break;
		}
	}
#endif
	if(dcr_value==0x21320000){
		*dcr = 0x21520000;		//set 8Gbit
		_memctl_update_phy_param();

		REG32(0xA9F01754)=0x0;
		REG32(0xABF01754)=0x0;
		REG32(0xAFF01754)=0x0;
		dram_addr = (volatile unsigned int *)0xAFF01754;
		*dram_addr = 0x5A0FF0A5;

//		puts("0xA9F01754 = 0x");puthex(REG32(0xA9F01754));puts("\n\r");
//		puts("0xABF01754 = 0x");puthex(REG32(0xABF01754));puts("\n\r");
//		puts("0xAFF01754 = 0x");puthex(REG32(0xAFF01754));puts("\n\r");

		volatile unsigned int work_around __attribute__((unused)) = REG32(0xA0000000);		//IF REMOVE, AUTO DETECT WILL BE FAIL!! UNKNOW ISSUE!!

		if(REG32(0xA9F01754)==0x5A0FF0A5){		// 2Gbit detect
			*dcr = 0x21320000;
			dram_size = 0x10000000;				//dram_size = 0x10008A03;				//format: n15: 1=16bit, n8..n4=tRFC, n2..n0=DRAM_type, n31..n16=DRAM_size
		}else if(REG32(0xABF01754)==0x5A0FF0A5){ 	// 4Gbit detect
			*dcr = 0x21420000;
			dram_size = 0x20000000;				//dram_size = 0x20009043;
		}else if(REG32(0xAFF01754)==0x5A0FF0A5){ 	// 8Gbit detect
			*dcr = 0x21520000;
			dram_size = 0x40000000;				//dram_size = 0x400095E3;
		}else{
			*dcr = 0x21320000;					// 2Gbit detect
			dram_size = 0x10000000;				//dram_size = 0x10008A03;
			}
		_memctl_update_phy_param();
	}else{
	*dcr = dcr_value;
	_memctl_update_phy_param();
		}

#else /* CONFIG_DRAM_AUTO_SIZE_DETECTION */

	dram_size = CONFIG_ONE_DRAM_CHIP_SIZE;
	#ifdef CONFIG_DRAM_BUS_WIDTH_8BIT
		size_arry = &ddr2_8bit_size[0];
	#else
		size_arry = &ddr2_16bit_size[0];
	#endif

	switch (dram_size){
		case 0x1000000: /* 16MB */
			dcr_value = size_arry[0];
			break;
		case 0x4000000: /* 64MB */
			dcr_value = size_arry[2];
			break;
		case 0x8000000: /* 128MB */
			dcr_value = size_arry[3];
			break;
		case 0x10000000: /* 256MB */
			dcr_value = size_arry[4];
			break;
		case 0x20000000: /* 512MB */
			dcr_value = size_arry[5];
			break;
		default: /* 32MB */
			dcr_value = size_arry[1];
			break;
	}

	*dcr = dcr_value;
#endif
	puts("DRAM size=0x");puthex(dram_size);puts(", ");
	if(EFVD_Capacity==0){
		// no diagnosis
	}else if(((EFVD_Capacity==1) && (dram_size!=0x4000000)) ||
			((EFVD_Capacity==2) && (dram_size!=0x8000000)) ||
			((EFVD_Capacity==3) && (dram_size!=0x10000000)) ||
			((EFVD_Capacity==4) && (dram_size!=0x20000000))){
		printf("ERROR: Detecting size is different with real DRAM size??\n");
	}

	return dram_size;

}

unsigned int ddr2_16bit_size[] __attribute__ ((section(".text")))=
	{ 0x11110000/* 16MB */,		0x11210000/* 32MB */,	0x11220000/* 64MB */,
	  0x21220000/* 128MB */,	0x21320000/* 256MB */,	0x21420000/* 512MB */,
	  0x00000000/* dummy */};

unsigned int ddr3_16bit_size[] __attribute__ ((section(".text")))=
	{ 0x00000000/* dummy */,	0x00000000/* dummy */,	0x21120000/* 64MB */,
	  0x21220000/* 128MB */,	0x21320000/* 256MB*/,	0x21420000/* 512MB */,
	  0x21520000/* 1GB */};

void memctlc_DRAM_size_recovery(unsigned int dram_size)
{
	volatile unsigned int *dcr;
	short size_idx;
	dcr = (volatile unsigned int *)DCR;

	/* Save real DRAM size @ SRAM (0x9F00006C) */
	_soc.dram_info = (void *)dram_size;

	/* Read efuse value, recover DRAM size  */
	size_idx = xlat_dram_size_num();		//size_idx = 0:16MB  1:32MB  2:64MB  3:128MB  4:256MB  5:512MB  6:1GB  -9=NA
//	printf("size_idx = %d\n",size_idx);

	if(size_idx != SIZE_NA){
		if(memctlc_DDR_Type()==IS_DDR3_SDRAM){
			*dcr = 	ddr3_16bit_size[size_idx];
		}else{
			*dcr =	ddr2_16bit_size[size_idx];
		}
		_memctl_update_phy_param();
//		printf("DCR = 0x%x\n",*dcr);
	}

	return;
}


#ifdef DDR2_USAGE

enum DDR2_FREQ_SEL {
	DDR2_650 = 0,
	DDR2_625,
	DDR2_600,
	DDR2_575,
	DDR2_550,
	DDR2_525,
	DDR2_500,
	DDR2_450,
	DDR2_400,
	DDR2_350,
	DDR2_300,
	DDR2_200
};

unsigned int tRFC_Spec_DDR2[] __attribute__ ((section(".text")))= {
	75, 	/*128Mbit*/
	75, 	/*256Mbit*/
	105, /*512Mbit*/
	128, /*1Gbit*/
	198, /*2Gbit*/
	328, /*4Mbit*/
};

unsigned int DDR2_DTR[12][3] __attribute__ ((section(".text")))={
	{	/* DDR2, 650MHz, CAS=7, WR=8 */
		0x67599826,
		0x0808041F,
		0x0001D000},
	{	/* DDR2, 625MHz, CAS=7, WR=8 */
		0x67588826,
		0x0808061F,
		0x0001C000},
	{	/* DDR2, 600MHz, CAS=7, WR=8 */
		0x67588826,
		0x0808061E,
		0x0001C000},
	{	/* DDR2, 575MHz, CAS=7, WR=8 */
		0x67544726,
		0x0606061E,
		0x0001A000},
	{	/* DDR2, 550MHz, CAS=7, WR=8 */
		0x67544726,
		0x0606061E,
		0x00019000},
	{	/* DDR2, 525MHz, CAS=7, WR=8 */
		0x67544726,
		0x0606051C,
		0x00017000},
	{	/* DDR2, 500MHz, CAS=7, WR=8 */
		0x67544626,
		0x0606041B,
		0x00013000},
	{	/* DDR2, 450MHz, CAS=7, WR=8*/
		0x67533626,
		0x06060418,
		0x00011000},
	{	/* DDR2, 400MHz, CAS=6, WR=6*/
		0x56433525,
		0x05050315,
		0x0000F000},
	{	/* DDR2, 350MHz, CAS=5, WR=5*/
		0x55432425,
		0x05050312,
		0x0000D000},
	{	/* DDR2, 300MHz, CAS=5, WR=5*/
		0x45322324,
		0x04040210,
		0x0000B000},
	{	/* DDR2, 200MHz, CAS=5, WR=4*/
		0x33233223,
		0x0303010A,
		0x00008000},
};

void memctlc_config_DDR2_DTR(unsigned int default_instruction, unsigned int dram_size)
{
	volatile unsigned int *dtr0, *dtr1, *dtr2;
	unsigned int dram_freq_mhz=0;
	unsigned int DRAM_capacity_index=0, DRAM_freq_index=0, tRFC_extend=3, dtr2_temp=0;
	unsigned int *tRFC;

	dtr0 = (volatile unsigned int *)DTR0;
	dtr1 = (volatile unsigned int *)DTR1;
	dtr2 = (volatile unsigned int *)DTR2;

	dram_freq_mhz = board_DRAM_freq_mhz();

	if(default_instruction == 1){

		/* Default instruction, set the DRAM as the maximun size */
		DRAM_capacity_index = 5;

	}else{

		/* Search from DDR1 base size 0x1000000 => 16M Bytes */
		for(DRAM_capacity_index=0; DRAM_capacity_index<6; DRAM_capacity_index++){
			if(dram_size == ((0x1000000) << DRAM_capacity_index)){
				break;
			}
		}
	}

	/* Set as default value */
	tRFC = &tRFC_Spec_DDR2[3];	// 1GB
	switch(dram_freq_mhz){
		case 650:
			DRAM_freq_index = DDR2_650;
			break;
		case 625:
			DRAM_freq_index = DDR2_625;
			break;
		case 600:
			DRAM_freq_index = DDR2_600;
			break;
		case 575:
			DRAM_freq_index = DDR2_575;
			break;
		case 550:
			DRAM_freq_index = DDR2_550;
			break;
		case 525:
			DRAM_freq_index = DDR2_525;
			break;
		case 500:
			DRAM_freq_index = DDR2_500;
			break;
		case 450:
			DRAM_freq_index = DDR2_450;
			break;
		case 400:
			DRAM_freq_index = DDR2_400;
			break;
		case 350:
			DRAM_freq_index = DDR2_350;
			break;
		case 300:
			DRAM_freq_index = DDR2_300;
			break;
		case 200:
			DRAM_freq_index = DDR2_200;
			break;
		default:
			DRAM_freq_index = DDR2_300;
			break;
	}
#ifdef	CONFIG_TCAS_MAC_PHY
	*dtr0 = (DDR2_DTR[DRAM_freq_index][0] + 0x10000000 );
	//*dtr0 = DDR2_DTR[DRAM_freq_index][0];
#else
	*dtr0 = DDR2_DTR[DRAM_freq_index][0];
#endif
	*dtr1 = DDR2_DTR[DRAM_freq_index][1];
	dtr2_temp = DDR2_DTR[DRAM_freq_index][2];

	*dtr2=dtr2_temp |
		((((tRFC[DRAM_capacity_index]*dram_freq_mhz)/1000)+tRFC_extend)<<DTR2_RFC_FD_S);

	return;

}


#endif /* DDR2_USAGE */

#ifdef DDR3_USAGE

enum DDR3_FREQ_SEL {
	DDR3_800 = 0,
	DDR3_750,
	DDR3_700,
	DDR3_650,
	DDR3_600,
	DDR3_550,
	DDR3_500,
	DDR3_450,
	DDR3_400,
	DDR3_350,
	DDR3_300,
	DDR3_200
};

unsigned int tRFC_Spec_DDR3[] __attribute__ ((section(".text")))= {
	90, /*512Mbit*/
	110, /*1Gbit*/
	160, /*2Gbit*/
	260, /*4Gbit*/
	350, /*8Gbit*/
};

unsigned int DDR3_DTR[12][3] __attribute__ ((section(".text")))={
	{	/* DDR3, 800MHz, 11-11-11 */
		0xBD76653B,
		0x0B0B0623,
		0x0001D00B},	//EN_TCC_DIFF, TAC=PHY-CWL
	{	/* DDR3, 750MHz, 11-11-11 */
		0xAB75543A,
		0x0A0A051F,
		0x0001B00B},	//EN_TCC_DIFF, TAC=PHY-CWL
	{	/* DDR3, 700MHz, 11-11-11 */
		0xAA75543A,
		0x0A0A051F,
		0x0001A00B},	//EN_TCC_DIFF, TAC=PHY-CWL
	{	/* DDR3, 650MHz, 9-9-9 */
		0x89655828,
		0x0909061F, 
		0x00019000},
	{	/* DDR3, 600MHz, 9-9-9 */
		0x89644828,
		0x0808051F, 
		0x00016000},
	{	/* DDR3, 550MHz, 9-9-9 */
		0x88644728,
		0x0808051E,
		0x00014000},
	{	/* DDR3, 500MHz, 7-8-8 */
		0x67533626,	//new, 0x77533637 can not work!
		0x0707031B,	//new, 0x0707041B can not work!
		0x00013000},
	{	/* DDR3, 450MHz, 7-8-8 */
		0x67533626,
		0x07070418,
		0x00011000},
	{	/* DDR3, 400MHz, 6-6-6 */
		0x56433525,
		0x05050315,
		0x0000F000},
	{	/* DDR3, 350MHz, 6-6-6 */
		0x55433425,
		0x05050312,
		0x0000D000},
	{	/* DDR3, 300MHz, 6-6-6 */
		0x55433325,
		0x05050210,
		0x0000B000},
	{	/* DDR3, 200MHz, 5--5-5 */
		0x43433224,
		0x0303010A,
		0x00008000},
};

void memctlc_config_DDR3_DTR(unsigned int default_instruction, unsigned int dram_size)
{
	volatile unsigned int *dtr0, *dtr1, *dtr2;
	unsigned int dram_freq_mhz=0;
	unsigned int DRAM_capacity_index=0, DRAM_freq_index=0, tRFC_extend=3, dtr2_temp=0;
	unsigned int *tRFC;

	dtr0 = (volatile unsigned int *)DTR0;
	dtr1 = (volatile unsigned int *)DTR1;
	dtr2 = (volatile unsigned int *)DTR2;

	dram_freq_mhz = board_DRAM_freq_mhz();

	if(default_instruction == 1){

		/* Default instruction, set the DRAM as the maximun size */
		DRAM_capacity_index = 4;

	}else{

		/* Search from DDR3 base size 0x1000000 => 64M Bytes */
		for(DRAM_capacity_index=0; DRAM_capacity_index<6; DRAM_capacity_index++){
			if(dram_size == ((0x4000000) << DRAM_capacity_index)){
				break;
			}
		}
	}

	/* Set as default value */
	tRFC = &tRFC_Spec_DDR3[0];
	switch(dram_freq_mhz){
		case 650:
			DRAM_freq_index = DDR3_650;
			break;
		case 600:
			DRAM_freq_index = DDR3_600;
			break;
		case 550:
			DRAM_freq_index = DDR3_550;
			break;
		case 500:
			DRAM_freq_index = DDR3_500;
			break;
		case 450:
			DRAM_freq_index = DDR3_450;
			break;
		case 400:
			DRAM_freq_index = DDR3_400;
			break;
		case 350:
			DRAM_freq_index = DDR3_350;
			break;
		case 300:
			DRAM_freq_index = DDR3_300;
			break;
		case 200:
			DRAM_freq_index = DDR3_200;
			break;
		default:
			DRAM_freq_index = DDR3_300;
			break;
	}
#ifdef	CONFIG_TCAS_MAC_PHY
	*dtr0 = (DDR3_DTR[DRAM_freq_index][0] + 0x10000000 );
	//*dtr0 = DDR2_DTR[DRAM_freq_index][0];
#else
	*dtr0 = DDR3_DTR[DRAM_freq_index][0];
#endif
	*dtr1 = DDR3_DTR[DRAM_freq_index][1];
	dtr2_temp = DDR3_DTR[DRAM_freq_index][2];

	*dtr2=dtr2_temp |
		((((tRFC[DRAM_capacity_index]*dram_freq_mhz)/1000)+tRFC_extend)<<DTR2_RFC_FD_S);

	return;

}

#endif /* DDR3_USAGE */

#ifdef  CONFIG_DRAM_AUTO_TIMING_SETTING
void memctlc_config_DTR(unsigned int default_instruction, unsigned int dram_size)
#else
void memctlc_config_DTR(void)
#endif
{
	volatile unsigned int *dtr0, *dtr1, *dtr2, *dcr;
#ifdef	CONFIG_DRAM_AUTO_TIMING_SETTING
	unsigned int dram_freq_mhz;
	unsigned int dram_base_size=0, dtr2_temp = 0,tRFC_temp;
	unsigned int DRAM_capacity_index=0;
	unsigned int DRAM_freq_index=0;
	unsigned int *tRFC = 0;
	unsigned char tRFC_extend=0,i,tref_unit=1;
#endif
	dcr = (volatile unsigned int *)DCR;
	dtr0 = (volatile unsigned int *)DTR0;
	dtr1 = (volatile unsigned int *)DTR1;
	dtr2 = (volatile unsigned int *)DTR2;

#ifndef CONFIG_DRAM_AUTO_TIMING_SETTING
	*dtr0 = CONFIG_DRAM_DTR0;
	*dtr1 = CONFIG_DRAM_DTR1;
	*dtr2 = CONFIG_DRAM_DTR2;
#else /* CONFIG_DRAM_AUTO_TIMING_SETTING */

	dram_freq_mhz = board_DRAM_freq_mhz();
	//puts("dram_freq_mhz=0x");puthex(dram_freq_mhz);puts("\n");

	if(default_instruction == 1){
		//puts("DCR=0x");puthex(*dcr);puts("\n");
		*dcr = 0x21320000;			// 2Gbits, both DDR2 and DDR3
		/* Default instruction, set the DRAM as the maximun size */
		if(memctlc_DDR_Type()==IS_DDR2_SDRAM){
			DRAM_capacity_index = 5;
		}else if (memctlc_DDR_Type()==IS_DDR3_SDRAM){
			DRAM_capacity_index = 4;
		}else
			DRAM_capacity_index = 4;

	}else{
		/* Provide the minmun dram size as base */
		if(memctlc_DDR_Type()==IS_DDR2_SDRAM){
			dram_base_size = 0x1000000; 	//min capacite 16Mbytes (128Mb)
			tRFC_extend = 3;
		}else if (memctlc_DDR_Type()==IS_DDR3_SDRAM){
			dram_base_size = 0x4000000; 	//min capacite 64Mbytes (512Mb)
			tRFC_extend = 5;
		}else{
			puts("DDR type fail...only supported DDR2 and DDR3\n");
			}

		for(DRAM_capacity_index=0; DRAM_capacity_index<6; DRAM_capacity_index++){
			if(dram_size == ((dram_base_size) << DRAM_capacity_index)){
				break;
			}
		}
	}
	if(memctlc_DDR_Type()==IS_DDR2_SDRAM){
		tRFC = &tRFC_Spec_DDR2[0];
		switch(dram_freq_mhz){
			case 650:
				DRAM_freq_index = DDR2_650;
				break;
			case 625:
				DRAM_freq_index = DDR2_625;
				break;
			case 600:
				DRAM_freq_index = DDR2_600;
				break;
			case 575:
				DRAM_freq_index = DDR2_575;
				break;
			case 550:
				DRAM_freq_index = DDR2_550;
				break;
			case 525:
				DRAM_freq_index = DDR2_525;
				break;
			case 500:
				DRAM_freq_index = DDR2_500;
				break;
			case 450:
				DRAM_freq_index = DDR2_450;
				break;
			case 400:
				DRAM_freq_index = DDR2_400;
				break;
			case 350:
				DRAM_freq_index = DDR2_350;
				break;
			case 300:
				DRAM_freq_index = DDR2_300;
				break;
			case 200:
				DRAM_freq_index = DDR2_200;
				break;
			default:
				DRAM_freq_index = DDR2_300;
				break;
		}
#ifdef	CONFIG_TCAS_MAC_PHY
		*dtr0 = (DDR2_DTR[DRAM_freq_index][0] + 0x10000000 );
#else
		*dtr0 = DDR2_DTR[DRAM_freq_index][0];
#endif
		*dtr1 = DDR2_DTR[DRAM_freq_index][1];
		dtr2_temp = DDR2_DTR[DRAM_freq_index][2];
	}else if(memctlc_DDR_Type()==IS_DDR3_SDRAM){
		tRFC = &tRFC_Spec_DDR3[0];
		switch(dram_freq_mhz){
			case 800:
				DRAM_freq_index = DDR3_800;
				break;
			case 750:
				DRAM_freq_index = DDR3_750;
				break;
			case 700:
				DRAM_freq_index = DDR3_700;
				break;
			case 650:
				DRAM_freq_index = DDR3_650;
				break;
			case 600:
				DRAM_freq_index = DDR3_600;
				break;
			case 550:
				DRAM_freq_index = DDR3_550;
				break;
			case 500:
				DRAM_freq_index = DDR3_500;
				break;
			case 450:
				DRAM_freq_index = DDR3_450;
				break;
			case 400:
				DRAM_freq_index = DDR3_400;
				break;
			case 350:
				DRAM_freq_index = DDR3_350;
				break;
			case 300:
				DRAM_freq_index = DDR3_300;
				break;
			case 200:
				DRAM_freq_index = DDR3_200;
				break;
			default:
				DRAM_freq_index = DDR3_300;
				break;
		}
#ifdef	CONFIG_TCAS_MAC_PHY
		*dtr0 = (DDR3_DTR[DRAM_freq_index][0] + 0x10000000 );
#else
		*dtr0 = DDR3_DTR[DRAM_freq_index][0];
#endif
		*dtr1 = DDR3_DTR[DRAM_freq_index][1];
		dtr2_temp = DDR3_DTR[DRAM_freq_index][2];
	}
	if(default_instruction != 1){
		printf("tRFC[%d]=0x%d(nS), ",DRAM_capacity_index,(tRFC[DRAM_capacity_index]+tRFC_extend));
//		puts("tRFC[0x");puthex(DRAM_capacity_index);puts("]=0x");puthex(tRFC[DRAM_capacity_index]);
//		puts("AK: DTR: tRFC=");puthex((1000*tRFC[DRAM_capacity_index])/dram_freq_mhz);puts(" nS\n");
//		puts("=0x");puthex(tRFC[DRAM_capacity_index]+tRFC_extend);puts("nS, ");
		for(i=0;i<((REG32(0xb8001008)>>4&0xf)+5);i++)
			tref_unit = tref_unit * 2;
		printf("tREF=%d(nS)\n",(1000 * tref_unit * ((REG32(0xb8001008)>>8&0xf)+1) /dram_freq_mhz));
	}
	tRFC_temp = ((tRFC[DRAM_capacity_index]*dram_freq_mhz)/1000)+tRFC_extend;
	if (tRFC_temp>0xFF){	//The Max tRFC is 0xFF.
		tRFC_temp=0xFF;
		if(default_instruction == 1)
			puts("WARNING! There isn't enough tRFC bit to set! tRFC set maximum(0xFF)");
	}
	*dtr2=dtr2_temp | (tRFC_temp<<DTR2_RFC_FD_S);

/*	if(default_instruction != 1){
    	printf("DTR[0:2]=0x%x, 0x%x, 0x%x\n", *dtr0, *dtr1, *dtr2);
    }
*/
#endif /* CONFIG_DRAM_AUTO_TIMING_SETTING */
		return;
}

void memctlc_config_delay_line(unsigned int dram_freq_mhz){
	unsigned char analog_delay_disable=1, dynamic_FIFO_rst=0, buffer_poniter=0;
	unsigned char DQS0_EN,DQS1_EN,DQS0_GROUP,DQS1_GROUP,HCLK_EN,DQM0_DLY,DQM1_DLY;

//ONLY digital delay line
	analog_delay_disable=1;
	if (otto_is_apro()) {
		if(memctlc_DDR_Type()==2){	//DDR2
			dynamic_FIFO_rst = 1;
			buffer_poniter = 1;
			DQS0_GROUP=0x0;		// DQS GROUP delay will be auto calibration.
			DQS1_GROUP=0x0;		// DQS GROUP delay will be auto calibration.
			HCLK_EN=0;			
			DQS0_EN=DQS1_EN=0x0;
			DQM0_DLY=0xa;		// DQM delay initial value, DDR calibration calibrates the final value
			DQM1_DLY=0xa;
		}else{  //DDR3
			dynamic_FIFO_rst = 1;
			buffer_poniter = 1;
			DQS0_GROUP=0x0;		// DQS GROUP delay will be auto calibration
			DQS1_GROUP=0x0;		// DQS GROUP delay will be auto calibration
			HCLK_EN=1;
			DQS0_EN=DQS1_EN=0x1f;
			DQM0_DLY=0xe;		// DQM delay initial value, DDR calibration calibrates the final value
			DQM1_DLY=0x8;
		}
		// Winbond patch
		if(EFVD_type==3 && EFVD_Vendor==1 && EFVD_Capacity==3){
			HCLK_EN=0;
			DQS0_EN=DQS1_EN=0xf;
		}
		// Etron patch
		if(EFVD_type==3 && EFVD_Vendor==3 && EFVD_Capacity==3 && EFVD_other==2){
			HCLK_EN=0;
			DQS0_EN=DQS1_EN=0xf;
		}		
		// DRAM patch from efuse
		if (EFPH_patch_num!=0 && EFPH_DM_delay_en==1){	
			if(EFPH_DM_delay<8){
				if(DQM0_DLY>=(EFPH_DM_delay+1))
					DQM0_DLY = DQM0_DLY - (EFPH_DM_delay+1);
				else
					DQM0_DLY = 0;
				if(DQM1_DLY>=(EFPH_DM_delay+1))
					DQM1_DLY = DQM1_DLY - (EFPH_DM_delay+1);
				else
					DQM1_DLY = 0;
			}else{
				DQM0_DLY = DQM0_DLY + (EFPH_DM_delay+1-8);
				DQM1_DLY = DQM1_DLY + (EFPH_DM_delay+1-8);
			}
		}
	}else{
		if(memctlc_DDR_Type()==2){	//DDR2
			dynamic_FIFO_rst = 1;
			buffer_poniter = 1;
			DQS0_GROUP=0x0;		// DQS GROUP delay will be auto calibration.
			DQS1_GROUP=0x0;		// DQS GROUP delay will be auto calibration.
			HCLK_EN=1;			// Allen suggests value
			DQS0_EN=DQS1_EN=0x2;// Allen suggests value
			DQM0_DLY=0x8;		// DQM delay initial value, DDR calibration calibrates the final value
			DQM1_DLY=0x8;
		}else{  //DDR3
			dynamic_FIFO_rst = 1;
			buffer_poniter = 1;
			DQS0_GROUP=0x0;		// DQS GROUP delay will be auto calibration
			DQS1_GROUP=0x0;		// DQS GROUP delay will be auto calibration
			HCLK_EN=1;			// Allen suggests value
			DQS0_EN=DQS1_EN=0x2;// Allen suggests value
			DQM0_DLY=0x8;		// DQM delay initial value, DDR calibration calibrates the final value
			DQM1_DLY=0x8;
		}
	}
	// DRAM patch from efuse
	if (EFPH_patch_num!=0 && EFPH_DQSEN_en==1){	
		HCLK_EN = EFPH_HCLK;
		DQS0_EN=DQS1_EN = EFPH_DQSEN*2;
	}

	if (otto_is_apro()) {	//For Apollo pro
		if(xlat_dram_size_num()==NA){	//For discrete
			switch(dram_freq_mhz){
				case 800:
				case 750:
				case 700:
				case 650:
					REG32(DDCR) = ((HCLK_EN&0x1)<<31) |((DQS0_EN&0x1F)<<24) |((HCLK_EN&0x1)<<23)  | ((DQS1_EN&0x1F)<<16);						//DQS0 HCLK, DQS0 EN, DQS1 HCLK, DQS1 EN
					REG32(DACCR) = ((analog_delay_disable&0x1)<<31) | ((DQS0_GROUP&0x1F)<< 16) |((DQS1_GROUP&0x1F)<< 8)|(dynamic_FIFO_rst<<5)|(buffer_poniter<<4);	/* Assign DQS0 and DQS1 group delay */
					REG32(DCDQMR) = (DQM1_DLY&0x1f)<<16 | (DQM0_DLY&0x1f)<<24;
					REG32(DWDMOR) = (DQM0_DLY&0x1f)<<16 | (DQM1_DLY&0x1f)<<24;	
					REG32(DDR_DELAY_CTRL_REG0)=0x0fffffff;			//Write: CKE_DLY[n27:24],CS1_DLY[n23:20],CS0_DLY[n19:16],ODT_DLY[n15:12],RAS_DLY[n11:8],CAS_DLY[7:4],WE_DLY[n3:0]
					REG32(DDR_DELAY_CTRL_REG1)=0x12120fff;
					REG32(DDR_DELAY_CTRL_REG2)=0xffffffff;
					REG32(DDR_DELAY_CTRL_REG3)=0xffffffff;
						REG32(DWDQSOR) = 0x1f1f0000;
				case 600:
					REG32(DDCR) = ((HCLK_EN&0x1)<<31) |((DQS0_EN&0x1F)<<24) |((HCLK_EN&0x1)<<23)  | ((DQS1_EN&0x1F)<<16);						//DQS0 HCLK, DQS0 EN, DQS1 HCLK, DQS1 EN
					REG32(DACCR) = ((analog_delay_disable&0x1)<<31) | ((DQS0_GROUP&0x1F)<< 16) |((DQS1_GROUP&0x1F)<< 8)|(dynamic_FIFO_rst<<5)|(buffer_poniter<<4);	/* Assign DQS0 and DQS1 group delay */
					REG32(DCDQMR) = (DQM1_DLY&0x1f)<<16 | (DQM0_DLY&0x1f)<<24;
					REG32(DWDMOR) = (DQM0_DLY&0x1f)<<16 | (DQM1_DLY&0x1f)<<24;	
					REG32(DDR_DELAY_CTRL_REG0)=0x0fffffff;
					REG32(DDR_DELAY_CTRL_REG1)=0x0c0c0fff;
					REG32(DDR_DELAY_CTRL_REG2)=0xffffffff;
					REG32(DDR_DELAY_CTRL_REG3)=0xffffffff;
					REG32(DWDQSOR) = 0x1f1f0000;
					break;
				case 575:
				case 550:	
				case 525:
				case 500:
					REG32(DDCR) = ((HCLK_EN&0x1)<<31) |((DQS0_EN&0x1F)<<24) |((HCLK_EN&0x1)<<23)  | ((DQS1_EN&0x1F)<<16);						//DQS0 HCLK, DQS0 EN, DQS1 HCLK, DQS1 EN
					REG32(DACCR) = ((analog_delay_disable&0x1)<<31) | ((DQS0_GROUP&0x1F)<< 16) |((DQS1_GROUP&0x1F)<< 8)|(dynamic_FIFO_rst<<5)|(buffer_poniter<<4);	/* Assign DQS0 and DQS1 group delay */
					REG32(DCDQMR) = (DQM1_DLY&0x1f)<<16 | (DQM0_DLY&0x1f)<<24;
					REG32(DWDMOR) = (DQM0_DLY&0x1f)<<16 | (DQM1_DLY&0x1f)<<24;	
					REG32(DDR_DELAY_CTRL_REG0)=0x0fffffff;
					REG32(DDR_DELAY_CTRL_REG1)=0x0c0c0fff;
					REG32(DDR_DELAY_CTRL_REG2)=0xffffffff;
					REG32(DDR_DELAY_CTRL_REG3)=0xffffffff;
					REG32(DWDQSOR) = 0x1f1f0000;
					break;
				case 450:
				case 400:
				case 350:
				case 300:
					REG32(DDCR) = ((HCLK_EN&0x1)<<31) |((DQS0_EN&0x1F)<<24) |((HCLK_EN&0x1)<<23)  | ((DQS1_EN&0x1F)<<16);						//DQS0 HCLK, DQS0 EN, DQS1 HCLK, DQS1 EN
					REG32(DACCR) = ((analog_delay_disable&0x1)<<31) | ((DQS0_GROUP&0x1F)<< 16) |((DQS1_GROUP&0x1F)<< 8)|(dynamic_FIFO_rst<<5)|(buffer_poniter<<4);	/* Assign DQS0 and DQS1 group delay */
					REG32(DCDQMR) = (DQM1_DLY&0x1f)<<16 | (DQM0_DLY&0x1f)<<24;
					REG32(DWDMOR) = (DQM0_DLY&0x1f)<<16 | (DQM1_DLY&0x1f)<<24;	
					REG32(DDR_DELAY_CTRL_REG0)=0x0fffffff;
					REG32(DDR_DELAY_CTRL_REG1)=0x0a0a0fff;
					REG32(DDR_DELAY_CTRL_REG2)=0xffffffff;
					REG32(DDR_DELAY_CTRL_REG3)=0xffffffff;
				break;
				default:
					REG32(DDCR) = ((HCLK_EN&0x1)<<31) |((DQS0_EN&0x1F)<<24) |((HCLK_EN&0x1)<<23)  | ((DQS1_EN&0x1F)<<16);						//DQS0 HCLK, DQS0 EN, DQS1 HCLK, DQS1 EN
					REG32(DACCR) = ((analog_delay_disable&0x1)<<31) | ((DQS0_GROUP&0x1F)<< 16) |((DQS1_GROUP&0x1F)<< 8)|(dynamic_FIFO_rst<<5)|(buffer_poniter<<4);	/* Assign DQS0 and DQS1 group delay */
					REG32(DCDQMR) = (DQM1_DLY&0x1f)<<16 | (DQM0_DLY&0x1f)<<24;
					REG32(DWDMOR) = (DQM0_DLY&0x1f)<<16 | (DQM1_DLY&0x1f)<<24;	
					REG32(DDR_DELAY_CTRL_REG0)=0x08888888;
					REG32(DDR_DELAY_CTRL_REG1)=0x0a0a0fff;
					REG32(DDR_DELAY_CTRL_REG2)=0x88888888;
					REG32(DDR_DELAY_CTRL_REG3)=0x88888888;
				}
		}else{	//For MCM
			switch(dram_freq_mhz){
				case 800:
				case 750:
				case 700:
				case 650:
					REG32(DDCR) = ((HCLK_EN&0x1)<<31) |((DQS0_EN&0x1F)<<24) |((HCLK_EN&0x1)<<23)  | ((DQS1_EN&0x1F)<<16);						//DQS0 HCLK, DQS0 EN, DQS1 HCLK, DQS1 EN
					REG32(DACCR) = ((analog_delay_disable&0x1)<<31) | ((DQS0_GROUP&0x1F)<< 16) |((DQS1_GROUP&0x1F)<< 8)|(dynamic_FIFO_rst<<5)|(buffer_poniter<<4);	/* Assign DQS0 and DQS1 group delay */
					REG32(DCDQMR) = (DQM1_DLY&0x1f)<<16 | (DQM0_DLY&0x1f)<<24;
					REG32(DWDMOR) = (DQM0_DLY&0x1f)<<16 | (DQM1_DLY&0x1f)<<24;	
					REG32(DDR_DELAY_CTRL_REG0)=0x0fffffff;			//Write: CKE_DLY[n27:24],CS1_DLY[n23:20],CS0_DLY[n19:16],ODT_DLY[n15:12],RAS_DLY[n11:8],CAS_DLY[7:4],WE_DLY[n3:0]
					REG32(DDR_DELAY_CTRL_REG1)=0x12120fff;
					REG32(DDR_DELAY_CTRL_REG2)=0xffffffff;
					REG32(DDR_DELAY_CTRL_REG3)=0xffffffff;
					REG32(DWDQSOR) = 0x1f1f0000;
					break;
				case 600:
					REG32(DDCR) = ((HCLK_EN&0x1)<<31) |((DQS0_EN&0x1F)<<24) |((HCLK_EN&0x1)<<23)  | ((DQS1_EN&0x1F)<<16);						//DQS0 HCLK, DQS0 EN, DQS1 HCLK, DQS1 EN
					REG32(DACCR) = ((analog_delay_disable&0x1)<<31) | ((DQS0_GROUP&0x1F)<< 16) |((DQS1_GROUP&0x1F)<< 8)|(dynamic_FIFO_rst<<5)|(buffer_poniter<<4);	/* Assign DQS0 and DQS1 group delay */
					REG32(DCDQMR) = (DQM1_DLY&0x1f)<<16 | (DQM0_DLY&0x1f)<<24;
					REG32(DWDMOR) = (DQM0_DLY&0x1f)<<16 | (DQM1_DLY&0x1f)<<24;	
					REG32(DDR_DELAY_CTRL_REG0)=0x0fffffff;
					REG32(DDR_DELAY_CTRL_REG1)=0x0c0c0fff;
					REG32(DDR_DELAY_CTRL_REG2)=0xffffffff;
					REG32(DDR_DELAY_CTRL_REG3)=0xffffffff;
					REG32(DWDQSOR) = 0x1f1f0000;
					break;
				case 575:
				case 550:	
				case 525:
				case 500:
					REG32(DDCR) = ((HCLK_EN&0x1)<<31) |((DQS0_EN&0x1F)<<24) |((HCLK_EN&0x1)<<23)  | ((DQS1_EN&0x1F)<<16);						//DQS0 HCLK, DQS0 EN, DQS1 HCLK, DQS1 EN
					REG32(DACCR) = ((analog_delay_disable&0x1)<<31) | ((DQS0_GROUP&0x1F)<< 16) |((DQS1_GROUP&0x1F)<< 8)|(dynamic_FIFO_rst<<5)|(buffer_poniter<<4);	/* Assign DQS0 and DQS1 group delay */
					REG32(DCDQMR) = (DQM1_DLY&0x1f)<<16 | (DQM0_DLY&0x1f)<<24;
					REG32(DWDMOR) = (DQM0_DLY&0x1f)<<16 | (DQM1_DLY&0x1f)<<24;	
					REG32(DDR_DELAY_CTRL_REG0)=0x0fffffff;
					REG32(DDR_DELAY_CTRL_REG1)=0x0c0c0fff;
					REG32(DDR_DELAY_CTRL_REG2)=0xffffffff;
					REG32(DDR_DELAY_CTRL_REG3)=0xffffffff;
					REG32(DWDQSOR) = 0x1f1f0000;
					break;
				case 450:
				case 400:
				case 350:
				case 300:
					REG32(DDCR) = ((HCLK_EN&0x1)<<31) |((DQS0_EN&0x1F)<<24) |((HCLK_EN&0x1)<<23)  | ((DQS1_EN&0x1F)<<16);						//DQS0 HCLK, DQS0 EN, DQS1 HCLK, DQS1 EN
					REG32(DACCR) = ((analog_delay_disable&0x1)<<31) | ((DQS0_GROUP&0x1F)<< 16) |((DQS1_GROUP&0x1F)<< 8)|(dynamic_FIFO_rst<<5)|(buffer_poniter<<4);	/* Assign DQS0 and DQS1 group delay */
					REG32(DCDQMR) = (DQM1_DLY&0x1f)<<16 | (DQM0_DLY&0x1f)<<24;
					REG32(DWDMOR) = (DQM0_DLY&0x1f)<<16 | (DQM1_DLY&0x1f)<<24;	
					REG32(DDR_DELAY_CTRL_REG0)=0x0fffffff;
					REG32(DDR_DELAY_CTRL_REG1)=0x0a0a0fff;
					REG32(DDR_DELAY_CTRL_REG2)=0xffffffff;
					REG32(DDR_DELAY_CTRL_REG3)=0xffffffff;
					REG32(DWDQSOR) = 0x0f0f0000;
					break;
				default:
					REG32(DDCR) = ((HCLK_EN&0x1)<<31) |((DQS0_EN&0x1F)<<24) |((HCLK_EN&0x1)<<23)  | ((DQS1_EN&0x1F)<<16);						//DQS0 HCLK, DQS0 EN, DQS1 HCLK, DQS1 EN
					REG32(DACCR) = ((analog_delay_disable&0x1)<<31) | ((DQS0_GROUP&0x1F)<< 16) |((DQS1_GROUP&0x1F)<< 8)|(dynamic_FIFO_rst<<5)|(buffer_poniter<<4);	/* Assign DQS0 and DQS1 group delay */
					REG32(DCDQMR) = (DQM1_DLY&0x1f)<<16 | (DQM0_DLY&0x1f)<<24;
					REG32(DWDMOR) = (DQM0_DLY&0x1f)<<16 | (DQM1_DLY&0x1f)<<24;	
					REG32(DDR_DELAY_CTRL_REG0)=0x08888888;
					REG32(DDR_DELAY_CTRL_REG1)=0x0a0a0888;
					REG32(DDR_DELAY_CTRL_REG2)=0x88888888;
					REG32(DDR_DELAY_CTRL_REG3)=0x88888888;
			}
		}
	}else{	//For RTL9603C-vd
		if(_is_mcm() !=1){	//discrete
		switch(dram_freq_mhz){
			case 800:
			case 750:
			case 700:
			case 650:
			case 600:
				REG32(DDCR) = ((HCLK_EN&0x1)<<31) |((DQS0_EN&0x1F)<<24) |((HCLK_EN&0x1)<<23)  | ((DQS1_EN&0x1F)<<16);						//DQS0 HCLK, DQS0 EN, DQS1 HCLK, DQS1 EN
				REG32(DACCR) = ((analog_delay_disable&0x1)<<31) | ((DQS0_GROUP&0x1F)<< 16) |((DQS1_GROUP&0x1F)<< 8)|(dynamic_FIFO_rst<<5)|(buffer_poniter<<4);	/* Assign DQS0 and DQS1 group delay */
				REG32(DCDQMR) = (DQM1_DLY&0x1f)<<16 | (DQM0_DLY&0x1f)<<24;
				REG32(DWDMOR) = (DQM0_DLY&0x1f)<<16 | (DQM1_DLY&0x1f)<<24;
				REG32(DDR_DELAY_CTRL_REG0)=0x0FFFFFFF;
				REG32(DDR_DELAY_CTRL_REG1)=0x08080FFF;
				REG32(DDR_DELAY_CTRL_REG2)=0xFFFFFFFF;
				REG32(DDR_DELAY_CTRL_REG3)=0xFFFFFFFF;
				REG32(DWDQSOR) = 0x1f1f0000;
				break;
			case 575:
			case 550:
			case 525:
			case 500:
				REG32(DDCR) = ((HCLK_EN&0x1)<<31) |((DQS0_EN&0x1F)<<24) |((HCLK_EN&0x1)<<23)  | ((DQS1_EN&0x1F)<<16);						//DQS0 HCLK, DQS0 EN, DQS1 HCLK, DQS1 EN
				REG32(DACCR) = ((analog_delay_disable&0x1)<<31) | ((DQS0_GROUP&0x1F)<< 16) |((DQS1_GROUP&0x1F)<< 8)|(dynamic_FIFO_rst<<5)|(buffer_poniter<<4);	/* Assign DQS0 and DQS1 group delay */
				REG32(DCDQMR) = (DQM1_DLY&0x1f)<<16 | (DQM0_DLY&0x1f)<<24;
				REG32(DWDMOR) = (DQM0_DLY&0x1f)<<16 | (DQM1_DLY&0x1f)<<24;
				REG32(DDR_DELAY_CTRL_REG0)=0x0FFFFFFF;
				REG32(DDR_DELAY_CTRL_REG1)=0x04040FFF;
				REG32(DDR_DELAY_CTRL_REG2)=0xFFFFFFFF;
				REG32(DDR_DELAY_CTRL_REG3)=0xFFFFFFFF;
				REG32(DWDQSOR) = 0x1f1f0000;
				break;
			case 450:
			case 400:
				REG32(DDCR) = ((HCLK_EN&0x1)<<31) |((DQS0_EN&0x1F)<<24) |((HCLK_EN&0x1)<<23)  | ((DQS1_EN&0x1F)<<16);						//DQS0 HCLK, DQS0 EN, DQS1 HCLK, DQS1 EN
				REG32(DACCR) = ((analog_delay_disable&0x1)<<31) | ((DQS0_GROUP&0x1F)<< 16) |((DQS1_GROUP&0x1F)<< 8)|(dynamic_FIFO_rst<<5)|(buffer_poniter<<4);	/* Assign DQS0 and DQS1 group delay */
				REG32(DCDQMR) = (DQM1_DLY&0x1f)<<16 | (DQM0_DLY&0x1f)<<24;
				REG32(DWDMOR) = (DQM0_DLY&0x1f)<<16 | (DQM1_DLY&0x1f)<<24;
				REG32(DDR_DELAY_CTRL_REG0)=0x0FFFFFFF;
				REG32(DDR_DELAY_CTRL_REG1)=0x00000FFF;
				REG32(DDR_DELAY_CTRL_REG2)=0xFFFFFFFF;
				REG32(DDR_DELAY_CTRL_REG3)=0xFFFFFFFF;
				REG32(DWDQSOR) = 0x1f1f0000;
				break;
			case 350:
			case 300:
				REG32(DDCR) = ((HCLK_EN&0x1)<<31) |((DQS0_EN&0x1F)<<24) |((HCLK_EN&0x1)<<23)  | ((DQS1_EN&0x1F)<<16);						//DQS0 HCLK, DQS0 EN, DQS1 HCLK, DQS1 EN
				REG32(DACCR) = ((analog_delay_disable&0x1)<<31) | ((DQS0_GROUP&0x1F)<< 16) |((DQS1_GROUP&0x1F)<< 8)|(dynamic_FIFO_rst<<5)|(buffer_poniter<<4);	/* Assign DQS0 and DQS1 group delay */
				REG32(DCDQMR) = (DQM1_DLY&0x1f)<<16 | (DQM0_DLY&0x1f)<<24;
				REG32(DWDMOR) = (DQM0_DLY&0x1f)<<16 | (DQM1_DLY&0x1f)<<24;
				REG32(DDR_DELAY_CTRL_REG0)=0x06666666;
				REG32(DDR_DELAY_CTRL_REG1)=0x00000666;
				REG32(DDR_DELAY_CTRL_REG2)=0x66666666;
				REG32(DDR_DELAY_CTRL_REG3)=0x66666666;
				REG32(DWDQSOR) = 0x0f0f0000;
				break;
			default:
				REG32(DDCR) = ((HCLK_EN&0x1)<<31) |((DQS0_EN&0x1F)<<24) |((HCLK_EN&0x1)<<23)  | ((DQS1_EN&0x1F)<<16);						//DQS0 HCLK, DQS0 EN, DQS1 HCLK, DQS1 EN
				REG32(DACCR) = ((analog_delay_disable&0x1)<<31) | ((DQS0_GROUP&0x1F)<< 16) |((DQS1_GROUP&0x1F)<< 8)|(dynamic_FIFO_rst<<5)|(buffer_poniter<<4);	/* Assign DQS0 and DQS1 group delay */
				REG32(DCDQMR) = (DQM1_DLY&0x1f)<<16 | (DQM0_DLY&0x1f)<<24;
				REG32(DWDMOR) = (DQM0_DLY&0x1f)<<16 | (DQM1_DLY&0x1f)<<24;
				REG32(DDR_DELAY_CTRL_REG0)=0x06666666;
				REG32(DDR_DELAY_CTRL_REG1)=0x00000666;
				REG32(DDR_DELAY_CTRL_REG2)=0x66666666;
				REG32(DDR_DELAY_CTRL_REG3)=0x66666666;
				REG32(DWDQSOR) = 0x0f0f0000;
				}
		}else{	//MCM
		switch(dram_freq_mhz){
			case 800:
			case 750:
			case 700:
			case 650:
			case 600:
				REG32(DDCR) = ((HCLK_EN&0x1)<<31) |((DQS0_EN&0x1F)<<24) |((HCLK_EN&0x1)<<23)  | ((DQS1_EN&0x1F)<<16);						//DQS0 HCLK, DQS0 EN, DQS1 HCLK, DQS1 EN
				REG32(DACCR) = ((analog_delay_disable&0x1)<<31) | ((DQS0_GROUP&0x1F)<< 16) |((DQS1_GROUP&0x1F)<< 8)|(dynamic_FIFO_rst<<5)|(buffer_poniter<<4);	/* Assign DQS0 and DQS1 group delay */
				REG32(DCDQMR) = (DQM1_DLY&0x1f)<<16 | (DQM0_DLY&0x1f)<<24;
				REG32(DWDMOR) = (DQM0_DLY&0x1f)<<16 | (DQM1_DLY&0x1f)<<24;
				REG32(DDR_DELAY_CTRL_REG0)=0x0FFFFFFF;
				REG32(DDR_DELAY_CTRL_REG1)=0x08080FFF;
				REG32(DDR_DELAY_CTRL_REG2)=0xFFFFFFFF;
				REG32(DDR_DELAY_CTRL_REG3)=0xFFFFFFFF;
				REG32(DWDQSOR) = 0x1f1f0000;
				break;
			case 575:
			case 550:
			case 525:
			case 500:
				REG32(DDCR) = ((HCLK_EN&0x1)<<31) |((DQS0_EN&0x1F)<<24) |((HCLK_EN&0x1)<<23)  | ((DQS1_EN&0x1F)<<16);						//DQS0 HCLK, DQS0 EN, DQS1 HCLK, DQS1 EN
				REG32(DACCR) = ((analog_delay_disable&0x1)<<31) | ((DQS0_GROUP&0x1F)<< 16) |((DQS1_GROUP&0x1F)<< 8)|(dynamic_FIFO_rst<<5)|(buffer_poniter<<4);	/* Assign DQS0 and DQS1 group delay */
				REG32(DCDQMR) = (DQM1_DLY&0x1f)<<16 | (DQM0_DLY&0x1f)<<24;
				REG32(DWDMOR) = (DQM0_DLY&0x1f)<<16 | (DQM1_DLY&0x1f)<<24;
				REG32(DDR_DELAY_CTRL_REG0)=0x0FFFFFFF;
				REG32(DDR_DELAY_CTRL_REG1)=0x04040FFF;
				REG32(DDR_DELAY_CTRL_REG2)=0xFFFFFFFF;
				REG32(DDR_DELAY_CTRL_REG3)=0xFFFFFFFF;
				REG32(DWDQSOR) = 0x1f1f0000;
				break;
			case 450:
			case 400:
				REG32(DDCR) = ((HCLK_EN&0x1)<<31) |((DQS0_EN&0x1F)<<24) |((HCLK_EN&0x1)<<23)  | ((DQS1_EN&0x1F)<<16);						//DQS0 HCLK, DQS0 EN, DQS1 HCLK, DQS1 EN
				REG32(DACCR) = ((analog_delay_disable&0x1)<<31) | ((DQS0_GROUP&0x1F)<< 16) |((DQS1_GROUP&0x1F)<< 8)|(dynamic_FIFO_rst<<5)|(buffer_poniter<<4);	/* Assign DQS0 and DQS1 group delay */
				REG32(DCDQMR) = (DQM1_DLY&0x1f)<<16 | (DQM0_DLY&0x1f)<<24;
				REG32(DWDMOR) = (DQM0_DLY&0x1f)<<16 | (DQM1_DLY&0x1f)<<24;
				REG32(DDR_DELAY_CTRL_REG0)=0x0FFFFFFF;
				REG32(DDR_DELAY_CTRL_REG1)=0x00000FFF;
				REG32(DDR_DELAY_CTRL_REG2)=0xFFFFFFFF;
				REG32(DDR_DELAY_CTRL_REG3)=0xFFFFFFFF;
				REG32(DWDQSOR) = 0x1f1f0000;
				break;
			case 350:
			case 300:
				REG32(DDCR) = ((HCLK_EN&0x1)<<31) |((DQS0_EN&0x1F)<<24) |((HCLK_EN&0x1)<<23)  | ((DQS1_EN&0x1F)<<16);						//DQS0 HCLK, DQS0 EN, DQS1 HCLK, DQS1 EN
				REG32(DACCR) = ((analog_delay_disable&0x1)<<31) | ((DQS0_GROUP&0x1F)<< 16) |((DQS1_GROUP&0x1F)<< 8)|(dynamic_FIFO_rst<<5)|(buffer_poniter<<4);	/* Assign DQS0 and DQS1 group delay */
				REG32(DCDQMR) = (DQM1_DLY&0x1f)<<16 | (DQM0_DLY&0x1f)<<24;
				REG32(DWDMOR) = (DQM0_DLY&0x1f)<<16 | (DQM1_DLY&0x1f)<<24;
				REG32(DDR_DELAY_CTRL_REG0)=0x06666666;
				REG32(DDR_DELAY_CTRL_REG1)=0x00000666;
				REG32(DDR_DELAY_CTRL_REG2)=0x66666666;
				REG32(DDR_DELAY_CTRL_REG3)=0x66666666;
				REG32(DWDQSOR) = 0x0f0f0000;
				break;
			default:
				REG32(DDCR) = ((HCLK_EN&0x1)<<31) |((DQS0_EN&0x1F)<<24) |((HCLK_EN&0x1)<<23)  | ((DQS1_EN&0x1F)<<16);						//DQS0 HCLK, DQS0 EN, DQS1 HCLK, DQS1 EN
				REG32(DACCR) = ((analog_delay_disable&0x1)<<31) | ((DQS0_GROUP&0x1F)<< 16) |((DQS1_GROUP&0x1F)<< 8)|(dynamic_FIFO_rst<<5)|(buffer_poniter<<4);	/* Assign DQS0 and DQS1 group delay */
				REG32(DCDQMR) = (DQM1_DLY&0x1f)<<16 | (DQM0_DLY&0x1f)<<24;
				REG32(DWDMOR) = (DQM0_DLY&0x1f)<<16 | (DQM1_DLY&0x1f)<<24;
				REG32(DDR_DELAY_CTRL_REG0)=0x06666666;
				REG32(DDR_DELAY_CTRL_REG1)=0x00000666;
				REG32(DDR_DELAY_CTRL_REG2)=0x66666666;
				REG32(DDR_DELAY_CTRL_REG3)=0x66666666;
				REG32(DWDQSOR) = 0x0f0f0000;
			}
		}
	}
	// DRAM patch from efuse
	if (EFPH_patch_num!=0 && EFPH_DQS_delay_en==1){	
		REG32(DDR_DELAY_CTRL_REG1)=(REG32(DDR_DELAY_CTRL_REG1) & 0x0000FFFF)|(EFPH_DQS_delay*2<<16)|(EFPH_DQS_delay*2<<24);
	}
	if (EFPH_patch_num!=0 && EFPH_Addr_delay_en==1){	
		REG32(DDR_DELAY_CTRL_REG0)=EFPH_Addr_delay|EFPH_Addr_delay<<4|EFPH_Addr_delay<<8|EFPH_Addr_delay<<12|EFPH_Addr_delay<<16|EFPH_Addr_delay<<20|EFPH_Addr_delay<<24;
		REG32(DDR_DELAY_CTRL_REG1)=(REG32(DDR_DELAY_CTRL_REG1) & 0xFFFFF000)|EFPH_Addr_delay|EFPH_Addr_delay<<4|EFPH_Addr_delay<<8;
		REG32(DDR_DELAY_CTRL_REG2)=EFPH_Addr_delay|EFPH_Addr_delay<<4|EFPH_Addr_delay<<8|EFPH_Addr_delay<<12|EFPH_Addr_delay<<16|EFPH_Addr_delay<<20|EFPH_Addr_delay<<24|EFPH_Addr_delay<<28;
		REG32(DDR_DELAY_CTRL_REG3)=EFPH_Addr_delay|EFPH_Addr_delay<<4|EFPH_Addr_delay<<8|EFPH_Addr_delay<<12|EFPH_Addr_delay<<16|EFPH_Addr_delay<<20|EFPH_Addr_delay<<24|EFPH_Addr_delay<<28;
	}	

	_memctl_update_phy_param();
	return;
}

#ifdef CONFIG_DRAM_PUPD
void memctlc_config_pupd(void)
{
	volatile unsigned int *dtr0, *ddcr, *ddpdr0, *ddpdr1;
	unsigned char tCAS_PHY, dqs0_half_clk, dqs1_half_clk, DQS0_PUPD_DET_DELAY, DQS1_PUPD_DET_DELAY;
	ddcr = (volatile unsigned int *)DDCR;
	dtr0 = (volatile unsigned int *)DTR0;
	ddpdr0 = (volatile unsigned int *)DDPDR0;
	ddpdr1 = (volatile unsigned int *)DDPDR1;

	DQS0_PUPD_DET_DELAY = DQS1_PUPD_DET_DELAY = 15; //default value
	tCAS_PHY = *dtr0 & 0x0f;
	dqs0_half_clk = (*ddcr >> 31)&0x1;
	dqs1_half_clk = (*ddcr >> 23)&0x1;
	*ddpdr0 =  (1 << 31 ) | (tCAS_PHY << 25) | (DQS0_PUPD_DET_DELAY << 20) | (dqs0_half_clk << 19) | (dqs0_half_clk << 18) | (tCAS_PHY << 8);
	*ddpdr1 =  (1 << 31 ) | (tCAS_PHY << 25) | (DQS1_PUPD_DET_DELAY << 20) | (dqs1_half_clk << 19) | (dqs1_half_clk << 18) | (tCAS_PHY << 8);
	_memctl_update_phy_param();
	puts("AK: ENABLE: DRAM PUPD detection\n\r");
}
#endif

/* Function Name:
 * 	memctlc_init_dram
 * Descripton:
 *
 * Input:
 *	None
 * Output:
 * 	None
 * Return:
 *	.
 */


#ifndef USE_OTTO_CG_FUNCTION
    #define MEM_PLL_INFO_SECTION
    typedef struct{
        u16_t mhz;
        u32_t pll0;
        u32_t pll1;
        u32_t pll2;
        u32_t pll3;
    }mem_pll_info_t;
#endif

// For RTL9603C-vd
MEM_PLL_INFO_SECTION mem_pll_info_t ddr2_pll[] = {
    {.mhz=525, .pll0=0x0000307f, .pll1=0xc0000210, .pll2=0x26622760, .pll3=0x270f0030},
    {.mhz=500, .pll0=0x0000307f, .pll1=0xc0000210, .pll2=0x26622760, .pll3=0x250f0030},
    {.mhz=450, .pll0=0x0000307f, .pll1=0xc0000210, .pll2=0x26622760, .pll3=0x210f0030},
    {.mhz=400, .pll0=0x0000307f, .pll1=0xc0000210, .pll2=0x26622760, .pll3=0x1d0f0030},
    {.mhz=350, .pll0=0x0000307f, .pll1=0xc0000210, .pll2=0x26622760, .pll3=0x190f0030},
    {.mhz=300, .pll0=0x0000307f, .pll1=0xc0000210, .pll2=0x2d622720, .pll3=0x150f0030},
    {.mhz=200, .pll0=0x0000007f, .pll1=0x80000210, .pll2=0x2c222520, .pll3=0x0d0f0030},
    {.mhz=  0, .pll0=0         , .pll1=0         , .pll2=0         , .pll3=0         },
};

MEM_PLL_INFO_SECTION mem_pll_info_t ddr3_pll[] = {
	{.mhz=800, .pll0=0x0000307f, .pll1=0xc0000231, .pll2=0x2ea23980, .pll3=0x3d0f0031},
	{.mhz=750, .pll0=0x0000307f, .pll1=0xc0000210, .pll2=0x2ea23980, .pll3=0x390f0030},
	{.mhz=700, .pll0=0x0000307f, .pll1=0xc0000210, .pll2=0x2ea23980, .pll3=0x350f0030},
    {.mhz=650, .pll0=0x0000307f, .pll1=0xc0000210, .pll2=0x2ea23980, .pll3=0x310f0030},
	{.mhz=600, .pll0=0x0000307f, .pll1=0xc0000210, .pll2=0x2ea23980, .pll3=0x2d0f0030},
    {.mhz=550, .pll0=0x0000307f, .pll1=0xc0000210, .pll2=0x26622760, .pll3=0x290f0030},
    {.mhz=525, .pll0=0x0000307f, .pll1=0xc0000210, .pll2=0x26622760, .pll3=0x270f0030},
    {.mhz=500, .pll0=0x0000307f, .pll1=0xc0000210, .pll2=0x26622760, .pll3=0x250f0030},
    {.mhz=450, .pll0=0x0000307f, .pll1=0xc0000210, .pll2=0x26622760, .pll3=0x210f0030},
    {.mhz=400, .pll0=0x0000307f, .pll1=0xc0000210, .pll2=0x26622760, .pll3=0x1d0f0030},
    {.mhz=350, .pll0=0x0000307f, .pll1=0xc0000210, .pll2=0x26622760, .pll3=0x190f0030},
    {.mhz=300, .pll0=0x0000307f, .pll1=0xc0000210, .pll2=0x2d622720, .pll3=0x150f0030},
    {.mhz=200, .pll0=0x0000307f, .pll1=0x80000210, .pll2=0x2c222520, .pll3=0x0d0f0030},
    {.mhz=  0, .pll0=0         , .pll1=0         , .pll2=0         , .pll3=0         },
};

MEM_PLL_INFO_SECTION mem_pll_info_t ddr2_mcm_pll[] = {
    {.mhz=525, .pll0=0x0000307f, .pll1=0xc0000210, .pll2=0x26622760, .pll3=0x270f0030},
    {.mhz=500, .pll0=0x0000307f, .pll1=0xc0000210, .pll2=0x26622760, .pll3=0x250f0030},
    {.mhz=450, .pll0=0x0000307f, .pll1=0xc0000210, .pll2=0x26622760, .pll3=0x210f0030},
    {.mhz=400, .pll0=0x0000307f, .pll1=0xc0000210, .pll2=0x26622760, .pll3=0x1d0f0030},
    {.mhz=350, .pll0=0x0000307f, .pll1=0xc0000210, .pll2=0x26622760, .pll3=0x190f0030},
    {.mhz=300, .pll0=0x0000307f, .pll1=0xc0000210, .pll2=0x2d622720, .pll3=0x150f0030},
    {.mhz=200, .pll0=0x0000007f, .pll1=0x80000210, .pll2=0x2c222520, .pll3=0x0d0f0030},
    {.mhz=  0, .pll0=0         , .pll1=0         , .pll2=0         , .pll3=0         },
};

MEM_PLL_INFO_SECTION mem_pll_info_t ddr3_mcm_pll[] = {
	{.mhz=800, .pll0=0x0000307f, .pll1=0xc0000231, .pll2=0x2ea23980, .pll3=0x3d0f0031},
	{.mhz=750, .pll0=0x0000307f, .pll1=0xc0000210, .pll2=0x2ea23980, .pll3=0x390f0030},
	{.mhz=700, .pll0=0x0000307f, .pll1=0xc0000210, .pll2=0x2ea23980, .pll3=0x350f0030},
    {.mhz=650, .pll0=0x0000307f, .pll1=0xc0000210, .pll2=0x2ea23980, .pll3=0x310f0030},
	{.mhz=600, .pll0=0x0000307f, .pll1=0xc0000210, .pll2=0x2ea23980, .pll3=0x2d0f0030},
    {.mhz=550, .pll0=0x0000307f, .pll1=0xc0000210, .pll2=0x26622760, .pll3=0x290f0030},
    {.mhz=525, .pll0=0x0000307f, .pll1=0xc0000210, .pll2=0x26622760, .pll3=0x270f0030},
    {.mhz=500, .pll0=0x0000307f, .pll1=0xc0000210, .pll2=0x26622760, .pll3=0x250f0030},
    {.mhz=450, .pll0=0x0000307f, .pll1=0xc0000210, .pll2=0x26622760, .pll3=0x210f0030},
    {.mhz=400, .pll0=0x0000307f, .pll1=0xc0000210, .pll2=0x26622760, .pll3=0x1d0f0030},
    {.mhz=350, .pll0=0x0000307f, .pll1=0xc0000210, .pll2=0x26622760, .pll3=0x190f0030},
    {.mhz=300, .pll0=0x0000307f, .pll1=0xc0000210, .pll2=0x2d622720, .pll3=0x150f0030},
    {.mhz=200, .pll0=0x0000007f, .pll1=0x80000210, .pll2=0x2c222520, .pll3=0x0d0f0030},
    {.mhz=  0, .pll0=0         , .pll1=0         , .pll2=0         , .pll3=0         },
};

// For Apollo pro
MEM_PLL_INFO_SECTION mem_pll_info_t apro_ddr2_pll[] = {
    {.mhz=525, .pll0=0x0010407f, .pll1=0xc0000210, .pll2=0x26622760, .pll3=0x270f0030},
    {.mhz=500, .pll0=0x0010407f, .pll1=0xc0000210, .pll2=0x26622760, .pll3=0x250f0030},
    {.mhz=450, .pll0=0x0000207f, .pll1=0xc0000210, .pll2=0x26622760, .pll3=0x210f0030},
    {.mhz=400, .pll0=0x0000207f, .pll1=0xc0000210, .pll2=0x26622760, .pll3=0x1d0f0030},
    {.mhz=350, .pll0=0x0000207f, .pll1=0xc0000210, .pll2=0x26622760, .pll3=0x190f0030},
    {.mhz=300, .pll0=0x0000207f, .pll1=0xc0000210, .pll2=0x2d622720, .pll3=0x150f0030},
    {.mhz=200, .pll0=0x0000007f, .pll1=0x80000210, .pll2=0x2c222520, .pll3=0x0d0f0030},
    {.mhz=  0, .pll0=0         , .pll1=0         , .pll2=0         , .pll3=0         },
};

MEM_PLL_INFO_SECTION mem_pll_info_t apro_ddr3_pll[] = {
	{.mhz=800, .pll0=0x0011007f, .pll1=0xc0000231, .pll2=0x2ea23980, .pll3=0x3d0f0031},
	{.mhz=750, .pll0=0x0010a07f, .pll1=0xc0000210, .pll2=0x2ea23980, .pll3=0x390f0030},
	{.mhz=700, .pll0=0x0010a07f, .pll1=0xc0000210, .pll2=0x2ea23980, .pll3=0x350f0030},
    {.mhz=650, .pll0=0x0010407f, .pll1=0xc0000210, .pll2=0x2ea23980, .pll3=0x310f0030},
	{.mhz=600, .pll0=0x0008407f, .pll1=0xc0000210, .pll2=0x2ea23980, .pll3=0x2d0f0030},
    {.mhz=550, .pll0=0x0008407f, .pll1=0xc0000210, .pll2=0x26622760, .pll3=0x290f0030},
    {.mhz=525, .pll0=0x0008407f, .pll1=0xc0000210, .pll2=0x26622760, .pll3=0x270f0030},
    {.mhz=500, .pll0=0x0008407f, .pll1=0xc0000210, .pll2=0x26622760, .pll3=0x250f0030},
    {.mhz=450, .pll0=0x0000207f, .pll1=0xc0000210, .pll2=0x26622760, .pll3=0x210f0030},
    {.mhz=400, .pll0=0x0000207f, .pll1=0xc0000210, .pll2=0x26622760, .pll3=0x1d0f0030},
    {.mhz=350, .pll0=0x0000207f, .pll1=0xc0000210, .pll2=0x26622760, .pll3=0x190f0030},
    {.mhz=300, .pll0=0x0000207f, .pll1=0xc0000210, .pll2=0x2d622720, .pll3=0x150f0030},
    {.mhz=200, .pll0=0x0000007f, .pll1=0x80000210, .pll2=0x2c222520, .pll3=0x0d0f0030},
    {.mhz=  0, .pll0=0         , .pll1=0         , .pll2=0         , .pll3=0         },
};

MEM_PLL_INFO_SECTION mem_pll_info_t apro_ddr2_mcm_pll[] = {
    {.mhz=525, .pll0=0x0000407f, .pll1=0xc0000210, .pll2=0x26622760, .pll3=0x270f0030},
    {.mhz=500, .pll0=0x0000407f, .pll1=0xc0000210, .pll2=0x26622760, .pll3=0x250f0030},
    {.mhz=450, .pll0=0x0000207f, .pll1=0xc0000210, .pll2=0x26622760, .pll3=0x210f0030},
    {.mhz=400, .pll0=0x0000207f, .pll1=0xc0000210, .pll2=0x26622760, .pll3=0x1d0f0030},
    {.mhz=350, .pll0=0x0000207f, .pll1=0xc0000210, .pll2=0x26622760, .pll3=0x190f0030},
    {.mhz=300, .pll0=0x0000207f, .pll1=0xc0000210, .pll2=0x2d622720, .pll3=0x150f0030},
    {.mhz=200, .pll0=0x0000007f, .pll1=0x80000210, .pll2=0x2c222520, .pll3=0x0d0f0030},
    {.mhz=  0, .pll0=0         , .pll1=0         , .pll2=0         , .pll3=0         },
};

MEM_PLL_INFO_SECTION mem_pll_info_t apro_ddr3_mcm_pll[] = {
	{.mhz=800, .pll0=0x0011007f, .pll1=0xc0000231, .pll2=0x2ea23980, .pll3=0x3d0f0031},
	{.mhz=750, .pll0=0x0010a07f, .pll1=0xc0000210, .pll2=0x2ea23980, .pll3=0x390f0030},
	{.mhz=700, .pll0=0x0010a07f, .pll1=0xc0000210, .pll2=0x2ea23980, .pll3=0x350f0030},
    {.mhz=650, .pll0=0x0010a07f, .pll1=0xc0000210, .pll2=0x2ea23980, .pll3=0x310f0030},
	{.mhz=600, .pll0=0x0000407f, .pll1=0xc0000210, .pll2=0x2ea23980, .pll3=0x2d0f0030},
    {.mhz=550, .pll0=0x0000407f, .pll1=0xc0000210, .pll2=0x26622760, .pll3=0x290f0030},
    {.mhz=525, .pll0=0x0000407f, .pll1=0xc0000210, .pll2=0x26622760, .pll3=0x270f0030},
    {.mhz=500, .pll0=0x0000407f, .pll1=0xc0000210, .pll2=0x26622760, .pll3=0x250f0030},
    {.mhz=450, .pll0=0x0000207f, .pll1=0xc0000210, .pll2=0x26622760, .pll3=0x210f0030},
    {.mhz=400, .pll0=0x0000207f, .pll1=0xc0000210, .pll2=0x26622760, .pll3=0x1d0f0030},
    {.mhz=350, .pll0=0x0000207f, .pll1=0xc0000210, .pll2=0x26622760, .pll3=0x190f0030},
    {.mhz=300, .pll0=0x0000207f, .pll1=0xc0000210, .pll2=0x2d622720, .pll3=0x150f0030},
    {.mhz=200, .pll0=0x0000007f, .pll1=0x80000210, .pll2=0x2c222520, .pll3=0x0d0f0030},
    {.mhz=  0, .pll0=0         , .pll1=0         , .pll2=0         , .pll3=0         },
};


u32_t find_mem_pll_setting(u32_t target_freq, mem_pll_info_t *ptr)
{
    u32_t ret_idx=0, cur_freq, final_freq=0;
    s32_t i=-1;
    while(1){
        cur_freq = ptr[++i].mhz;
        if(0 == cur_freq) break;
        if((target_freq >= cur_freq) && (cur_freq > final_freq)){
            final_freq = cur_freq;
            ret_idx = i;
        }
    }
    if(0 == final_freq){
        ret_idx = i - 1;
    }
    return ret_idx;
}

#ifndef USE_OTTO_CG_FUNCTION
void DRAM_frequency_setting(void){
	unsigned delay_cnt,polling_limit;
	//REG32(MEMPLL159_128) &= ~MEMPLL_CLK_EN;
	//delay_cnt = 0x100;
	while(delay_cnt--);
	REG32(MEMPLL159_128) &= ~MEMPLL_CLK_OE;
	delay_cnt = 0x100;
	while(delay_cnt--);
	//REG32(SYSREG_PLL_CTRL_REG) &= ~(1<<31);

    unsigned int index=0;
    if(memctlc_DDR_Type()==IS_DDR2_SDRAM){
#ifdef	CONFIG_RAM_650
        index = find_pll_setting(650, ddr2_pll);
#elif	CONFIG_RAM_600
        index = find_pll_setting(600, ddr2_pll);
#elif	CONFIG_RAM_550
        index = find_pll_setting(550, ddr2_pll);
#elif	CONFIG_RAM_525 || CONFIG_RAM_MP
        index = find_pll_setting(525, ddr2_pll);
#elif	CONFIG_RAM_500
        index = find_pll_setting(500, ddr2_pll);
#elif	CONFIG_RAM_450
        index = find_pll_setting(450, ddr2_pll);
#elif	CONFIG_RAM_400
        index = find_pll_setting(400, ddr2_pll);
#elif	CONFIG_RAM_350
        index = find_pll_setting(350, ddr2_pll);
#elif	CONFIG_RAM_300
        index = find_pll_setting(300, ddr2_pll);
#else	//CONFIG_RAM_200
        index = find_pll_setting(0, ddr2_pll);
#endif
    	REG32(MEMPLL31_0)	= ddr2_pll[index].pll0;
    	REG32(MEMPLL63_32)	= ddr2_pll[index].pll1;
    	REG32(MEMPLL95_64)	= ddr2_pll[index].pll2;
    	REG32(MEMPLL127_96) = ddr2_pll[index].pll3;
	}else{	//DDR3
#ifdef	CONFIG_RAM_800
        index = find_pll_setting(800, ddr3_pll);
#elif	CONFIG_RAM_750
        index = find_pll_setting(750, ddr3_pll);
#elif	CONFIG_RAM_700
        index = find_pll_setting(700, ddr3_pll);
#elif	CONFIG_RAM_650
        index = find_pll_setting(650, ddr3_pll);
#elif	CONFIG_RAM_600
        index = find_pll_setting(600, ddr3_pll);
#elif	CONFIG_RAM_550
        index = find_pll_setting(550, ddr3_pll);
#elif	CONFIG_RAM_525 || CONFIG_RAM_MP
        index = find_pll_setting(525, ddr3_pll);
#elif	CONFIG_RAM_500
        index = find_pll_setting(500, ddr3_pll);
#elif	CONFIG_RAM_450
        index = find_pll_setting(450, ddr3_pll);
#elif	CONFIG_RAM_400
        index = find_pll_setting(400, ddr3_pll);
#elif	CONFIG_RAM_350
        index = find_pll_setting(350, ddr3_pll);
#elif	CONFIG_RAM_300
        index = find_pll_setting(300, ddr3_pll);
#else	//CONFIG_RAM_200
        index = find_pll_setting(0, ddr3_pll);
#endif
        REG32(MEMPLL31_0)   = ddr3_pll[index].pll0;
        REG32(MEMPLL63_32)  = ddr3_pll[index].pll1;
        REG32(MEMPLL95_64)  = ddr3_pll[index].pll2;
        REG32(MEMPLL127_96) = ddr3_pll[index].pll3;
    }

    REG32(MEMPLL159_128)= 0x00000000;
    REG32(MEMPLL191_160)= 0x00000000;
    REG32(MEMPLL223_192)= 0x00000000;

#if 0
	if((REG32(MEMPLL63_32)>>5) & 0x10) { REG32(MEMPLL127_96) |= 1<<5;}
	if(REG32(MEMPLL63_32) & 0x10) { REG32(MEMPLL127_96) |= 1<<4;}
	if((REG32(MEMPLL31_0)>>27) & 0x10) { REG32(MEMPLL127_96) |= 1<<3;}
	if((REG32(MEMPLL31_0)>>22) & 0x10) { REG32(MEMPLL127_96) |= 1<<2;}
	if((REG32(MEMPLL31_0)>>17) & 0x10) { REG32(MEMPLL127_96) |= 1<<1;}
	if((REG32(MEMPLL31_0)>>12) & 0x10) { REG32(MEMPLL127_96) |= 1;}
#endif
	// Delay...
	delay_cnt = 0x800;						//210ns
	while(delay_cnt--);

	// Enable PLL CLK
	REG32(MEMPLL159_128) |= (MEMPLL_CLK_OE | MEMPLL_CLK_EN);

	// trigger the DRAM initialization procedure
	REG32(MCR) = REG32(MCR) | (1<<14);
	puts("TRIG: DRAM initialization procedure, wait done bit...");
	polling_limit = 0x1000;
	while((REG32(MCR)>>14) & 0x1){
		polling_limit--;
		puts(".");
		if(polling_limit==0){
			puts("Not ready!\n\r");
			break;
		}
	}
	puts("Done!\n\r");

#if 1 //show MEMPLL
	puts("PLL: MEMPLL31 = 0x");puthex(REG32(MEMPLL31_0));
	puts("\nPLL: MEMPLL63 = 0x");puthex(REG32(MEMPLL63_32));
	puts("\nPLL: MEMPLL95 = 0x");puthex(REG32(MEMPLL95_64));
	puts("\nPLL: MEMPLL127 = 0x");puthex(REG32(MEMPLL127_96));
	puts("\nPLL: MEMPLL159 = 0x");puthex(REG32(MEMPLL159_128));
	puts("\nPLL: MEMPLL191 = 0x");puthex(REG32(MEMPLL191_160));
	puts("\nPLL: MEMPLL223 = 0x");puthex(REG32(MEMPLL223_192));puts("\n");
#endif
}
#endif
void CMU_setup(unsigned short DRAM_freq){
	unsigned int delay_cnt=0x8FFFF;
	unsigned short OCP0_freq;
	unsigned short OCP1_freq;
	unsigned short LX_freq;

	//check reg_en_DIV2_cpu0 for different ocp0 freq. calculation
	if (REG32(SYSREG_PLL_CTL32_63_REG) & (1 <<18)) {
	OCP0_freq = ((((REG32(SYSREG_SYSCLK_CONTROL_REG) >> OCP0_PLL_DIV_S) & 0x3f) + 2) * 25);
	}
	else {
		OCP0_freq = ((((REG32(SYSREG_SYSCLK_CONTROL_REG) >> OCP0_PLL_DIV_S) & 0x3f) + 2) * 50);
	}
	OCP1_freq = ((((REG32(SYSREG_SYSCLK_CONTROL_REG) >> OCP1_PLL_DIV_S) & 0x3f) + 2) * 25);

	/*There is a wrapper between OC0 and memctrl, so the Freq. known by memctrl is actually an half of the original one. */
	OCP0_freq = OCP0_freq / 2;

    LX_freq = 1000 / ((REG32(SYSREG_LX_PLL_SEL_REG) & SYSREG_LX_PLL_CTRL_LXPLL_FD_MASK) + 2);

	/*setup OCP/DRAM slow bit*/
	if (DRAM_freq >= OCP0_freq) {
		REG32(SYSREG_CMUCR_OC0_REG) |= SE_DRAM;
	}
	else {
		REG32(SYSREG_CMUCR_OC0_REG) &= ~(SE_DRAM);
	}

	if (DRAM_freq >= OCP1_freq) {
		REG32(SYSREG_CMUCR_OC1_REG) |= SE_DRAM;
	}
	else {
		REG32(SYSREG_CMUCR_OC1_REG) &= ~(SE_DRAM);
	}

	/*setup LX/DRAM slow bit*/
	if (DRAM_freq >= LX_freq) {
		REG32(SYSREG_LBSBCR) |= (LX0_FRQ_SLOWER | LX1_FRQ_SLOWER | LX2_FRQ_SLOWER | LX3_FRQ_SLOWER);
	}

	puts("System FREQ. info:");
	puts("OCP0: 0x"); puthex(OCP0_freq*2); puts(" MHz, ");
	puts("OCP1: 0x"); puthex(OCP1_freq); puts(" MHz, ");
	puts("DRAM: 0x"); puthex(DRAM_freq); puts(" MHz, ");
	puts("LX: 0x"); puthex(LX_freq); puts(" MHz\n\r");

	//show CMU slow bits setup
	puts("Slow bits configuration:\n\r");
	puts("0xb8000380: 0x"); puthex(REG32(SYSREG_CMUGCR_OC0_REG)); puts("\n\r");
	puts("0xb8000390: 0x"); puthex(REG32(SYSREG_CMUGCR_OC0_REG)); puts("\n\r");
	puts("0xb8000388: 0x"); puthex(REG32(SYSREG_CMUCR_OC0_REG)); puts("\n\r");
	puts("0xb8000398: 0x"); puthex(REG32(SYSREG_CMUCR_OC1_REG)); puts("\n\r");
	puts("0xb80003a0: 0x"); puthex(REG32(SYSREG_LBSBCR)); puts("\n\r");
	puts("SCATS settings:\n\r");
	puts("0xb80040f8: 0x"); puthex(REG32(0xb80040f8)); puts("\n\r");
#if 0
	puts("MEMCR:\n\r");
	puts("0xb8001000: 0x"); puthex(REG32(0xb8001000)); puts("\n\r");
	puts("0xb8001004: 0x"); puthex(REG32(0xb8001004)); puts("\n\r");
	puts("0xb8001008: 0x"); puthex(REG32(0xb8001008)); puts("\n\r");
	puts("0xb800100c: 0x"); puthex(REG32(0xb800100c)); puts("\n\r");
	puts("0xb8001010: 0x"); puthex(REG32(0xb8001010)); puts("\n\r");
	puts("MEMCR CMU debug:\n\r");
	REG32(0xb8000614) = 0x00110000;
	puts("(OPC0) write 0xb8000614 = 0x00110000\n\r");
	puts("0xb800061c: 0x"); puthex(REG32(0xb800061c)); puts("\n\r");
	REG32(0xb8000614) = 0x00110100;
	puts("(OCP1) write 0xb8000614 = 0x00110100\n\r");
	puts("0xb800061c: 0x"); puthex(REG32(0xb800061c)); puts("\n\r");
	puts("----------------------------------------------------------------\n\r");
#endif

	while(delay_cnt--);
	return;
}

#define CONFIG_DRAM_SCAN

#ifdef CONFIG_DRAM_SCAN
uint32 random_pat[] __attribute__ ((section(".text")))= {
			0x5c10f3b6, 0x32ae1be4, 0xbe083cd3, 0x48ec00ae,
			0xe70cb6c3, 0x604f89a3, 0xa69f3ab9, 0x3282c99f,
			0xd9ea62fc, 0xdc5696f6, 0x9e6b19cf, 0x198fccb0,
			0xaa3612f1, 0x42253776, 0xf20dc024, 0xbbf6a66b,
			0xf7bcb28a, 0xbb2f3791, 0x6674f1d2, 0x85f15e11,
			0x610b7079, 0x706bf655, 0x33be799d, 0x6eda5ac4,
			0xaa134dd2, 0x613a9ea7, 0xdf2a8b7e, 0x84bbb681,
			0xd4c49fb7, 0xce807f7e, 0xc4a5a72b, 0xee98a9ba,
		   };

#define _memctl_DCache_flush_invalidate _1004K_L1_DCache_flush

void delay_time(volatile uint32 delay_cnt){

	while(delay_cnt--);

}
uint32 Simple_DRAM_test(uint32 start_addr, uint32 dword_len){

	volatile uint32 *p_start, data_tmp;;
	uint32 b_len, pat_data;

	// Do cache write, so clear Cache
	_memctl_DCache_flush_invalidate();

	// Write data
	p_start = (volatile uint32 *)start_addr;
	for(b_len = 0; b_len < dword_len; b_len += sizeof(uint32)){
		*p_start = random_pat[(b_len / sizeof(uint32)) % (sizeof(random_pat)/4)];
		p_start++;
	}

	// Do cache read, so clear Cache
	_memctl_DCache_flush_invalidate();

    // Read data and verify
    p_start = (volatile uint32 *)start_addr;
    for(b_len = 0; b_len < dword_len; b_len += sizeof(uint32)){
		data_tmp = *p_start;
		pat_data = random_pat[(b_len / sizeof(uint32)) % (sizeof(random_pat)/4)];
		if(pat_data!=data_tmp)
			return 1;	//ERROR
		p_start++;
	}
	return 0;		//PASS
}

void DDRPLL_CLK_Disable(void){

	// Disable PLL CLK
	REG32(MEMPLL159_128) &= ~MEMPLL_CLK_OE;

	delay_time(0x10000);

}
void DDRPLL_CLK_Enable(void){

	delay_time(0x10000);

	// Enable PLL CLK
	REG32(MEMPLL159_128) |= (MEMPLL_CLK_OE | MEMPLL_CLK_EN);

	while((REG32(MCR)>>14) & 0x1);
}

void DRAM_param_restore(int store_recover){

	static uint32 PLL0_tmp,PLL1_tmp,PLL3_tmp,delay0_tmp,delay1_tmp,delay2_tmp,delay3_tmp,DWDQSOR_tmp;

	if(store_recover==1){
		PLL0_tmp = REG32(MEMPLL31_0);
		PLL1_tmp = REG32(MEMPLL63_32);
		PLL3_tmp = REG32(MEMPLL127_96);
		delay0_tmp = REG32(DDR_DELAY_CTRL_REG0);
		delay1_tmp = REG32(DDR_DELAY_CTRL_REG1);
		delay2_tmp = REG32(DDR_DELAY_CTRL_REG2);
		delay3_tmp = REG32(DDR_DELAY_CTRL_REG3);
		DWDQSOR_tmp = REG32(DWDQSOR);
		puts("Store DRAM paramter: PLL0=0x"); puthex(PLL0_tmp); puts("  PLL1=0x"); puthex(PLL1_tmp); ;puts("  PLL3=0x"); puthex(PLL3_tmp); puts("\n");
	}else {
		DDRPLL_CLK_Disable();
		REG32(MEMPLL31_0) = PLL0_tmp;
		REG32(MEMPLL63_32) = PLL1_tmp;
		REG32(MEMPLL127_96) = PLL3_tmp;
		DDRPLL_CLK_Enable();

		memctlc_dram_phy_reset();
//		memctlc_ddr3_dll_reset();

		REG32(DDR_DELAY_CTRL_REG0) = delay0_tmp;
		REG32(DDR_DELAY_CTRL_REG1) = delay1_tmp;
		REG32(DDR_DELAY_CTRL_REG2) = delay2_tmp;
		REG32(DDR_DELAY_CTRL_REG3) = delay3_tmp;
		REG32(DWDQSOR) = DWDQSOR_tmp;
		puts("Recovery DRAM paramter: PLL0=0x"); puthex(REG32(MEMPLL31_0)); puts("  PLL1=0x"); puthex(REG32(MEMPLL63_32)); ;puts("  PLL3=0x"); puthex(REG32(MEMPLL127_96)); puts("\n");
	}
	return;

}

void AC_Scan(void){

	unsigned char i;
	uint32 *dcsdcr0,*dcsdcr1,*dcsdcr2,*dcsdcr3;
	uint32 dcsdcr0_tmp,dcsdcr1_tmp,dcsdcr2_tmp,dcsdcr3_tmp;

	dcsdcr0 = (uint32 *)DDR_DELAY_CTRL_REG0;
	dcsdcr1 = (uint32 *)DDR_DELAY_CTRL_REG1;
	dcsdcr2 = (uint32 *)DDR_DELAY_CTRL_REG2;
	dcsdcr3 = (uint32 *)DDR_DELAY_CTRL_REG3;

	dcsdcr0_tmp = *dcsdcr0;
	dcsdcr1_tmp = *dcsdcr1;
	dcsdcr2_tmp = *dcsdcr2;
	dcsdcr3_tmp = *dcsdcr3;

	puts("===== AC delay tap MAP =====\n");
	puts("0 1 2 3 4 5 6 7 8 9 a b c d e f\n");

	for(i=0;i<16;i++){
		*dcsdcr0= (i<<20) | (i<<16) | (i<<8) | (i<<4) | (i<<0);
		*dcsdcr1= (dcsdcr1_tmp & 0xFFFF0000) | (i<<8) | (i<<4) | (i<<0);
		*dcsdcr2= (i<<28) | (i<<24) | (i<<20) | (i<<16) | (i<<12) | (i<<8) | (i<<4) | (i<<0);
		*dcsdcr3= (i<<28) | (i<<24) | (i<<20) | (i<<16) | (i<<12) | (i<<8) | (i<<4) | (i<<0);
		_memctl_update_phy_param();

		// DRAM TEST
		if ((Simple_DRAM_test(0x80000000,0x4000))==0)
			puts("0 "); 	//PASS
		else
			puts("1 ");		//FAIL
	}
	puts("\n===== AC delay tap MAP =====\n");

	//restore delay tap
	*dcsdcr0 = dcsdcr0_tmp;
	*dcsdcr1 = dcsdcr1_tmp;
	*dcsdcr2 = dcsdcr2_tmp;
	*dcsdcr3 = dcsdcr3_tmp;

}




void PI_Setting(unsigned char PI, uint32 PI_val){


	unsigned char j;
	uint32 *PLL0, *PLL1;
	uint32 bit_shift, bit_mask, PLL_target;

	PLL0 = (uint32 *)MEMPLL31_0;
	PLL1 = (uint32 *)MEMPLL63_32;
	bit_mask = 0x1f;

	if(PI==0)
		bit_shift = 12;
	else if(PI==1)
		bit_shift = 17;
	else if(PI==2)
		bit_shift = 22;
	else if(PI==3)
		bit_shift = 27;
	else if(PI==4)
		bit_shift = 0;
	else
		bit_shift = 5;

	// Disable Clock
	DDRPLL_CLK_Disable();

	// Set PI
	if(PI<=3){
		PLL_target = (*PLL0 & ~(bit_mask << bit_shift)) | (PI_val << bit_shift);
		for(j=0;j<32;j++){		//change echo bit per time
			*PLL0 = (*PLL0 & ~(0x1<<j)) | (((PLL_target>>j)& 0x1)<<j);
		}
	}else{
		PLL_target = (*PLL1 & ~(bit_mask << bit_shift)) | (PI_val << bit_shift);
		for(j=0;j<32;j++){		//change echo bit per time
			*PLL1 = (*PLL1 & ~(0x1<<j)) | (((PLL_target>>j)& 0x1)<<j);
		}
	}
	// Set oesync=1, if PI > f
	if((REG32(MEMPLL63_32)>>5) & 0x10) { REG32(MEMPLL127_96) |= 1<<5;}
	if(REG32(MEMPLL63_32) & 0x10) { REG32(MEMPLL127_96) |= 1<<4;}
	if((REG32(MEMPLL31_0)>>27) & 0x10) { REG32(MEMPLL127_96) |= 1<<3;}
	if((REG32(MEMPLL31_0)>>22) & 0x10) { REG32(MEMPLL127_96) |= 1<<2;}
	if((REG32(MEMPLL31_0)>>17) & 0x10) { REG32(MEMPLL127_96) |= 1<<1;}
	if((REG32(MEMPLL31_0)>>12) & 0x10) { REG32(MEMPLL127_96) |= 1;}

	// Enable Clock
	DDRPLL_CLK_Enable();
	// DRAM PHY reset
	memctlc_dram_phy_reset();

}

void PLL_Setting(unsigned char PLL, unsigned int PLL_target){

	unsigned char j;
	uint32 *PLL0, *PLL1;
	PLL0 = (uint32 *)MEMPLL31_0;
	PLL1 = (uint32 *)MEMPLL63_32;

	// Disable Clock
	DDRPLL_CLK_Disable();

	if(PLL==0){
		for(j=0;j<32;j++){		//change echo bit per time
			*PLL0 = (*PLL0 & ~(0x1<<j)) | (((PLL_target>>j)& 0x1)<<j);
		}
	}else if(PLL==1){
		for(j=0;j<32;j++){		//change echo bit per time
			*PLL1 = (*PLL1 & ~(0x1<<j)) | (((PLL_target>>j)& 0x1)<<j);
		}
	}else{}
	// Set oesync=1, if PI > f
	if((REG32(MEMPLL63_32)>>5) & 0x10) { REG32(MEMPLL127_96) |= 1<<5;}
	if(REG32(MEMPLL63_32) & 0x10) { REG32(MEMPLL127_96) |= 1<<4;}
	if((REG32(MEMPLL31_0)>>27) & 0x10) { REG32(MEMPLL127_96) |= 1<<3;}
	if((REG32(MEMPLL31_0)>>22) & 0x10) { REG32(MEMPLL127_96) |= 1<<2;}
	if((REG32(MEMPLL31_0)>>17) & 0x10) { REG32(MEMPLL127_96) |= 1<<1;}
	if((REG32(MEMPLL31_0)>>12) & 0x10) { REG32(MEMPLL127_96) |= 1;}

	// Enable Clock
	DDRPLL_CLK_Enable();
	// DRAM PHY reset
	memctlc_dram_phy_reset();
}

void PI_Scan(unsigned char PI){

	unsigned char i;
	uint32 *PLL0, *PLL1, *PLL3;
	uint32 PLL0_tmp, PLL1_tmp, PLL3_tmp;

	PLL0 = (uint32 *)MEMPLL31_0;
	PLL1 = (uint32 *)MEMPLL63_32;
	PLL3 = (uint32 *)MEMPLL127_96;
	PLL0_tmp = *PLL0;
	PLL1_tmp = *PLL1;
	PLL3_tmp = *PLL3;

	puts("===== START PI"); puthex(PI); puts(" delay tap MAP =====\n");
	puts("0 1 2 3 4 5 6 7 8 9 a b c d e f|0 1 2 3 4 5 6 7 8 9 a b c d e f\n");

	for(i=0;i<32;i++){

		PI_Setting(PI,i);
//		puts("*PLL0 = "); puthex(*PLL0); puts("  *PLL1 = "); puthex(*PLL1); puts("\n");

		// DRAM TEST
		if ((Simple_DRAM_test(0x80000000,0x40000))==0)
			puts("0 "); 	//PASS
		else
			puts("1 ");		//FAIL

	}

	puts("\n");
//	DRAM_param_restore(0);


	// Restore delay tap
	DDRPLL_CLK_Disable();
	*PLL0=PLL0_tmp;
	*PLL1=PLL1_tmp;
	*PLL3=PLL3_tmp;
	// Enable Clock
	DDRPLL_CLK_Enable();
	// DRAM PHY reset
	memctlc_dram_phy_reset();

	puts("RECVOERY DEFAULT:"); puts("*PLL0="); puthex(*PLL0); puts(" *PLL1="); puthex(*PLL1);puts("\n");

}

void PI_Scan3(void){

	unsigned char i;
	uint32 PI0_val, PI2_val, PI3_val,*PLL0;
	uint32 *PLL1, *PLL3;
	uint32 PLL0_tmp, PLL1_tmp,PLL3_tmp;
//	unsigned char j;
//	uint32 *dig_delay_reg, *dig_delay_base;

//	dig_delay_base = (uint32 *)0xb8001510;
	PLL0 = (uint32 *)MEMPLL31_0;
	PLL1 = (uint32 *)MEMPLL63_32;
	PLL3 = (uint32 *)MEMPLL127_96;
	PLL0_tmp = *PLL0;
	PLL1_tmp = *PLL1;
	PLL3_tmp = *PLL3;

	PI0_val = (*PLL0 >> 12) & 0x1f;
	PI2_val = (*PLL0 >> 22) & 0x1f;
	PI3_val = (*PLL0 >> 27) & 0x1f;

	puts("===== START PI0/PI2/PI3 delay tap MAP =====\n");
	puts("PI0 = 0x"); puthex(PI0_val); 	puts(" PI2 = 0x"); puthex(PI2_val); 	puts(" PI3 = 0x"); puthex(PI3_val);
	puts("\n");
	puts("0 1 2 3 4 5 6 7 8 9 a b c d e f|0 1 2 3 4 5 6 7 8 9 a b c d e f\n");

	for(i=0;i<20;i++){

		PI_Setting(0,(PI0_val+i)>0x1f?0x1f:(PI0_val+i));
		PI_Setting(2,PI2_val+i);
		PI_Setting(3,PI3_val+i);
//		puts("*PLL0=0x"); puthex(*PLL0);puts("  *PLL1=0x"); puthex(*PLL1); puts("  *PLL3=0x"); puthex(*PLL3); puts("\n");
//		delay_time(0x10000000);
/*
		// digitial delay sync
		for (j=0;j<16;j++){
			dig_delay_reg = dig_delay_base + j;
			*dig_delay_reg = *dig_delay_reg &

		}
*/
		// DRAM TEST
		if ((Simple_DRAM_test(0x80000000,0x40000))==0)
			puts("0 "); 	//PASS
		else
			puts("1 ");		//FAIL

	}

	puts("\n");
//	DRAM_param_restore(0);
//	puts("*PLL0=0x"); puthex(PLL0_tmp);puts("  *PLL1=0x"); puthex(PLL1_tmp); puts("  *PLL3=0x"); puthex(PLL3_tmp); puts("\n");

	// Restore delay tap
	DDRPLL_CLK_Disable();
	*PLL0=PLL0_tmp;
	*PLL1=PLL1_tmp;
	*PLL3=PLL3_tmp;
	// Enable Clock
	DDRPLL_CLK_Enable();
	// DRAM PHY reset
	memctlc_dram_phy_reset();
	// DRAM reset
	memctlc_ddr3_dll_reset();

	puts("RECVOERY DEFAULT:"); puts("*PLL0="); puthex(*PLL0); puts(" *PLL1="); puthex(*PLL1);puts("\n");

}


void Write_Leveling(unsigned char apply){

	unsigned char i;
	uint32 *dtr0,*dcfdrr0,*dcfdrr1,*dcfdfr0,*dcfdfr1,*dmcr;
	uint32 PLL0_good=0,dtr0_tmp,cl,chg;

	dtr0 = (uint32 *)DTR0;
	dcfdrr0 = (uint32 *)0xB8001594;
	dcfdrr1 = (uint32 *)0xB8001598;
	dcfdfr0 = (uint32 *)0xB800159c;
	dcfdfr1 = (uint32 *)0xB80015a0;
	dmcr = (uint32 *)DMCR;
	chg=0;

 	// Store DTR0 value
	dtr0_tmp = *dtr0;
	// Set CL = CWL = PHY
	cl = (*dtr0 >> 28) & 0xf;
	*dtr0 = (*dtr0 & ~((0xf << 20) | 0xf)) | (cl <<20) | cl;
	puts("set CL = CWL = PHY, DTR = "); puthex(
*dtr0); puts("\n");

	// Enable DRAM write leveling
	*dmcr = 0x110084;
	while(*dmcr & DMCR_MRS_BUSY);
	delay_time(0x10000);

	// Enable controller write leveling
	*dmcr = 0x2130000;
	while(*dmcr & DMCR_MRS_BUSY);

	memctlc_dram_phy_reset();

	puts("===== START Write Leveling by CLK (if 1, FIFO!=0, it means CLK leads DQS0/1)=====\n");
	puts("0 1 2 3 4 5 6 7 8 9 a b c d e f|0 1 2 3 4 5 6 7 8 9 a b c d e f\n");

	// Set PI0-Clock 0~16
	for (i=0;i<16;i++){
		PI_Setting(0,i);
		//puts("*PLL0 = "); puthex(REG32(0xb8000234)); puts("  *PLL1 = "); puthex(REG32(0xb8000238)); puts("\n");
		REG32(0xa0000000)=0x5a5a5a5a;
		REG32(0xa0000004)=0xa5a5a5a5;

		if(*dcfdrr0|*dcfdrr1|*dcfdfr0|*dcfdfr1){
			puts("1 ");		// CLK leads DQS0/1
			if(i!=0 && chg==0) {PLL0_good = REG32(MEMPLL31_0);}
			chg=1;
		}else{
			puts("0 ");		// DQS0/1 leads CLK
			if(i!=0 && chg==1) {PLL0_good = REG32(MEMPLL31_0);}
			chg=0;
		}
//		puthex((*dcfdrr0|*dcfdrr1|*dcfdfr0|*dcfdfr1)?1:0);puts(" ");
	}
	puts("\n");
	DRAM_param_restore(0);

	puts("===== START Write Leveling by DQS0/1 (if 1, FIFO!=0, it means CLK leads DQS0/1)=====\n");
	puts("0 1 2 3 4 5 6 7 8 9 a b c d e f|0 1 2 3 4 5 6 7 8 9 a b c d e f\n");

	// Set PI2/3-DQS0/1 0~16
	for (i=0;i<16;i++){

		PI_Setting(2,i);
		PI_Setting(3,i);
		//puts("*PLL0 = "); puthex(REG32(0xb8000234)); puts("  *PLL1 = "); puthex(REG32(0xb8000238)); puts("\n");
		REG32(0xa0000000)=0x5a5a5a5a;
		REG32(0xa0000004)=0xa5a5a5a5;

		if(*dcfdrr0|*dcfdrr1|*dcfdfr0|*dcfdfr1){
			puts("1 ");		// CLK leads DQS0/1
			if(i!=0 && chg==0) {PLL0_good = REG32(MEMPLL31_0);}
			chg=1;
		}else{
			puts("0 ");		// DQS0/1 leads CLK
			if(i!=0 && chg==1) {PLL0_good = REG32(MEMPLL31_0);}
			chg=0;
		}
//		puthex((*dcfdrr0|*dcfdrr1|*dcfdfr0|*dcfdfr1)?1:0);puts(" ");
	}
	puts("\n");

	if(apply==1){
	// Set PLL scan value
		PLL_Setting(0,PLL0_good);
		puts("Set PLL0 0x"); puthex(REG32(MEMPLL31_0)); puts("\n");
	}
	// Recovery DTR0 value
	*dtr0 = dtr0_tmp;
	puts("RECVOERY DEFAULT:"); puts("*DTR0="); puthex(*dtr0); puts("\n");

	memctlc_ddr3_dll_reset();

}

#endif

void DDR_check_reset(void){

	REG32(0xA1F01754)=0;
	REG32(0xA1F01754)=0xcafe9999;

	if(REG32(0xA1F01754)!=0xcafe9999){	 //DRAM test FAIL
		//workaround
		printf("AK: Reset & init. DRAM again\n\r");	//FAIL
		memctlc_reset_procedure();
		if(memctlc_DDR_Type()==IS_DDR3_SDRAM)
			memctlc_ddr3_dll_reset();
		else
			memctlc_ddr2_dll_reset();
	}

	if(Simple_DRAM_test(0x80008000,0x100000)!=0){	 //DRAM test FAIL
		//workaround
		printf("AK: Reset & init. DRAM again\n\r");	//FAIL
		memctlc_reset_procedure();
		if(memctlc_DDR_Type()==IS_DDR3_SDRAM)
			memctlc_ddr3_dll_reset();
		else
			memctlc_ddr2_dll_reset();
	}
}

#define DDR_AK_VERSION (0x00040004) //V4.4 = 0x0004_0004
symb_idefine(
	ddr_ak_version,
	ENV_DDR_AK_VERSION,
	DDR_AK_VERSION
	);

/*********** U28 DRAM calibration V4.4 20220303 **************
 V0.1: First version for RTL9603C-vd
 V0.2: Support DRAM clock rate 400MHz~600MHz
 V0.3: 1.Tune DRAM controller driving   2.Modify printing informations
       3.Optimize code flow   4.Add DM auto selection
 V0.4: 1. set zctrl_clk_sel = zclk/32   2. DDR3 600MHz DTR1=0x808041f
 V0.5: Merge RTL9603C into RTL9603C-vd
 V0.6: Tune DRAM parameter by Efuse value
 V4.0: Add efuse vendor define and Winbond DQS enable patch
 V4.1: Etron DQS enable patch(formal release)
 V4.2: Update DDR2/DDR3 MRS flow and watchdog_enable function
 V4.3: If fail, trigger reset again(for Winbond). Correct cache flush function. 
 V4.3: optimize code
 V4.4: Fix dram init. issue fail for specific dram model (workaround)
************************************************************/

void memctlc_init_dram(void)
{
    printf("\nAK: ******* DRAM calibration version: v%d.%d *******.\n", (DDR_AK_VERSION>>16)&0xFFFF, DDR_AK_VERSION&0xFFFF);

	//For RTL9603C-vd
	struct ZQ_Value DDR2_VALUE[] = {
		{.group = "  Clock",	.ODT = 51, .OCD = 60},
		{.group = "Address",	.ODT = 51, .OCD = 53},
		{.group = " 	DQ",	.ODT = 51, .OCD = 48},
		{.group = "    DQS",	.ODT = 51, .OCD = 48},
	};

	struct ZQ_Value DDR3_VALUE[] = {
		{.group = "  Clock",	.ODT = 51, .OCD = 40},
		{.group = "Address",	.ODT = 51, .OCD = 40},
		{.group = " 	DQ",	.ODT = 51, .OCD = 48},
		{.group = "    DQS",	.ODT = 51, .OCD = 40},
	};

	struct ZQ_Value DDR3_MCM_VALUE[] = {
		{.group = "  Clock",	.ODT = 51, .OCD = 53},
		{.group = "Address",	.ODT = 51, .OCD = 60},
		{.group = " 	DQ",	.ODT = 51, .OCD = 48},
		{.group = "    DQS",	.ODT = 51, .OCD = 60},
	};

	//For Apollo pro
	struct ZQ_Value APRO_DDR2_VALUE[] = {
		{.group = "  Clock",	.ODT = 51, .OCD = 60},
		{.group = "Address",	.ODT = 51, .OCD = 53},
		{.group = " 	DQ",	.ODT = 51, .OCD = 48},
		{.group = "    DQS",	.ODT = 51, .OCD = 48},
	};

	struct ZQ_Value APRO_DDR3_VALUE[] = {
		{.group = "  Clock",	.ODT = 30, .OCD = 60},
		{.group = "Address",	.ODT = 30, .OCD = 53},
		{.group = " 	DQ",	.ODT = 30, .OCD = 48},
		{.group = "    DQS",	.ODT = 30, .OCD = 48},
	};

	struct ZQ_Value APRO_DDR3_MCM_VALUE[] = {
		{.group = "  Clock",	.ODT = 30, .OCD = 48},
		{.group = "Address",	.ODT = 30, .OCD = 80},
		{.group = " 	DQ",	.ODT = 30, .OCD = 60},
		{.group = "    DQS",	.ODT = 30, .OCD = 60},
	};


	/* unsigned int dram_type, i; */
	unsigned int mem_clk_mhz, dram_size=0;
	volatile unsigned int delay_loop;
	memcntlr_plat_preset();

	//Efuse patch
	EFPH_patch_num=Efuse_DRAM_patch();
	printf(" EP=%d\n",EFPH_patch_num);
	
	// Enable watchdog for whole the initial procedure
	sys_watchdog_enable(20, 1);

	// DRAM Reset Proceduce
	memctlc_reset_procedure();

	/* Delay a little bit for waiting for system to enter stable state.*/
//	delay_loop = 0x2000;
//	while(delay_loop--);

	// Triplle sync, read buffer full mask setting
	memctlc_config_misc();

#ifndef USE_OTTO_CG_FUNCTION
	/* Controller PLL setting */
	DRAM_frequency_setting();
	mem_clk_mhz = board_DRAM_freq_mhz();
	CMU_setup(mem_clk_mhz);
#else
    mem_clk_mhz = board_DRAM_freq_mhz();
#endif
	memctlc_config_delay_line(mem_clk_mhz);

	/* Delay a little bit for waiting for system to enter stable state.*/
	delay_loop = 0x1000;
	while(delay_loop--);

	/* Configure DRAM timing parameters */
#ifdef CONFIG_DRAM_AUTO_TIMING_SETTING
	memctlc_config_DTR(1, dram_size);
#else
	memctlc_config_DTR();
#endif

#ifdef CONFIG_DRAM_PUPD
		memctlc_config_pupd();
#endif

	
	if(EFPH_patch_num!=0 && EFPH_ZQ_en==1){
		set_patch_value();
		memctlc_ZQ_seperate_calibration(PATCH_VALUE);
	}else{
		if (otto_is_apro()) {
			if(memctlc_DDR_Type()==IS_DDR2_SDRAM){			//DDR2
				memctlc_ZQ_seperate_calibration(APRO_DDR2_VALUE);
		        puts("ZQ: ");
			}else{
				if(xlat_dram_size_num()==NA){		//DDR3 discete
		            memctlc_ZQ_seperate_calibration(APRO_DDR3_VALUE);
					puts("ZQ: EXTERNAL DDR3");
				}else{										//DDR3 MCM
					memctlc_ZQ_seperate_calibration(APRO_DDR3_MCM_VALUE);
					puts("ZQ: INTERAL DDR3");
				}
				dram_ZQCS_ZQCL_enable();
			}
		}else{
			if(memctlc_DDR_Type()==IS_DDR2_SDRAM){			//DDR2
				memctlc_ZQ_seperate_calibration(DDR2_VALUE);
		        puts("ZQ: DDR2");
			}else{
				if(_is_mcm()!=1){		//DDR3 discrete
		            memctlc_ZQ_seperate_calibration(DDR3_VALUE);
					puts("ZQ: EXTERNAL DDR3");
				}else{										//DDR3 MCM
					memctlc_ZQ_seperate_calibration(DDR3_MCM_VALUE);
					puts("ZQ: INTERAL DDR3");
				}
				dram_ZQCS_ZQCL_enable();
			}
		}

	}
	/* Reset DRAM DLL */
	if(memctlc_DDR_Type()==IS_DDR2_SDRAM){
#ifdef DDR2_USAGE
		memctlc_ddr2_dll_reset();
#endif
	}else if(memctlc_DDR_Type()==IS_DDR3_SDRAM){
#ifdef DDR3_USAGE
		memctlc_ddr3_dll_reset();
#endif
	}else{
		puts("AK: Error, Unknown DRAM type!\n\r");
		while(1);
	}

	puts("AK: Start DDR_Calibration...\r");

	/* RTL8685G just do DQ/DQS calibration */
	DDR_Calibration(1);
	puts("AK: Finish Calibration:\rAK: ");

	memctlc_dram_phy_reset();

	DDR_check_reset();		//Maybe Winbond DRAM init. fail, it's workaround.

	/* Configure DRAM size */
	dram_size = memctlc_config_DRAM_size();

	/* DRAM size recovery for MCM */
	memctlc_DRAM_size_recovery(dram_size);

#ifdef CONFIG_DRAM_AUTO_TIMING_SETTING
	memctlc_config_DTR(0, dram_size);
#endif
	/* Disable Read after Write */
	//REG32(MCERAWCR0) = 0x17111100;	//Defualt value is already 1(Disable)
	/* Enable parallel bank & Outstanding ECO */
	REG32(DCR) |= (1<<12) | (3<<0);
	REG32(DMCR) = REG32(DMCR);
	while(REG32(DMCR) & DMCR_MRS_BUSY);

	sys_watchdog_disable();

	memcntlr_plat_postset();

	return;
}

#ifdef DDR2_USAGE
void memctlc_ddr2_dll_reset(void)
{
	volatile unsigned int *dmcr;
	unsigned int mr[4];

	dmcr = (volatile unsigned int *)DMCR;

	_DTR_DDR2_MRS_setting(mr);

	/* 1. Disable DLL */
	*dmcr = mr[1] | DDR2_EMR1_DLL_DIS;
	while(*dmcr & DMCR_MRS_BUSY);
	udelay(1);

	/* 2. Set EMR2 */
	*dmcr = mr[2];
	while(*dmcr & DMCR_MRS_BUSY);
	udelay(1);
	
	/* 3. Set EMR1 & Enable DLL */
	*dmcr = mr[1] & (~DDR2_EMR1_DLL_DIS);
	while(*dmcr & DMCR_MRS_BUSY);
	udelay(1);

	/* 4. Set MR & Reset DLL */
	*dmcr = mr[0] | DDR2_MR_DLL_RESET_YES ;
	while(*dmcr & DMCR_MRS_BUSY);
	udelay(1);

	/* 5. Waiting 200 clock cycles */
	udelay(4);

	/* 6. reset phy fifo */
	memctlc_dram_phy_reset();

	return;
}
#endif

#ifdef DDR3_USAGE
void memctlc_ddr3_dll_reset(void)
{
	volatile unsigned int *dmcr, *dtr0;
	unsigned int dtr[3], mr[4];

	dmcr = (volatile unsigned int *)DMCR;
	dtr0 = (volatile unsigned int *)DTR0;

	dtr[0]= *dtr0;
	dtr[1]= *(dtr0 + 1);
	dtr[2]= *(dtr0 + 2);

	_DTR_DDR3_MRS_setting(dtr, mr);

	/* 1. Disable DLL */
	*dmcr = mr[1] | DDR3_EMR1_DLL_DIS;
	while(*dmcr & DMCR_MRS_BUSY);
	udelay(1);

	/* 2. Set MR2 */
	*dmcr = mr[2];
	while(*dmcr & DMCR_MRS_BUSY);
	udelay(1);

	/* 3. Set MR3 */
	*dmcr = mr[3];
	while(*dmcr & DMCR_MRS_BUSY);
	udelay(1);

	/* 4. Set MR1 & Enable DLL */
	*dmcr = mr[1] & (~DDR3_EMR1_DLL_DIS);
	while(*dmcr & DMCR_MRS_BUSY);
	udelay(1);

	/* 5. Set MR0 & Reset DLL */
	*dmcr = mr[0] | DDR3_MR_DLL_RESET_YES ;
	while(*dmcr & DMCR_MRS_BUSY);
	udelay(1);

	/* 6. Waiting 512 clock cycles */
	udelay(4);

	/* 7. reset phy fifo */
	memctlc_dram_phy_reset();

	return;
}
#endif

void dram_ZQCS_ZQCL_enable(void)
{
	volatile unsigned int *ZQ_ctrl, *Sil_pat, *dmcr;

	ZQ_ctrl = (volatile unsigned int *)D3ZQCCR;
	Sil_pat = (volatile unsigned int *)DACSPCR;
	dmcr = (volatile unsigned int *)DMCR;

	/* ZQCL Trigger */
	*ZQ_ctrl = *ZQ_ctrl | ZQ_LONG_TRI;
	while(*ZQ_ctrl & ZQ_LONG_TRI_BUSY);

	/* ZQCS Enable */
	*Sil_pat = *Sil_pat | AC_SILEN_PERIOD_EN | AC_SILEN_PERIOD_UNIT | AC_SILEN_PERIOD;	//AC_SILEN_PERIOD_UNIT x AC_SILEN_PERIOD = ZQCS period
	*ZQ_ctrl = *ZQ_ctrl | ZQ_SHORT_EN | T_ZQCS;	//T_ZQCS means the ZQCS requires time, which dram can't access data

	/* DMCR update */
	*dmcr = *dmcr;
	puts(", ZQCL done, ZQCS Enable\n\r");

}

void memctlc_dram_phy_reset(void)
{
	volatile unsigned int *phy_ctl;

	phy_ctl = (volatile unsigned int *)DACCR;
	*phy_ctl = *phy_ctl & ((unsigned int) 0xFFFFFFEF);
	*phy_ctl = *phy_ctl | ((unsigned int) 0x10);
	//_memctl_debug_printf("memctlc_dram_phy_reset: 0x%08p(0x%08x)\n", phy_ctl, *phy_ctl);

	return;
}

