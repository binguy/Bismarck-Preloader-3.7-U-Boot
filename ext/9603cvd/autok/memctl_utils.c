#include <soc.h>
#include "autoconf.h"

#include "./bspchip.h"
#include "./memctl.h"
#include "./memctl_func.h"

#include <extra.h>

unsigned int get_memory_ddr2_dram_odt_parameters(unsigned int *para_array);
unsigned int get_memory_dram_reduce_drv_parameters(unsigned int *para_array);
unsigned int get_memory_ddr3_dram_rtt_nom_parameters(unsigned int *para_array);
unsigned int get_memory_ddr3_dram_rtt_wr_parameters(unsigned int *para_array);

/* DRAM patch from efuse */
extern unsigned int EFPH_patch_num;
extern unsigned int EFPH_DRAM_ZQ_en;
extern unsigned int EFPH_DRAM_ODT;
extern unsigned int EFPH_DRAM_driving;


/* Function Name:
 * 	memctlc_DDR_Type
 * Descripton:
 *	Determine the DRAM type.
 * Input:
 *	None
 * Output:
 * 	None
 * Return:
 *     3 - DRAM type is DDR3 SDRAM
 *     2  -DRAM type is DDR2 SDRAM
 *  	1  -DRAM type is DDR SDRAM
 *	0  -DRAM type isn't DDR SDRAM
 */
unsigned int memctlc_DDR_Type(void)
 {
	if(MCR_DRAMTYPE_DDR3 == (REG32(MCR) & MCR_DRAMTYPE_MASK))
		return 3;
	else if(MCR_DRAMTYPE_DDR2 == (REG32(MCR) & MCR_DRAMTYPE_MASK))
		return 2;
	else if(MCR_DRAMTYPE_DDR == (REG32(MCR) & MCR_DRAMTYPE_MASK))
		return 1;
	else
		return 0;
 }

/* Function Name:
 * 	_DTR_DDR1_MRS_setting
 * Descripton:
 *	Find out the values of the mode registers according to the DTR0/1/2 setting
 *	for DDR1 SDRAM.
 * Input:
 *	sug_dtr	- The DTR0/1/2 setting.
 * Output:
 *	mr	- The values of the mode registers.
 * Return:
 *	None
 * Note:
 *	None
 */
#ifdef CONFIG_DDR1_USAGE
#if 1
void _DTR_DDR1_MRS_setting(unsigned int *mr)
#else
void _DTR_DDR1_MRS_setting(unsigned int *sug_dtr, unsigned int *mr)
#endif
{
	unsigned int cas, buswidth;
	/* Default value of Mode registers */
	mr[0] = DMCR_MRS_MODE_MR | DDR1_MR_BURST_SEQ | DDR1_MR_OP_NOR |\
		DMCR_MR_MODE_EN ;

	mr[1] = DMCR_MRS_MODE_EMR1 | DDR1_EMR1_DLL_EN | DDR1_EMR1_DRV_NOR |\
		DMCR_MR_MODE_EN;

	/* Extract CAS and WR in DTR0 */
	cas = (REG32(DTR0) & DTR0_CAS_MASK) >> DTR0_CAS_FD_S;
	buswidth = (REG32(DCR) & DCR_DBUSWID_MASK) >> DCR_DBUSWID_FD_S;
	switch (cas){
		case 0:
			mr[0] = mr[0] | DDR1_MR_CAS_25;
			break;
		case 1:
			mr[0] = mr[0] | DDR1_MR_CAS_2;
			break;
		case 2:
			mr[0] = mr[0] | DDR1_MR_CAS_3;
			break;
		default:
			mr[0] = mr[0] | DDR1_MR_CAS_3;
			break;

	}

	switch (buswidth){
		case 0:
			mr[0] = mr[0] | DDR1_MR_BURST_4;
			break;
		case 1:
			mr[0] = mr[0] | DDR1_MR_BURST_2;
			break;
		default:
			mr[0] = mr[0] | DDR1_MR_BURST_2;
			break;
	}

	return;
}
#endif

/* Function Name:
 * 	_DTR_DDR2_MRS_setting
 * Descripton:
 *	Find out the values of the mode registers according to the DTR0/1/2 setting
 *	for DDR2 SDRAM.
 * Input:
 *	sug_dtr	- The DTR0/1/2 setting.
 * Output:
 *	mr	- The values of the mode registers.
 * Return:
 *	None
 * Note:
 *	None
 */
