#include <soc.h>
UTIL_FAR void ft_enable_uart(void);
UTIL_FAR void ft_disable_uart(void);
//print use puts()
#define BIT(x) (1 << x)
#define UART_BASE_ADDRESS 0xb8002000
#define UART_RBR     *((volatile unsigned char *)UART_BASE_ADDRESS+0x00)
#define UART_LSR     *((volatile unsigned char *)UART_BASE_ADDRESS+0x14)
#define SYSREG_PLL_CTL32_63_REG (0xB800020C)
#define SYSREG_SYSTEM_STATUS_REG                (0xB8000044)
#define SYSREG_SYSTEM_STATUS_CF_CKSE_OCP0_MASK          BIT(2)
#define SYSREG_SYSTEM_STATUS_CF_CKSE_OCP1_MASK          BIT(3)
#define SYSREG_SYSCLK_CONTROL_REG               (0xB8000200)

#define SYSREG_ANA1_CTL_REG                     (0xB8000218)
#define EFUSE_ADDR 0xb8000644
#define EFUSE_ADDR1 0xb8000648
	
#define EFUSE_REG_OFFSET 		(16)
#define EFUSE_DATA_OFFSET 		(0)
#define DRAM_INFO_EFUSE_ADDR	(2)
#define DRAM_LDO_EFUSE_ADDR	(3)
#define DRAM_DSP_EFUSE_ADDR	(6)
int getchar(void)
{
	while(1 != (UART_LSR & 0x01));
	return (UART_RBR);
}
/*	
  * Function		: EFUSE_ReadPhyMdio for RL6518
  * Input			: regaddr
  * Output		: return value
  * Description 	: Read Giga_EthPHY parameters through MDIO 
  *
  */
unsigned short EFUSE_ReadPhyMdio(unsigned char regaddr){
	REG32(EFUSE_ADDR)	= (01<<24) | ( (regaddr&0x1f)<<EFUSE_REG_OFFSET); 
	REG32(EFUSE_ADDR)	= (00<<24);
	mdelay(1);
	return (REG32(EFUSE_ADDR1)&0x0ffff);
	//printf("HostPCIe PHY addr= 0x%02x, data= 0x%04x	 \n\r",(read_data>>8)&0xFF,read_data>>16 );
}


/*	
  * Function		: EFUSE_SetPhyMdioWrite for RL6518
  * Input			: regaddr, val
  * Description 	: Write Giga_Eth PHY parameters through MDIO 
  *
  */
void EFUSE_SetPhyMdioWrite(unsigned char regaddr, unsigned short val)
{
	REG32(EFUSE_ADDR)	= (06<<24) | ( (regaddr&0x1f)<<EFUSE_REG_OFFSET) | ((val&0xffff)<<EFUSE_DATA_OFFSET) ; 
	REG32(EFUSE_ADDR)	= (07<<24) | ( (regaddr&0x1f)<<EFUSE_REG_OFFSET) | ((val&0xffff)<<EFUSE_DATA_OFFSET) ; 
	REG32(EFUSE_ADDR1)	= (04<<24);
	mdelay(1);
}
unsigned char RL6518_DRAM_LDO_EFUSE_CHECK(unsigned char value)
{
	unsigned char bit_offset;
	unsigned short read_tmp,current_LDO_EFUSE_value;
	EFUSE_ReadPhyMdio(DRAM_LDO_EFUSE_ADDR);
	current_LDO_EFUSE_value=EFUSE_ReadPhyMdio(DRAM_LDO_EFUSE_ADDR);
	for(bit_offset=0;bit_offset<16;){
		read_tmp = ((current_LDO_EFUSE_value>>bit_offset)&0x0f);
		if(read_tmp==0){					//check LDO field empty.
			if(bit_offset==0){				//empty of efuse's LDO all field
				EFUSE_SetPhyMdioWrite(DRAM_LDO_EFUSE_ADDR,value);			//write value equal efuse vlaue
				return 1;
			}else{
				if(((current_LDO_EFUSE_value>>(bit_offset-4))&0x0f)==value)	//check write value equ before LDO field.
					return 1;
				else{
					EFUSE_SetPhyMdioWrite(DRAM_LDO_EFUSE_ADDR,(value<<bit_offset));
					EFUSE_ReadPhyMdio(DRAM_LDO_EFUSE_ADDR);
					return 1;
					}
				}
			}		//if(read_tmp==0)
		bit_offset+=4;
		}			//for loop
	return 0;
}
void RL6518_DRAM_LDO_OUTPUT_VOLTAGE_TEST(void)
{
	unsigned char index=0,input=0,start_step=0,ascii_base_offset=0;
	unsigned int reg_tmp=0;
	start_step = 1;
	REG32(0xb8000218) = 0x3d0;
	reg_tmp = REG32(SYSREG_ANA1_CTL_REG) & 0x01FFF0;
	for(index=start_step;index<15;index++){
		REG32(SYSREG_ANA1_CTL_REG) = reg_tmp | index;
		printf("T_DLDO_%d\n\r",index-start_step);
		while(1)	{
			input=getchar();
			if(input=='N')								//next step.
				break;				
			if((input>=0x30) & (input<=0x39)){					//if 'D' do-nothing, 'X' show calibration value but not write to efuse, 'Z' calibration value write to efuse
				ascii_base_offset=0x30;
				}
			if((input>=0x41) & (input<=0x46)){					//if 'D' do-nothing, 'X' show calibration value but not write to efuse, 'Z' calibration value write to efuse
				ascii_base_offset=0x41+10;
				}
			if((ascii_base_offset==0x30) | (ascii_base_offset==0x41)){
				if(RL6518_DRAM_LDO_EFUSE_CHECK((input-ascii_base_offset)+start_step)){
					printf("k_DLDO_%d\n\r",(input-ascii_base_offset));
				}else{
					printf("k_DLDO.\n\r");
					}
				REG32(SYSREG_ANA1_CTL_REG) = reg_tmp |((input-ascii_base_offset)+start_step);
				return;
				}
			if(input=='E'){								// return .
				printf("k_DLDO_%d\n\r",(input-ascii_base_offset));
				return;		
				}				
			}	//while(1)
		}
}

