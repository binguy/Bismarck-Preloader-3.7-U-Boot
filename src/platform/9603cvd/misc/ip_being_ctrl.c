#include <util.h>

SECTION_RECYCLE
void ctrl_ip_existence(void) {
	IP_EN_CTRLrv = (0 | (1<<1)); //Keep GMAC0
	NEW_IP_EN_CTRLrv = (0 | (1<<4)); //Keep SPI NAND
	return;
}
REG_INIT_FUNC(ctrl_ip_existence, 2);
