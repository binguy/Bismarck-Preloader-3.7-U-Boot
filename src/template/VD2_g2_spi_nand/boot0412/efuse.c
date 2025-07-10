/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2003  
* All rights reserved.
* 
* Abstract: 
* 		Efuse R/W source code.
*		Kevin.Chung@realtek.com
* ---------------------------------------------------------------
*/
#include "efuse.h"
#include "board.h"

struct efuse_data efuse_info __attribute__ ((section (".sdata")));

static inline void delay(void) {
	volatile unsigned int wait_loop_cnt = 0;

	wait_loop_cnt = 500 * 10000;
        while(wait_loop_cnt--);
}

/*	
  * Function		: EFUSE_ReadPhyMdio for RL6518
  * Input			: regaddr
  * Output		: return value
  * Description 	: Read Giga_EthPHY parameters through MDIO 
  *
  */
unsigned short memctl_EFUSE_ReadPhyMdio(unsigned char regaddr)
{
	REG32(EFUSE_ADDR)	= (1<<24) | ((regaddr&0x1f)<<EFUSE_REG_OFFSET); 
	REG32(EFUSE_ADDR)	= (0<<24) | ((regaddr&0x1f)<<EFUSE_REG_OFFSET);
	delay();
	return (REG32(EFUSE_ADDR1)&0x0ffff);
}


/*	
  * Function		: EFUSE_SetPhyMdioWrite for RL6518
  * Input			: regaddr, val
  * Description 	: Write Giga_Eth PHY parameters through MDIO 
  *
  */
void memctl_EFUSE_SetPhyMdioWrite(unsigned char regaddr, unsigned short val)
{
	REG32(EFUSE_ADDR)	= (06<<24) | ( (regaddr&0x1f)<<EFUSE_REG_OFFSET) | ((val&0xffff)<<EFUSE_DATA_OFFSET) ; 
	REG32(EFUSE_ADDR)	= (07<<24) | ( (regaddr&0x1f)<<EFUSE_REG_OFFSET) | ((val&0xffff)<<EFUSE_DATA_OFFSET) ; 
	REG32(EFUSE_ADDR1)	= (04<<24);
	delay();
}


void get_efuse_info(void) 
{
	int port;
	unsigned int gphy_efuse_val;

	//cpu freq.
	efuse_info.cpuFreq = (memctl_EFUSE_ReadPhyMdio(CPU_INFO_EFUSE_ADDR) >> CPU_INFO_EFUSE_OFFSET) & CPU_INFO_EFUSE_BITMASK;
	//efuse_info.cpuFreq = 0x3;
	//ldo
	efuse_info.ldo = memctl_EFUSE_ReadPhyMdio(DRAM_LDO_EFUSE_ADDR);
	//efuse_info.ldo = 0x0007;
	//gphy cap.
	gphy_efuse_val = memctl_EFUSE_ReadPhyMdio(GPHY_INFO_EFUSE_ADDR);
	for (port = 0; port < GPHY_PORT_NUM; port++) {
		efuse_info.gphyCap[port] = (gphy_efuse_val >> port*3) & GPHY_INFO_EFUSE_BITMASK;
		//efuse_info.gphyCap[port] = 1;
	}	
}

unsigned short efuse_get_phy_cap(int port) {
	unsigned short val = 0;

	switch (efuse_info.gphyCap[port]) {
		case gphyCap_FE:
			val = 0xc;
			break;

		case gphyCap_GE:
			val = 0x0;
			break;
	}

	return val;
}
