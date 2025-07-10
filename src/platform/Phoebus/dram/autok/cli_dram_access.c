#include <soc.h>
#include <cli/cli_util.h>
#include <cli/cli_access.h>
#include <dram/autok/memctl_func.h>

#ifndef SECTION_CLI_ACCESS
#	define SECTION_CLI_ACCESS
#endif

#ifndef SECTION_CLI_UTIL
#	define SECTION_CLI_UTIL
#endif

extern void dram_setup(void);

SECTION_CLI_ACCESS cli_cmd_ret_t
cli_ddr_setup(const void *user, u32_t argc, const char *argv[]) {
	dram_setup();
	return CCR_OK;
}

SECTION_CLI_UTIL cli_cmd_ret_t
cli_ddr_window_scan1(const void *user, u32_t argc, const char *argv[]) {
	DRAM_param_restore(1);              //0:store  1:restore

	puts("Write_Leveling start!!\n");
	Write_Leveling(0);          //0:use default  1:use write leveling value

	DRAM_param_restore(0);
	return CCR_OK;
}

SECTION_CLI_UTIL cli_cmd_ret_t
cli_ddr_window_scan2(const void *user, u32_t argc, const char *argv[]) {
	DRAM_param_restore(1);              //0:store  1:restore

	puts("Scan CA TAP start!!\n");
	PI_Scan(1);

	DRAM_param_restore(0);
	return CCR_OK;
}

SECTION_CLI_UTIL cli_cmd_ret_t
cli_ddr_window_scan3(const void *user, u32_t argc, const char *argv[]) {
	DRAM_param_restore(1);              //0:store  1:restore

	puts("Scan CK/DQS TAP start!!\n");
	PI_Scan3();

	DRAM_param_restore(0);
	return CCR_OK;
}

SECTION_CLI_UTIL cli_cmd_ret_t
cli_ddr_window_scan4(const void *user, u32_t argc, const char *argv[]) {

	DRAM_param_restore(1);              //0:store  1:restore

	puts("DQ VS DQS start!!\n");
	PI_Scan(4);

	DRAM_param_restore(0);
	return CCR_OK;
}

SECTION_CLI_UTIL cli_cmd_ret_t
cli_ddr_window_scan5(const void *user, u32_t argc, const char *argv[]) {
	DRAM_param_restore(1);              //0:store  1:restore
	puts("DQ VS DQS start!!\n");
	PI_Scan(5);

	DRAM_param_restore(0);
	return CCR_OK;
}

SECTION_CLI_UTIL cli_cmd_ret_t
cli_ddr_window_scan6(const void *user, u32_t argc, const char *argv[]) {
	DRAM_param_restore(1);              //0:store  1:restore

	puts("AC_Scan start!!\n");
	AC_Scan();

	DRAM_param_restore(0);
	return CCR_OK;
}

extern cli_cmd_ret_t cli_std_call(const void *user, u32_t argc, const char *argv[]);

cli_top_node(ddr_win, cli_std_call);
cli_add_node(scan1, ddr_win, cli_ddr_window_scan1);
cli_add_help(scan1, "ddr_win scan1");

cli_add_node(scan2, ddr_win, cli_ddr_window_scan2);
cli_add_help(scan2, "ddr_win scan2");

cli_add_node(scan3, ddr_win, cli_ddr_window_scan3);
cli_add_help(scan3, "ddr_win scan3");

cli_add_node(scan4, ddr_win, cli_ddr_window_scan4);
cli_add_help(scan4, "ddr_win scan4");

cli_add_node(scan5, ddr_win, cli_ddr_window_scan5);
cli_add_help(scan5, "ddr_win scan5");

cli_add_node(scan6, ddr_win, cli_ddr_window_scan6);
cli_add_help(scan6, "ddr_win scan6");

