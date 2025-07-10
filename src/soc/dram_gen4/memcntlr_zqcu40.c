#include <soc.h>
#include <dram/memcntlr.h>

MEMCNTLR_SECTION
u32_t zq_calibration(void) {
    u32_t plimit = 0x10000;
    u32_t ddzqpsr;

    RMOD_DDZQPCR(zctrl_start, 1);

    while (RFLD_DDZQPCR(zctrl_start) == 1) {
        plimit--;
        if (plimit == 0) { return MEM_CNTLR_ZQ_TIMEOUT;}
    }

    ddzqpsr = DDZQPSRrv;
    if (ddzqpsr & 0x20000000) {
        u32_t odtp, odtn, ocdp, ocdn;
        odtp = ((ddzqpsr >> 27) & 0x3);
        odtn = ((ddzqpsr >> 25) & 0x3);
        ocdp = ((ddzqpsr >> 23) & 0x3);
        ocdn = ((ddzqpsr >> 21) & 0x3);
        if ((odtp != 0) || /* ODTP must be completed with no error */
            (odtn == 2) || /* ODTN must NOT be underflow (may tolerate code overflow error) */
            (ocdp == 1) || /* OCDP must NOT be overflow (may tolerate code underflow error) */
            (ocdn == 1))   /* OCDN must NOT be overflow (may tolerate code underflow error) */
        {   return MEM_CNTLR_ZQ_ERROR; }
        else return MEM_CNTLR_ZQ_RELAXED_OK; 
    }
    return MEM_CNTLR_ZQ_STRICT_OK;
}
