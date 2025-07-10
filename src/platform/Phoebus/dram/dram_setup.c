#include <init_define.h>
#include <uart/uart.h>

#include <misc/misc_setting.h>

#define puthex(hex) printf("%x", hex)
#include <dram/memcntlr_reg.h>
#include <dram/memcntlr_util.h>
#include <dram/autok/autoconf.h>
#include <dram/autok/board_mem_diag.c>
#include <dram/autok/mem_plat_setting.c>
#include <dram/autok/memctl.c>
#include <dram/autok/mips_cache_ops.c>
#include <dram/autok/memctl_cali_dram.c>
#include <dram/autok/memctl_utils.c>


void memctlc_init_dram(void); // at ./ext/xxxx/autok/memctl.c

static void dram_calibration_entry(void) {
	puts("II: DRAM calibration starts...\r");
	memctlc_init_dram();
}

void dram_setup(void) {
    dram_calibration_entry();

#ifdef STICKY_FREQ_XLAT
    volatile uint32 *dtr0 = (uint32 *)DTR0;
    volatile uint32 *dmcr = (uint32 *)DMCR;
    uint32 val = *dtr0;

    if(REG32(0xb800121c) == 3){
        if (_soc.sid == PLR_SID_APRO) { //DDR3, clock=600
            val = ((*dtr0 & ~(0xff << 4)) | (0x84 << 4));
        } else if(_soc.sid == PLR_SID_APRO_GEN2) {  //DDR3, clock=666
            val = ((*dtr0 & ~(0xff << 4)) | (0x94 << 4));
        }

        /* Set refresh time to 7.68us for MCM model */
        *dtr0 = val;

        /* DMCR update */
	    udelay(1);
        *dmcr = *dmcr;
        while(*dmcr & DMCR_MRS_BUSY);
        udelay(1);
    } else if (REG32(0xb800121c) == 4) {
        if(_soc.sid == PLR_SID_APRO_GEN2) {  //DDR3, clock=750
            /* Set refresh time to 7.68us for MCM model */
            *dtr0 = 0xBB75543A;
        }

        /* DMCR update */
        udelay(1);
        *dmcr = *dmcr;
        while(*dmcr & DMCR_MRS_BUSY);
        udelay(1);
    }
#endif

	ISTAT_SET(cal, MEM_CAL_OK);
}
REG_INIT_FUNC(dram_setup, 28);
