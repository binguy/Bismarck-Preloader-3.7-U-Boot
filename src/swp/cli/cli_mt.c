#include <soc.h>
#include <cli/cli_access.h>
#include <soc_exam_util/mt_ram_test.h>


#ifndef SECTION_CLI_UTIL
    #define SECTION_CLI_UTIL
#endif

typedef enum {
    MT_RAM_TC_ALL   = 0,
    MT_RAM_TC_SIMPLE_NORMAL = 1,
    MT_RAM_TC_UNALIGNMENT= 2,
} mt_ram_test_case;


int __attribute__ ((weak))
mt_highmem_test(u32_t max_size) {
    puts("WW: missing mt_highmem_test\n");
    return MT_FAIL;
}

int __attribute__ ((weak))
mt_highmem_simple_normal_test(u32_t max_size) {
    puts("WW: missing mt_highmem_simple_normal_test\n");
    return MT_FAIL;
}


#if (OTTO_FLASH_SPI_NAND)
#include <soc_exam_util/spi_nand_memory_test.c>
#else
int __attribute__ ((weak))
spi_nand_flash_memory_test(u32_t addr, u32_t size, u32_t loops) {
    puts("WW: missing spi_nand_flash_memory_test\n");
    return MT_FAIL;
}
#endif


SECTION_CLI_UTIL cli_cmd_ret_t
cli_std_mt(const void *user, u32_t argc, const char *argv[])
{
    u32_t total_test_runs=1;
    mt_ram_test_case mrtc = MT_RAM_TC_ALL;
        
    // "mt ram <start addr> <test size> [-l/-loops <test loops>] [-simple_normal_case/-snc || -unalignment_case/-uac]"
    if(argc<4) return CCR_INCOMPLETE_CMD;
    else{
        u32_t i;
        for (i=4; i<argc; i++){
            if((cli_strcmp(argv[i], "-loops", ' ') == 0) || (cli_strcmp(argv[i], "-l", ' ') == 0)){
                if((i+1) >= argc) return CCR_INCOMPLETE_CMD;
                total_test_runs=atoi(argv[++i]);
            }else if((cli_strcmp(argv[i], "-simple_normal_case", ' ') == 0) || (cli_strcmp(argv[i], "-snc", ' ') == 0)){
                mrtc = MT_RAM_TC_SIMPLE_NORMAL;
            }else if((cli_strcmp(argv[i], "-unalignment_case", ' ') == 0) || (cli_strcmp(argv[i], "-uac", ' ') == 0)){
                mrtc = MT_RAM_TC_UNALIGNMENT;
            }
        }
    }
    u32_t addr=atoi(argv[2]);
    u32_t size=atoi(argv[3]);
    
    if (size > 0x10000000) {
        EPRINTF("size should be NOT large than 256MB\n");
        return CCR_FAIL; 
    }
    
    printf("II: mt ram test\n    addr:0x%08x, size:0x%x, loop:%d \n", addr, size, total_test_runs);
    
    u32_t loop;
    for(loop=1 ; loop<=total_test_runs ; loop++){
        printf("II: #%d runs\n", loop);

        switch(mrtc){
            case MT_RAM_TC_SIMPLE_NORMAL:
                if(MT_SUCCESS!=mt_ram_simple_normal_test(addr, size)) return CCR_FAIL; 
                break;
            case MT_RAM_TC_UNALIGNMENT:
                if(MT_SUCCESS!=mt_ram_unaligned_wr(addr, size)) return CCR_FAIL; 
                break;
            case MT_RAM_TC_ALL:
            default:
                if(MT_SUCCESS!=mt_ram_test(addr, size)) return CCR_FAIL;
                break;
        }
    }
    return CCR_OK;
}
extern void Write_Leveling(unsigned char apply);

SECTION_CLI_UTIL cli_cmd_ret_t
cli_write_leveling(const void *user, u32_t argc, const char *argv[])
{
	if(atoi(argv[1])==0)
	    printf("restore pi value\n");
	else
		printf("apply pi value\n");
    Write_Leveling(atoi(argv[1]));
	return CCR_OK;
}

extern void PI_Scan(unsigned char PI);

SECTION_CLI_UTIL cli_cmd_ret_t
cli_PI_Scan(const void *user, u32_t argc, const char *argv[])
{

	switch(atoi(argv[1])){
		case 0: 
			printf("scan CK PI\n"); break;
		case 1: 
			printf("scan CA PI\n"); break;
		case 2: 
			printf("scan DQS0 PI\n"); break;
		case 3: 
			printf("scan DQS1 PI\n"); break;
		case 4: 
			printf("scan DQL PI\n"); break;
		case 5: 
			printf("scan DQH PI\n"); break;
	}
	PI_Scan(atoi(argv[1]));
	return CCR_OK;
}

