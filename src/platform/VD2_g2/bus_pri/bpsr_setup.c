#include <soc.h>
#include <bus_pri/bpsr_reg_map.h>
#include <dram/memcntlr_reg.h>

void bpsr_register_init(void)
{
	return;
}

void bpsr_change_bus_priority(void)
{
    return;
}
REG_INIT_FUNC(bpsr_register_init, 16);
REG_INIT_FUNC(bpsr_change_bus_priority, 17);

