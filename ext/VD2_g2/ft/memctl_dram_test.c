/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2003  
* All rights reserved.
* 
* Abstract: 
* 		SDRAM bond wire and bit toggle check source code.
*		Kevin.Chung@realtek.com
* ---------------------------------------------------------------
*/
#include <soc.h>
#include "mips_cache_ops.h"
#define DCR 0xb8001004
unsigned int memctlc_bondwire_check(void)
{
	volatile unsigned int *dcr;
	unsigned int *addr;
	unsigned int temp,scan_map;
	unsigned char row,col,bank,bus_width;
	dcr = (volatile unsigned int *)DCR;
	scan_map=0;
	
	temp=*dcr;
	col = ((temp>>16)&0x0f)+8;				// 0000 => A0~A7
	row = ((temp>>20)&0x0f)+11;				// 0000 => A0~A10
	bus_width = (temp>>24)&0x03; 
	bank = ((temp>>28)&0x03)+1;				// 0000 => BA0
//	printf("DRAM size = 0x%08x\n",1<<(col+row+bus_width+bank));
	
	for(temp=col;temp < col+row+bank;temp++){
		addr = (unsigned int *)(0xa0000000|(0x1<<(temp+bus_width)));               // first row start bit, col + 1
		*addr = temp;
		*(addr+1)= ~(temp);
	}

	//check
	for(temp=col;temp < col+row+bank;temp++){
		addr = (unsigned int *)(0xa0000000|(0x1<<(temp+bus_width)));               // first row start bit, col + 1
		if(*addr !=temp){
				scan_map|=(1<<temp);
		}
		if(*(addr+1) !=(~(temp))){
				scan_map|=(1<<temp);
		}
	}
	if(scan_map){
			puts("F_DRAM_BONDWIRE.\n\r");				
	}else{
			puts("P_DRAM_BONDWIRE.\n\r");
	}   
	return 1<<(col+row+bus_width+bank);	
}