static void dram_special_test(u32_t start_addr, u32_t len, u32_t loop) {
	/**********************************************************
	 The special test is provided by ESMT Kitty on 2019/01/19.
	**********************************************************/

	u8_t *start_char, data_char=0xFF;
	u32_t *start_word, i, start_addr_tmp, addr_value;
	u32_t addr_cycle, background_bit=0xFFFFFFFF, prog_loop, error_flag=0, loop_cnt=0;

	/* "addr_cycle" is column size from 0 to max */
	if((REG32(0xb8001004)&0xFFFF0000) == 0x11210000){
		addr_cycle=0x400;	//0x400 for 1K column(A0~A8)
		printf("Change 0/1 bit every 1K. (Column is A0~A8)\n\r");
	}else{
		addr_cycle=0x800;	//0x800 for 2K column(A0~A9)
		printf("Change 0/1 bit every 2K. (Column is A0~A9)\n\r");
	}

	while(loop--){
		loop_cnt++;
		for (prog_loop=0; prog_loop<2;prog_loop++){
			//1.Write background with “0”
			printf("                               \rloop %d...write background\r",loop_cnt);
			start_word = (u32_t *)(start_addr);
			for (i=0; i<len; i+=4) {
				*start_word = background_bit;
				//printf("step1: address(0x%x) = 0x%x \n\r",start_word,*start_word);
				start_word++;
			}
			writeback_invalidate_dcache_all();

			//2.Write X=0, Y=0 ~ max with data “1”. X=1, Y=0 ~max with data “0” loop X to max
			printf("                               \rloop %d...write data\r",loop_cnt);
			start_addr_tmp = start_addr;
			start_char = (u8_t *)(start_addr);
			for (i=0; i<len; i++) {
				if((start_addr_tmp % (addr_cycle*2))<addr_cycle){
					*start_char = data_char;
				}else{
					*start_char = ~data_char;
				}
				//printf("step2: address(0x%x) = 0x%x \n\r",start_char,*start_char);
				start_char++;
				start_addr_tmp++;
			}
			writeback_invalidate_dcache_all();

			//3.Read X=0 , Y=0 with data and write invert data ,loop Y to max. Repeat loop X to max
			//check data
			printf("                               \rloop %d...read/check data\r",loop_cnt);
			start_word = (u32_t *)(start_addr);
			start_addr_tmp = start_addr;
			for (i=0; i<len; i+=4) {
				addr_value = *start_word;
				if((start_addr_tmp % (addr_cycle*2))<addr_cycle){
					//printf("step3:(<0x800)start_word=(0x%x) start_addr_tmp=(0x%x)\n\r",start_word,start_addr_tmp);
					if(addr_value != (data_char|(data_char<<8)|(data_char<<16)|(data_char<<24))){
						printf("ERROR: address(0x%x) 0x%x != 0xFFFFFFFF\n",start_word,addr_value);
						error_flag = 1;
					}
				}else{
					//printf("step3:(>0x800)start_word=(0x%x) start_addr_tmp=(0x%x)\n\r",start_word,start_addr_tmp);
					if(addr_value != ~(data_char|(data_char<<8)|(data_char<<16)|(data_char<<24))){
						printf("ERROR: address(0x%x) 0x%x != 0x00000000\n",start_word,addr_value);
						error_flag = 1;
					}
				}
				start_word++;
				start_addr_tmp+=4;
			}
			//wirte invert data
			printf("                               \rloop %d...write invert data\r",loop_cnt);			
			start_char = (u8_t *)(start_addr);
			start_addr_tmp = start_addr;
			for (i=0; i<len; i++) {
				if((start_addr_tmp % (addr_cycle*2))<addr_cycle){
					*start_char = ~data_char;
				}else{
					*start_char = data_char;
				}
				//printf("step3: address(0x%x) = 0x%x \n\r",start_char,*start_char);
				start_char++;
				start_addr_tmp++;
			}
			writeback_invalidate_dcache_all();
			//4. read all array data
			printf("                               \rloop %d...read/check invert data\r",loop_cnt);
			start_word = (unsigned int *)(start_addr);
			start_addr_tmp = start_addr;
			for (i=0; i<len; i+=4) {
				addr_value = *start_word;
				if((start_addr_tmp % (addr_cycle*2))<addr_cycle){
					//printf("step4:(<0x800)start_word=(0x%x) start_addr_tmp=(0x%x)\n\r",start_word,start_addr_tmp);
					if(addr_value != ~(data_char|(data_char<<8)|(data_char<<16)|(data_char<<24))){
						printf("ERROR: address(0x%x) 0x%x != 0x%x\n",start_word,addr_value,~(data_char|(data_char<<8)|(data_char<<16)|(data_char<<24)));
						error_flag = 1;
					}
				}else{
					//printf("step4:(>0x800)start_word=(0x%x) start_addr_tmp=(0x%x)\n\r",start_word,start_addr_tmp);
					if(addr_value != (data_char|(data_char<<8)|(data_char<<16)|(data_char<<24))){
						printf("ERROR: address(0x%x) 0x%x != 0x%x\n",start_word,addr_value,(data_char|(data_char<<8)|(data_char<<16)|(data_char<<24)));
						error_flag = 1;
					}
				}
				start_word++;
				start_addr_tmp+=4;
			}

			//5. change background with invert data , repeat 1~4
			data_char = ~data_char;				//invert data
			background_bit = ~background_bit;	//invert background
		}


		if(error_flag==1){
			printf("                               \rloop %d...FAIL!!!\n",loop_cnt);
		}else{
			printf("                               \rloop %d...PASS\n",loop_cnt);
		}
		error_flag = 0;		//reset error_flag
	}

	return; 
}

SECTION_CLI_UTIL
static cli_cmd_ret_t cli_special_test(const void *user, u32_t argc, const char *argv[]) {
	u32_t start_addr, len, loop;

	if (argc != 4) {
		return CCR_INCOMPLETE_CMD;
	}

	start_addr = atoi(argv[1]);
	len = atoi(argv[2]);
	loop = atoi(argv[3]);

	puts("=== DRAM special test ===\n");

	dram_special_test(start_addr, len, loop);

	return CCR_OK;
}

cli_top_node(dram_spcl_test, cli_special_test);
cli_add_help(dram_spcl_test, "dram_spcl_test [start address] [len] [loop], A SPECIAL DRAM test");
