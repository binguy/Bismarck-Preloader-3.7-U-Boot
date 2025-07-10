#include <soc.h>
#define MP_OFFSET 0x0
UTIL_FAR void ft_enable_uart(void);
UTIL_FAR void ft_disable_uart(void);
extern unsigned int  memctlc_bondwire_check(void);
extern unsigned char dram_bitmap_scan(unsigned start_address,unsigned end_address,unsigned block_length,unsigned char setup_flag);
extern unsigned int memctlc_err_map;
void ft_entry(void) {
	ft_enable_uart();
	puts("\n\n");
	if(memctlc_err_map == 1) puts("F_ZQ\n");
	else puts("P_ZQ\n");
	if(dram_bitmap_scan(0+MP_OFFSET,memctlc_bondwire_check(),0x1000,1))
		puts("P_DTOGGLE\n");
	else
		puts("F_DTOGGLE\n");
	ft_disable_uart();
	return;
}
REG_INIT_FUNC(ft_entry, 31);