void ft_pll_setup(void)
{
	unsigned int wait_loop_cnt = 0;
	unsigned int sys_pll_ctrl_val = REG32(0xb8000200);
	unsigned int sysreg_pll_ctl_reg = REG32(SYSREG_PLL_CTL32_63_REG);
	char bufcc;
	sys_pll_ctrl_val &= 0xFF000000;
	printf("\n\rP_OCP\n\r");
	while(1) {
		bufcc=getchar();
		if(bufcc == '7') {
			sysreg_pll_ctl_reg |= (1<<18);
			sys_pll_ctrl_val |= 0x001C0E0E;
			printf("P_750M\n\r");
			break;
		}
		if(bufcc == '1') {
			sysreg_pll_ctl_reg &= ~(1<<18);
			sys_pll_ctrl_val |= 0x0012120E;
			printf("P_1000M\n\r");
			break;
		}
		if(bufcc == '5') {
			sysreg_pll_ctl_reg &= ~(1<<18);
			sys_pll_ctrl_val |= 0x0013120E;
			printf("P_1050M\n\r");
			break;
		}
		if(bufcc == '8') {
			sysreg_pll_ctl_reg |= (1<<18);
			sys_pll_ctrl_val |= 0x001E0E0E;
			printf("P_800M\n\r");
			break;
		}
		if(bufcc == '9') {
			sysreg_pll_ctl_reg &= ~(1<<18);
			sys_pll_ctrl_val |= 0x0010120E;
			printf("P_900M\n\r");
			break;
		}
	}
	/* (1) CPU CLOCK select LX PLL clock */
	wait_loop_cnt = 700 * 10000;
	while(wait_loop_cnt--) {
		if(wait_loop_cnt == 3500000) {
			REG32(SYSREG_SYSTEM_STATUS_REG) &= (~(SYSREG_SYSTEM_STATUS_CF_CKSE_OCP0_MASK|SYSREG_SYSTEM_STATUS_CF_CKSE_OCP1_MASK));
		}
	}
	/* (2) Invoke the PLL change. */
	wait_loop_cnt = 500 * 1000;
	while(wait_loop_cnt--);
	
	REG32(SYSREG_PLL_CTL32_63_REG) = sysreg_pll_ctl_reg;
	REG32(SYSREG_SYSCLK_CONTROL_REG) = sys_pll_ctrl_val;

	wait_loop_cnt = 500 * 1000;
	while(wait_loop_cnt--);

	/* (5) CPU CLOCK select OCP0 PLL clock */
	wait_loop_cnt = 700 * 10000;
	while(wait_loop_cnt--){
		if(wait_loop_cnt == 3500000) {
			REG32(SYSREG_SYSTEM_STATUS_REG) |=  SYSREG_SYSTEM_STATUS_CF_CKSE_OCP0_MASK|SYSREG_SYSTEM_STATUS_CF_CKSE_OCP1_MASK;
		}
	}
//	printf("PLL_63_REG=0x%x\n",REG32(SYSREG_PLL_CTL32_63_REG));
//	printf("SYSREG_SYSCLK= 0x%x\n",REG32(SYSREG_SYSCLK_CONTROL_REG));
//	printf("slow bit REG=0x%x\n",REG32(0xb8000388));
	return;
}
void ft_pll(void) {
	ft_enable_uart();
	ft_pll_setup();
	//LDO CHECK
	RL6518_DRAM_LDO_OUTPUT_VOLTAGE_TEST();	
	ft_disable_uart();
	return;
}
REG_INIT_FUNC(ft_pll, 14);