/*
extern void PI_Scan3(void);

SECTION_CLI_UTIL cli_cmd_ret_t
cli_PI_Scan3(const void *user, u32_t argc, const char *argv[])
{
	printf("scan CK, DQS and DQ PI at same time\n");

    PI_Scan3();
	return CCR_OK;
}
*/
__attribute__((weak))
unsigned int get_ddr_size(void)
{
    u32_t dcr = REG32(0xB8001004);
    u8_t bank = ((dcr>>28)&0x3)+1;
    u8_t dbw  = (dcr>>24)&0x3;
    u8_t row  = ((dcr>>20)&0xF)+11;
    u8_t col  = ((dcr>>16)&0xF)+8;
    u32_t size = 1 << (bank+dbw+row+col);
    return size;
}

SECTION_CLI_UTIL cli_cmd_ret_t
cli_highmem_mt(const void *user, u32_t argc, const char *argv[])
{
    u32_t total_test_runs=1;
    mt_ram_test_case mrtc = MT_RAM_TC_ALL;

    // "mt highmem [-loops|-l <test loops>] [-simple_normal_case/-snc]"
    if(argc<2) return CCR_INCOMPLETE_CMD;
    else{
        u32_t i;
        for (i=2; i<argc; i++){
            if((cli_strcmp(argv[i], "-loops", ' ') == 0) || (cli_strcmp(argv[i], "-l", ' ') == 0)){
                if((i+1) >= argc) return CCR_INCOMPLETE_CMD;
                total_test_runs=atoi(argv[++i]);
            }else if((cli_strcmp(argv[i], "-simple_normal_case", ' ') == 0) || (cli_strcmp(argv[i], "-snc", ' ') == 0)){
                mrtc = MT_RAM_TC_SIMPLE_NORMAL;
            }
        }
    }

    u32_t max_size=get_ddr_size();
    
    if (max_size <= SIZE_256MB) {
        printf("II: need more than 256MB DRAM size (max_size=0x%x)\n", max_size);
        return CCR_FAIL;
    }
    
    printf("II: mt high memory test\n    DDR size: %dMB, loop:%d \n",
            max_size>>20, total_test_runs);
    
    u32_t loop;
    for(loop=0 ; loop<total_test_runs ; loop++){
        printf("II: #%d runs\n", loop+1);
        switch(mrtc){
            case MT_RAM_TC_SIMPLE_NORMAL:
                if(MT_SUCCESS!=mt_highmem_simple_normal_test(max_size)) return CCR_FAIL;
                break;
                
            case MT_RAM_TC_ALL:
            default:
               if(MT_SUCCESS!=mt_highmem_test(max_size)) return CCR_FAIL;
               break;
        }
    }
    return CCR_OK;
}

SECTION_CLI_UTIL cli_cmd_ret_t
cli_spi_nand_mt(const void *user, u32_t argc, const char *argv[])
{
    // "mt spi_nand <start block> <end block> [test loops]"
    if(argc<4) return CCR_INCOMPLETE_CMD;

    u32_t blk_start = atoi(argv[2]);
    u32_t blk_cnt   = atoi(argv[3]);
    u32_t loops=1;
    if(5==argc) loops = atoi(argv[4]);
    
    spi_nand_flash_memory_test(blk_start, blk_cnt, loops);
    return CCR_OK;
}

cli_top_node(mt, VZERO);
    cli_add_node(ram, mt, (cli_cmd_func_t *)cli_std_mt);
    cli_add_help(ram, "mt ram <start addr> <test size> [-l/-loops <test loops>] [-simple_normal_case/-snc || -unalignment_case/-uac]");
    cli_add_node(highmem, mt, (cli_cmd_func_t *)cli_highmem_mt);
    cli_add_help(ram, "mt highmem [-loops|-l <test loops>] [-simple_normal_case/-snc]");
    cli_add_node(spi_nand, mt, (cli_cmd_func_t *)cli_spi_nand_mt);
    cli_add_help(spi_nand, "mt spi_nand <start block> <blks to test> [test loops]");

cli_top_node(wl, cli_write_leveling);
    cli_add_help(wl, "wl <=1 apply, =0 restore>");

cli_top_node(scan_pi, cli_PI_Scan);
    cli_add_help(scan_pi, "scan_pi <PI#>");
/*
cli_top_node(scan_pi3, cli_PI_Scan3);
    cli_add_help(scan_pi3, "scan_pi3 <PI#>");
*/
