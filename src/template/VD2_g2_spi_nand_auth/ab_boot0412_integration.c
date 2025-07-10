#include <util.h>
#include <autoconf.h>

#define puthex(h) printf("%x", h)

#include <boot0412/board_mem_diag.c>

/* Adapted from CMU_setup() @ boot0412/memctl.c */
unsigned int board_CPU_freq_mhz(const unsigned int cmu_ctrl_reg __attribute__((unused))) {
	unsigned int OCP0_freq;

	if (REG32(SYSREG_PLL_CTL32_63_REG) & (1 <<18)) {
		OCP0_freq = ((((REG32(SYSREG_SYSCLK_CONTROL_REG) >> OCP0_PLL_DIV_S) & 0x3f) + 2) * 25);	
	}
	else {
		OCP0_freq = ((((REG32(SYSREG_SYSCLK_CONTROL_REG) >> OCP0_PLL_DIV_S) & 0x3f) + 2) * 50);	
	}

	return OCP0_freq;
}