#if 1	//	CONFIG_DRAM_AUTO_TIMING_SETTING
unsigned char	dram_bitmap_scan(unsigned start_address,unsigned end_address,unsigned block_length,unsigned char setup_flag)
{
#define force_checksum 1
#if 1
	volatile unsigned int *GDMA0_config_base, *GDMA1_config_base, *GDMA3_config_base;
	unsigned int bitmap_scan_addr_index,GDMA_active_times,GDMA_active_times_total;
	unsigned int GDMA0_current_addr=0,GDMA1_current_addr=0,GDMA3_current_addr=0;
	unsigned char status_flag; //n0: GDMA0 done, n1: GDMA1 done, n2: GDMA3 done
	unsigned int check_sum_temp=0;
#endif	
	unsigned char pattern_cnt;
	//unsigned int write_pattern[]={0x12345678,0xEDCBA987,0xFFFFFFFF,0,0x5a5aa5a5,0xa5a55a5a};
	//unsigned int write_pattern[]={0x12345678,0xEDCBA987};
	unsigned int write_pattern[]={0x5a5aa5a5,0xa5a55a5a};
#ifdef force_checksum
	check_sum_temp=0x3fffc00;		//for write_pattern[]={0x5a5aa5a5,0xa5a55a5a}
#endif
	unsigned int *CPU_read_address;
	unsigned int read_length,current_addr=0;
	CPU_read_address = (unsigned int *) (0x80000000 |start_address);
	read_length = (end_address-start_address)/4;

#if 1
	GDMA0_config_base = (volatile unsigned int *)0xB8144000;
	GDMA1_config_base = (volatile unsigned int *)0xB800A000;
	GDMA3_config_base = (volatile unsigned int *)0xB8018000;
#endif
#if 0
	if(end_address>0x10000000)
		end_address=0x10000000;
	printf("bitmap scan range 0x%x ~ 0x%x\n",start_address,end_address);
#endif
#if 1
	GDMA_active_times_total=(end_address-start_address)/block_length;
	if(setup_flag){
		REG32(0xb8000600)|=(0x3<<8);		//enable lxbus 1 & 0 GDMA
		REG32(0xb800063c)|=(0x1<<7);		//enable lxbus 3 GDMA

		// GDMA config
		*(GDMA0_config_base)=0x0;			// GDMA control Register
		*(GDMA1_config_base)=0x0;			// GDMA control Register
		*(GDMA3_config_base)=0x0;			// GDMA control Register
		*(GDMA0_config_base+1)=0x80000000;	// 0x4, GDMA Interrupt Mask Register
		*(GDMA1_config_base+1)=0x80000000;	// 0x4, GDMA Interrupt Mask Register
		*(GDMA3_config_base+1)=0x80000000;	// 0x4, GDMA Interrupt Mask Register
		*(GDMA0_config_base+2)=0x88800000;	// 0x8, GDMA Interrupt Status Register
		*(GDMA1_config_base+2)=0x88800000;	// 0x8, GDMA Interrupt Status Register
		*(GDMA3_config_base+2)=0x88800000;	// 0x8, GDMA Interrupt Status Register
		*(GDMA0_config_base+0x9)=0x80000000 | block_length; 	// 0x24, Set destination desc length
		*(GDMA1_config_base+0x9)=0x80000000 | block_length; 	// 0x24, Set destination desc length	
		*(GDMA3_config_base+0x9)=0x80000000 | block_length; 	// 0x24, Set destination desc length
		*(GDMA0_config_base+0x19)=0x80000000 | block_length; // 0x64, Set destination desc length
		*(GDMA1_config_base+0x19)=0x80000000 | block_length; // 0x64, Set destination desc length
		*(GDMA3_config_base+0x19)=0x80000000 | block_length; // 0x64, Set destination desc length
		}
#endif
#if 1
	for(pattern_cnt=0;pattern_cnt< (sizeof(write_pattern)/sizeof(unsigned int));pattern_cnt++){
		for(current_addr=0;current_addr<read_length;current_addr++)
			*(CPU_read_address+current_addr)=write_pattern[pattern_cnt];
		_1004K_L1_DCache_flush();
#endif		
#if 0
	for(pattern_cnt=0;pattern_cnt< (sizeof(write_pattern)/sizeof(unsigned int));pattern_cnt++){
		bitmap_scan_addr_index = start_address;
		GDMA_active_times=GDMA_active_times_total;
		status_flag=0;		//n0: 0:GDMA0 done, n1: 0:GDMA1 done, n2: 0:GDMA3 done
		
		// first run GDMA0 for get checksum pattern.	
		GDMA0_current_addr = bitmap_scan_addr_index;
		*(GDMA0_config_base+0x18)=GDMA0_current_addr;		// 0x60
		*(GDMA0_config_base+0x3)=write_pattern[pattern_cnt]; // 0xc, Initial checksum value
		*(GDMA0_config_base)=0xC30000A0;				

		// first run GDMA1
		bitmap_scan_addr_index+=block_length;
		GDMA1_current_addr = bitmap_scan_addr_index;
		*(GDMA1_config_base+0x18)=GDMA1_current_addr;		// 0x60
		*(GDMA1_config_base+0x3)=write_pattern[pattern_cnt]; // 0xc, Initial checksum value
		*(GDMA1_config_base)=0xC30000A0;
		
		// first run GDMA3
		bitmap_scan_addr_index+=block_length;
		GDMA3_current_addr = bitmap_scan_addr_index;
		*(GDMA3_config_base+0x18)=GDMA3_current_addr;		// 0x60
		*(GDMA3_config_base+0x3)=write_pattern[pattern_cnt]; // 0xc, Initial checksum value
		*(GDMA3_config_base)=0xC30000A0;

		// start DRAM bitmap scan
		do{
			//printf("GDMA_active_times=0x%x, half_done = %d\n",GDMA_active_times,half_done);
			if(*(GDMA0_config_base+0x2)){		//0x8, check interrupt flag
				*(GDMA0_config_base+0x2)=0x88800000;	//0x8, clean interrupt flag
				if(GDMA_active_times==0){
					status_flag|=1;
				}else{
					GDMA_active_times--;
					bitmap_scan_addr_index+=block_length;
					GDMA0_current_addr=bitmap_scan_addr_index;
					*(GDMA0_config_base+0x3)=write_pattern[pattern_cnt]; //0xc, Initial checksum value
					*(GDMA0_config_base+0x18)=GDMA0_current_addr; // 0x60
					*(GDMA0_config_base)=0xC30000A0;
				}
				//printf("GDMA_times=%d,GDMA0_current_addr=0x%x\n",GDMA_times,GDMA0_current_addr);
			}
			if(*(GDMA1_config_base+0x2)){		//0x8, check interrupt flag
				*(GDMA1_config_base+0x2)=0x88800000;	//0x8, clean interrupt flag
				if(GDMA_active_times==0){
					status_flag|=2;
				}else{
					GDMA_active_times--;
					bitmap_scan_addr_index+=block_length;
					GDMA1_current_addr=bitmap_scan_addr_index;
					*(GDMA1_config_base+0x3)=write_pattern[pattern_cnt]; //0xc, Initial checksum value
					*(GDMA1_config_base+0x18)=GDMA1_current_addr; // 0x60
					*(GDMA1_config_base)=0xC30000A0;
				}
			}
			if(*(GDMA3_config_base+0x2)){		//0x8, check interrupt flag
				*(GDMA3_config_base+0x2)=0x88800000;	//0x8, clean interrupt flag
				if(GDMA_active_times==0){
					status_flag|=4;
				}else{
					GDMA_active_times--;
					bitmap_scan_addr_index+=block_length;
					GDMA3_current_addr=bitmap_scan_addr_index;
					*(GDMA3_config_base+0x3)=write_pattern[pattern_cnt]; //0xc, Initial checksum value
					*(GDMA3_config_base+0x18)=GDMA3_current_addr; // 0x60
					*(GDMA3_config_base)=0xC30000A0;
				}
			}			
			//printf("SET_GDMA_active_times=0x%x,status_flag=%d\n",GDMA_active_times,status_flag);
#if 0			
#ifdef CPU_read_check	
			if(*(CPU_read_address+current_addr)!=write_pattern[pattern_cnt]){
				printf("addr(0x%x):0x%x != pattern(0x%x).\n\r",(current_addr*4)|0xa0000000,*(CPU_read_address+current_addr),write_pattern[pattern_cnt]);
				return 0;
			}else{
				current_addr++;
				if(current_addr>read_length){
					status_flag|=0x8;
					current_addr=0;
					}
			}
#endif				
#endif
		}while((status_flag&0x07)!=0x7);		//n0: 0:GDMA0 done, n1: 0:GDMA1 done, n2: 0:GDMA3 done
		//}while((status_flag&0x0F)!=0x0F);		//n0: 0:GDMA0 done, n1: 0:GDMA1 done, n2: 0:GDMA3 done	
#endif		
#if	0
		//printf("current_addr=0x%x, read_length=0x%x, write_pattern=0x%x \n",current_addr,read_length,write_pattern[pattern_cnt]);
		for(current_addr=0;current_addr<=read_length;current_addr++)
			if(*(CPU_read_address+current_addr)!=write_pattern[pattern_cnt]){
				printf("addr(0x%x):0x%x != pattern(0x%x).\n\r",(current_addr*4)|0x80000000,*(CPU_read_address+current_addr),write_pattern[pattern_cnt]);
				return 0;
			}
#endif			
#if 1
		bitmap_scan_addr_index = start_address;
		GDMA_active_times=GDMA_active_times_total;
		status_flag=0;		//n0: GDMA0 done, n1: GDMA1 done, n2: GDMA3 done
		
		*(GDMA0_config_base+0x3)=0; //0xc, checksum initial					
		*(GDMA0_config_base+0x8)=bitmap_scan_addr_index; //0x20					
		*(GDMA0_config_base)=0xC10000A0;
#ifndef force_checksum
		while(*(GDMA0_config_base+0x2)==0);
		*(GDMA0_config_base+0x2)=0x88800000;	//0x8, clean interrupt flag
		check_sum_temp = (*(GDMA0_config_base+0x3));
		//printf("check_sum_temp=0x%x \n",check_sum_temp);
		GDMA_active_times--;			
		
		bitmap_scan_addr_index+=block_length;
		*(GDMA0_config_base+0x3)=0; //0xc, checksum initial					
		*(GDMA0_config_base+0x8)=bitmap_scan_addr_index; //0x20					
		*(GDMA0_config_base)=0xC10000A0;
#endif		
		GDMA_active_times--;

		bitmap_scan_addr_index+=block_length;
		*(GDMA1_config_base+0x3)=0; //0xc, checksum initial					
		*(GDMA1_config_base+0x8)=bitmap_scan_addr_index; //0x20					
		*(GDMA1_config_base)=0xC10000A0;
		GDMA_active_times--;

		bitmap_scan_addr_index+=block_length;
		*(GDMA3_config_base+0x3)=0; //0xc, checksum initial					
		*(GDMA3_config_base+0x8)=bitmap_scan_addr_index; //0x20					
		*(GDMA3_config_base)=0xC10000A0;		
		GDMA_active_times--;

		do{
			//printf("GDMA_active_times=0x%x, half_done = %d\n",GDMA_active_times,half_done);
			if(*(GDMA0_config_base+0x2)){		//0x8, check interrupt flag
				*(GDMA0_config_base+0x2)=0x88800000;	//0x8, clean interrupt flag
				if((*(GDMA0_config_base+0x3))!= check_sum_temp){
					printf("bit fail0...0x%x (0x%x),%d,0x%x\n",*(GDMA0_config_base+0x3),check_sum_temp,pattern_cnt,GDMA_active_times);
					printf("GDMA0_current_addr 0x%x=(0x%x) \n",GDMA0_current_addr |0x80000000,REG32(GDMA0_current_addr|0xa0000000));
					return 0;
				}
				if(GDMA_active_times==0){
					status_flag|=1;
				}else{
					GDMA_active_times--;
					bitmap_scan_addr_index+=block_length;
					GDMA0_current_addr=bitmap_scan_addr_index;
					//if(GDMA_active_times<10)
						//printf("GDMA0_current_addr=0x%x \n",GDMA0_current_addr);
					*(GDMA0_config_base+0x3)=0; //0xc, checksum initial					
					*(GDMA0_config_base+0x8)=GDMA0_current_addr; //0x20					
					*(GDMA0_config_base)=0xC10000A0;					
				}
			}
			if(*(GDMA1_config_base+0x2)){		//0x8, check interrupt flag
				*(GDMA1_config_base+0x2)=0x88800000;	//0x8, clean interrupt flag
				if((*(GDMA1_config_base+0x3))!= check_sum_temp){
					printf("bit fail1...0x%x (0x%x),%d,0x%x\n",*(GDMA1_config_base+0x3),check_sum_temp,pattern_cnt,GDMA_active_times);
					printf("GDMA1_current_addr 0x%x=(0x%x) \n",GDMA1_current_addr |0x80000000,REG32(GDMA1_current_addr|0xa0000000));
					return 0;
				}				
				if(GDMA_active_times==0){
					status_flag|=2;
				}else{
					GDMA_active_times--;
					bitmap_scan_addr_index+=block_length;
					GDMA1_current_addr=bitmap_scan_addr_index;
					//if(GDMA_active_times<10)
						//printf("GDMA1_current_addr=0x%x \n",GDMA1_current_addr);
					*(GDMA1_config_base+0x3)=0; //0xc, checksum initial 				
					*(GDMA1_config_base+0x8)=GDMA1_current_addr; //0x20 				
					*(GDMA1_config_base)=0xC10000A0;		

				}
			}
			if(*(GDMA3_config_base+0x2)){		//0x8, check interrupt flag
				*(GDMA3_config_base+0x2)=0x88800000;	//0x8, clean interrupt flag
				if((*(GDMA3_config_base+0x3))!= check_sum_temp){
					printf("bit fail3...0x%x (0x%x),%d,0x%x\n",*(GDMA3_config_base+0x3),check_sum_temp,pattern_cnt,GDMA_active_times);
					printf("GDMA3_current_addr 0x%x=(0x%x) \n",GDMA3_current_addr |0x80000000,REG32(GDMA3_current_addr|0xa0000000));
					return 0;
					}
				if(GDMA_active_times==0){
					status_flag|=4;
				}else{
					GDMA_active_times--;
					bitmap_scan_addr_index+=block_length;
					GDMA3_current_addr=bitmap_scan_addr_index;
					//if(GDMA_active_times<10)
						//printf("GDMA3_current_addr=0x%x \n",GDMA3_current_addr);
					*(GDMA3_config_base+0x3)=0; //0xc, checksum initial 				
					*(GDMA3_config_base+0x8)=GDMA3_current_addr; //0x20 				
					*(GDMA3_config_base)=0xC10000A0;		
				}
			}
			//printf("SUM_GDMA_active_times=0x%x,status_flag=%d\n",GDMA_active_times,status_flag);
		}while((status_flag&7)!=7);		//n0: 0:GDMA0 done, n1: 0:GDMA1 done, n2: 0:GDMA3 done		
#endif
		//printf("pattern_cnt=%d,block_length=0x%x\n",pattern_cnt,block_length);
#if	0				
			for(i=start_address;i<end_address;){
				if(REG32(i|0xa0000000)!=write_pattern[pattern_cnt])
					printf("CPU check pattern fail...addr 0x%x = 0x%x (0x%x)\n",i,REG32(i|0x80000000),write_pattern[pattern_cnt]);
				i+=4;
				}
#endif			
	}
	return 1;		//printf("Pass...\n");
}
/*
unsigned char dram_bitmap_scan_all(unsigned start_address,unsigned end_address)
{
#define MSRRrv		REG32(0xB8001038)
#define DOR0rv		REG32(0xB8004200)
#define FLUSH_OCP_CMD() 			({ MSRRrv=0x80000000; while(0x40000000!=MSRRrv);})
#define ZONE0_OFFSET_CONFIG(off)	({ DOR0rv=(off); })
	unsigned char test_times,test_fail=0;

	puts("DRAM capacity=0x");puthex(end_address);

	if(end_address<=0x10000000){
		return dram_bitmap_scan(0,end_address,0x1000,1);
	}
	//test_times=2^(((end_address>>28)&3)-1);
	//test_times=((end_address>>28)&0x0F)-1;		// each test block 2Gbit.
	test_times=((end_address>>28)&0x0F)-1;		// each test block 2Gbit.
	//initial 
	if(dram_bitmap_scan(0,0x10000000,0x1000,1)==0){
		//printf("0xb8004200=0x%x, test_times=%d\n",REG32(0xb8004200),test_times);
		return 0;	
		}
	for(;test_times>0;test_times--){
		FLUSH_OCP_CMD();
		ZONE0_OFFSET_CONFIG(end_address-0x10000000);		//set zone start address.
		FLUSH_OCP_CMD();
		//printf("0xb8004200=0x%x, test_times=%d\n",REG32(0xb8004200),test_times);
		if(end_address==0x20000000){							//  between 2G to 4Gbit must jump SRAM mapping only for NAND flash.
			if(dram_bitmap_scan(0,0x0F000000,0x1000,0)){
				if(dram_bitmap_scan(0x0F008000,0x10000000,0x1000,0)==0){
					test_fail++;
				}
			}else
				test_fail++;
		}else{
			if(dram_bitmap_scan(0,0x10000000,0x1000,0)==0){
				test_fail++;
			}
		}
		if(test_fail){
			FLUSH_OCP_CMD();
			ZONE0_OFFSET_CONFIG(0);
			FLUSH_OCP_CMD();
			puts("fail range address=0x");puthex(end_address);
			return 0;
			}
		end_address-=0x10000000;
	}
	FLUSH_OCP_CMD();
	ZONE0_OFFSET_CONFIG(0);
	FLUSH_OCP_CMD();	
	return 1;

}
*/

#endif