#ifdef CONFIG_DDR2_USAGE
#if 1
void _DTR_DDR2_MRS_setting(unsigned int *mr)
#else
void _DTR_DDR2_MRS_setting(unsigned int *sug_dtr, unsigned int *mr)
#endif
{
	unsigned int cas, wr, odt_value, drv_str;
	/* Default value of Mode registers */
	mr[0] = DMCR_MRS_MODE_MR | DDR2_MR_BURST_4 | DDR2_MR_BURST_SEQ | \
		DDR2_MR_TM_NOR | DDR2_MR_DLL_RESET_NO | DDR2_MR_PD_FAST |\
		DMCR_MR_MODE_EN ;

	if(get_memory_ddr2_dram_odt_parameters(&odt_value)){
		switch (odt_value){
			case 0:
				odt_value = DDR2_EMR1_RTT_DIS;
				break;
			case 75:
				odt_value = DDR2_EMR1_RTT_75;
				break;
			case 150:
				odt_value = DDR2_EMR1_RTT_150;
				break;
			default: /* 50 */
				odt_value = DDR2_EMR1_RTT_50;
				break;
		}
	}else{
		odt_value = DDR2_EMR1_RTT_75;
	}
	
	if(get_memory_dram_reduce_drv_parameters(&drv_str)){
		if(drv_str){/* reduce */
			drv_str = DDR2_EMR1_DIC_REDUCE;
		}else{
			drv_str = DDR2_EMR1_DIC_FULL;
		}
	}else{ /* full mode */
		drv_str = DDR2_EMR1_DIC_FULL;
	}
	// DRAM patch from efuse
	if (EFPH_patch_num!=0 && EFPH_DRAM_ZQ_en){
		if(EFPH_DRAM_ODT==0)
			odt_value = DDR2_EMR1_RTT_DIS;
		else if(EFPH_DRAM_ODT==1)
			odt_value = DDR2_EMR1_RTT_75;
		else if(EFPH_DRAM_ODT==2)
			odt_value = DDR2_EMR1_RTT_150;
		else
			odt_value = DDR2_EMR1_RTT_50;

		if(EFPH_DRAM_driving==0)
			drv_str = DDR2_EMR1_DIC_REDUCE;
		else
			drv_str = DDR2_EMR1_DIC_FULL;
	}

	mr[1] = DDR2_EMR1_DLL_EN | drv_str |\
		odt_value | DDR2_EMR1_ADD_0 | DDR2_EMR1_OCD_EX | \
		DDR2_EMR1_QOFF_EN | DDR2_EMR1_NDQS_EN | DDR2_EMR1_RDQS_DIS |\
		DMCR_MR_MODE_EN | DMCR_MRS_MODE_EMR1;

	mr[2] = DDR2_EMR2_HTREF_DIS | DDR2_EMR2_DCC_DIS | DDR2_EMR2_PASELF_FULL |\
		DMCR_MR_MODE_EN | DMCR_MRS_MODE_EMR2;

	mr[3] = DMCR_MR_MODE_EN | DMCR_MRS_MODE_EMR3;

	/* Extract CAS and WR in DTR0 */
	cas = REG32(DTR0) & DTR0_CAS_PHY_MASK;
	wr = (REG32(DTR0) & DTR0_WR_MASK) >> DTR0_WR_FD_S;

	if( cas == 1)
		mr[0] = mr[0] | DDR2_MR_CAS_2;
	else if(cas == 2)
		mr[0] = mr[0] | DDR2_MR_CAS_3;
	else if(cas == 3)
		mr[0] = mr[0] | DDR2_MR_CAS_4;
	else if(cas == 4)
		mr[0] = mr[0] | DDR2_MR_CAS_5;
	else if(cas == 5)
		mr[0] = mr[0] | DDR2_MR_CAS_6;
	else if(cas ==6)
		mr[0] = mr[0] | DDR2_MR_CAS_7;
	else
		mr[0] = mr[0] | DDR2_MR_CAS_6;

	if( wr == 1)
		mr[0] = mr[0] | DDR2_MR_WR_2;
	else if(wr == 2)
		mr[0] = mr[0] | DDR2_MR_WR_3;
	else if(wr == 3)
		mr[0] = mr[0] | DDR2_MR_WR_4;
	else if(wr == 4)
		mr[0] = mr[0] | DDR2_MR_WR_5;
	else if(wr == 5)
		mr[0] = mr[0] | DDR2_MR_WR_6;
	else if(wr == 6)
		mr[0] = mr[0] | DDR2_MR_WR_7;
	else
		mr[0] = mr[0] | DDR2_MR_WR_7;

	puts("AK: MRS: mr[0]=0x");puthex(mr[0]);
	puts(", mr[1]=0x");puthex(mr[1]);
	puts(", mr[2]=0x");puthex(mr[2]);
	puts(", mr[3]=0x");puthex(mr[3]);puts("\n\r");

	return;
}
#endif

