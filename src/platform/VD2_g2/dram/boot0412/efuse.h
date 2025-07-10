#ifndef _MEMCTL_EFUSE_H_
#define _MEMCTL_EFUSE_H_

#define EFUSE_ADDR 	0xb8000644
#define EFUSE_ADDR1 	0xb8000648

#define EFUSE_REG_OFFSET 	(16)
#define EFUSE_DATA_OFFSET 	(0)
#define DRAM_INFO_EFUSE_ADDR	(2)
#define CPU_INFO_EFUSE_ADDR	(2)
#define CPU_INFO_EFUSE_OFFSET 	(14)
#define CPU_INFO_EFUSE_BITMASK 	(3)
#define DRAM_LDO_EFUSE_ADDR 	(3)
#define GPHY_INFO_EFUSE_ADDR	(7)
#define GPHY_INFO_EFUSE_BITMASK	(7)
#define GPHY_INFO_EFUSE_PORT0_OFFSET 	(0)
#define GPHY_INFO_EFUSE_PORT1_OFFSET 	(3)
#define GPHY_INFO_EFUSE_PORT2_OFFSET 	(6)
#define GPHY_INFO_EFUSE_PORT3_OFFSET 	(9)
#define GPHY_INFO_EFUSE_PORT4_OFFSET 	(12)

#define GPHY_PORT_NUM 5

enum efuse_cpu_freq {
	cpuFreq_1G = 0,
	cpuFreq_900 = 1,
	cpuFreq_800 = 2,
	cpuFreq_750 = 3,
	
};

enum efuse_gphy_cap {
	gphyCap_GE = 0,
	gphyCap_FE = 1,
};

struct efuse_data {
	unsigned short cpuFreq;
	unsigned short ldo;
	unsigned short gphyCap[GPHY_PORT_NUM];
};

unsigned short memctl_EFUSE_ReadPhyMdio(unsigned char regaddr);
void memctl_EFUSE_SetPhyMdioWrite(unsigned char regaddr, unsigned short val);
void get_efuse_info(void);
unsigned short efuse_get_phy_cap(int port);
extern struct efuse_data efuse_info;
#endif
