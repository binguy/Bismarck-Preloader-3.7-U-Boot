#include <init_define.h>
#include <autoconf.h>

#ifdef printf
#undef printf
#endif

extern void puthex(int h);

void _1004K_L1_DCache_flush(void);

#include <boot0412/memctl.c>