/* Function Name:
 * 	_DTR_DDR3_MRS_setting
 * Descripton:
 *	Find out the values of the mode registers according to the DTR0/1/2 setting
 *	for DDR2 SDRAM.
 * Input:
 *	sug_dtr	- The DTR0/1/2 setting.
 * Output:
 *	mr	- The values of the mode registers.
 * Return:
 *	None
 * Note:
 *	None
 */
#ifdef CONFIG_DDR3_USAGE
void _DTR_DDR3_MRS_setting(unsigned int *sug_dtr, unsigned int *mr)
{
	unsigned int cas, wr, cwl, MRS_tmp;
	unsigned int rtt_nom_value, rtt_wr_value, dram_reduce_drv;

	/* Default value of Mode registers */
	mr[0] = DMCR_MRS_MODE_MR | DDR3_MR_BURST_8 | DDR3_MR_READ_BURST_NIBBLE | \
		DDR3_MR_TM_NOR | DDR3_MR_DLL_RESET_NO | DDR3_MR_PD_FAST |\
		DMCR_MR_MODE_EN ;

	mr[1] = DDR3_EMR1_DLL_EN | DDR3_EMR1_DIC_RZQ_DIV_6 |\
		DDR3_EMR1_RTT_NOM_DIS | DDR3_EMR1_ADD_0 | DDR3_EMR1_WRITE_LEVEL_DIS | \
		DDR3_EMR1_TDQS_DIS | DDR3_EMR1_QOFF_EN |\
		DMCR_MR_MODE_EN | DMCR_MRS_MODE_EMR1;
	mr[2] = DDR3_EMR2_PASR_FULL | DDR3_EMR2_ASR_DIS | DDR3_EMR2_SRT_NOR |\
		DDR3_EMR2_RTT_WR_DIS | DMCR_MR_MODE_EN | DMCR_MRS_MODE_EMR2;

	mr[3] = DDR3_EMR3_MPR_OP_NOR | DDR3_EMR3_MPR_LOC_PRE_PAT |\
		DMCR_MR_MODE_EN | DMCR_MRS_MODE_EMR3;


	if(get_memory_ddr3_dram_rtt_nom_parameters(&rtt_nom_value)){
		if(rtt_nom_value != 0){
			MRS_tmp=(240/rtt_nom_value)/2;
			if(MRS_tmp==1)			/* div 2 */
				rtt_nom_value = DDR3_EMR1_RTT_NOM_RZQ_DIV2;
			else if(MRS_tmp==2)		/* div 4*/
				rtt_nom_value = DDR3_EMR1_RTT_NOM_RZQ_DIV4;
			else if(MRS_tmp==3)		/* div 6 */
				rtt_nom_value = DDR3_EMR1_RTT_NOM_RZQ_DIV6;
			else if(MRS_tmp==4)		/* div 8 */
				rtt_nom_value = DDR3_EMR1_RTT_NOM_RZQ_DIV8;
			else if(MRS_tmp==6)		/* div 12 */
				rtt_nom_value = DDR3_EMR1_RTT_NOM_RZQ_DIV12;
			else 						/* 40 */
				rtt_nom_value = DDR3_EMR1_RTT_NOM_RZQ_DIV2;

		}else{
			rtt_nom_value = DDR3_EMR1_RTT_NOM_DIS;
		}
	}else{
		rtt_nom_value = DDR3_EMR1_RTT_NOM_DIS;
	}

	if(get_memory_ddr3_dram_rtt_wr_parameters(&rtt_wr_value)){
		if(rtt_wr_value != 0){
			MRS_tmp=(240/rtt_wr_value)/2;
			if(MRS_tmp==2)		/* div 4 */
				rtt_wr_value = DDR3_EMR2_RTT_WR_RZQ_DIV_4;
			else					/* div 2 */
				rtt_wr_value = DDR3_EMR2_RTT_WR_RZQ_DIV_2;
		}else{
			rtt_wr_value = DDR3_EMR2_RTT_WR_DIS;
		}
	}else{
		rtt_wr_value = DDR3_EMR2_RTT_WR_RZQ_DIV_2;
	}


	if(get_memory_dram_reduce_drv_parameters(&dram_reduce_drv)){
		if(dram_reduce_drv){
			dram_reduce_drv = DDR3_EMR1_DIC_RZQ_DIV_6;
		}else{
			dram_reduce_drv = DDR3_EMR1_DIC_RZQ_DIV_7;
		}
	}else{
		dram_reduce_drv = DDR3_EMR1_DIC_RZQ_DIV_6;
	}
	// DRAM patch from efuse
	if (EFPH_patch_num!=0 && EFPH_DRAM_ZQ_en){
		if(EFPH_DRAM_ODT==0)
			rtt_nom_value = DDR3_EMR1_RTT_NOM_DIS;
		else if(EFPH_DRAM_ODT==1)
			rtt_nom_value = DDR3_EMR1_RTT_NOM_RZQ_DIV4;
		else if(EFPH_DRAM_ODT==2)
			rtt_nom_value = DDR3_EMR1_RTT_NOM_RZQ_DIV2;
		else
			rtt_nom_value = DDR3_EMR1_RTT_NOM_RZQ_DIV6;

		if(EFPH_DRAM_driving==0)
			dram_reduce_drv = DDR3_EMR1_DIC_RZQ_DIV_6;
		else
			dram_reduce_drv = DDR3_EMR1_DIC_RZQ_DIV_7;
	}

	mr[1] = mr[1] | rtt_nom_value | dram_reduce_drv;
	mr[2] = mr[2] | rtt_wr_value ;



	/* Extract CAS and WR in DTR0 */
	cas = sug_dtr[0] & DTR0_CAS_PHY_MASK;
	wr = (sug_dtr[0] & DTR0_WR_MASK) >> DTR0_WR_FD_S;
	cwl = (sug_dtr[0] & DTR0_CWL_MASK) >> DTR0_CWL_FD_S;

	if(cas==4)
		mr[0] = mr[0] | DDR3_MR_CAS_5;
	else if(cas==5)
		mr[0] = mr[0] | DDR3_MR_CAS_6;
	else if(cas==6)
		mr[0] = mr[0] | DDR3_MR_CAS_7;
	else if(cas==7)
		mr[0] = mr[0] | DDR3_MR_CAS_8;
	else if(cas==8)
		mr[0] = mr[0] | DDR3_MR_CAS_9;
	else if(cas==9)
		mr[0] = mr[0] | DDR3_MR_CAS_10;
	else if(cas==10)
		mr[0] = mr[0] | DDR3_MR_CAS_11;
	else if(cas==11)
		mr[0] = mr[0] | DDR3_MR_CAS_12;
	else if(cas==12)
		mr[0] = mr[0] | DDR3_MR_CAS_13;
	else if(cas==13)
		mr[0] = mr[0] | DDR3_MR_CAS_14;
	else
		mr[0] = mr[0] | DDR3_MR_CAS_6;

	if(wr==4)
		mr[0] = mr[0] | DDR3_MR_WR_5;
	else if(wr==5)
		mr[0] = mr[0] | DDR3_MR_WR_6;
	else if(wr==6)
		mr[0] = mr[0] | DDR3_MR_WR_7;
	else if(wr==7)
		mr[0] = mr[0] | DDR3_MR_WR_8;
	else if(wr==8)
		mr[0] = mr[0] | DDR3_MR_WR_10;
	else if(wr==9)
		mr[0] = mr[0] | DDR3_MR_WR_10;
	else if(wr==10)
		mr[0] = mr[0] | DDR3_MR_WR_12;
	else if(wr==11)
		mr[0] = mr[0] | DDR3_MR_WR_12;
	else if(wr==12)
		mr[0] = mr[0] | DDR3_MR_WR_14;
	else if(wr==13)
		mr[0] = mr[0] | DDR3_MR_WR_14;
	else
		mr[0] = mr[0] | DDR3_MR_WR_6;

	if(cwl==4)
		mr[2] = mr[2] | DDR3_EMR2_CWL_5;
	else if(cwl==5)
		mr[2] = mr[2] | DDR3_EMR2_CWL_6;
	else if(cwl==6)
		mr[2] = mr[2] | DDR3_EMR2_CWL_7;
	else if(cwl==7)
		mr[2] = mr[2] | DDR3_EMR2_CWL_8;
	else if(cwl==8)
		mr[2] = mr[2] | DDR3_EMR2_CWL_9;
	else
		mr[2] = mr[2] | DDR3_EMR2_CWL_6;

	puts("AK: MRS: mr[0]=0x");puthex(mr[0]);
	puts(",  mr[1]=0x");puthex(mr[1]);
	puts(",  mr[2]=0x");puthex(mr[2]);
	puts(",  mr[3]=0x");puthex(mr[3]);puts("\n\r");
	return;
}
#endif
