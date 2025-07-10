#include <soc.h>
#define FAIL 1
#define PASS 0
unsigned char RL6518B_UTOPIA_GPIO_TEST(void){
	unsigned int result,REG_6D4,REG_228,REG_04C,clock_cnt,count_value;
//	unsigned int clock_cnt,count_value;
//	unsigned int result;
	REG_6D4=REG32(0xb80006D4);
	REG_228=REG32(0xb8000228);
	REG_04C=REG32(0xb800004C);
	if(((REG32(0xb8000100)>>24)&0x3) !=0 ){
		puts("must pull down GPE2(bond enable).\n\r");
		return FAIL;
	}
#if 1
	//REG32(0xb80006D4) = (REG32(0xb80006D4)&(~(3<<28))) |(2<<28);
	REG32(0xb80006D4)|= (3<<28);
	REG32(0xb8000228)|= (1<<12);
	REG32(0xb800004C)|= (3<<16);
	mdelay(1);

	// Read GPIO_E7 status
	/* Start */
		clock_cnt=100;
		REG32(0xb8003230)= 0x0FFFFFFF;
		REG32(0xb8003238)= 0x11000002;	//counter enable, 100MHz
		//delay_msec(1);
		while(((REG32(0xb8003328)>>7)&0x1)==0){
			if(REG32(0xb8003234)>0x08fffff)
					return 10;
				}		//rising edge	
		while((REG32(0xb8003328)>>7)&0x1){
			if(REG32(0xb8003234)>0x08fffff)
					return 11;
				}		//falling edge
		count_value=REG32(0xb8003234);
		do{
			while((REG32(0xb8003328)>>7)&0x1){
			if(REG32(0xb8003234)>0x08fffff)
					return 12;
				}		//falling edge		
			while(((REG32(0xb8003328)>>7)&0x1)==0){
			if(REG32(0xb8003234)>0x08fffff)
					return 13;
				}		//rising edge	
		}while(clock_cnt--);	
		count_value=REG32(0xb8003234)-count_value;	
		REG32(0xb8003238)= 0;	//counter disable	
#if 0
		if(count_value>max)
			max=count_value;
		if(count_value<min)
			min=count_value;	
		printf("TC2 value=0x%x (%d) max(%d) min(%d)\n\r",count_value,count_value,max,min);	
#else
		//if(count_value>7000 && count_value<10500)		
		if(count_value<6000){		
		//printf("UTOPIA_clock_fail\n\r");
		return 15;
			}
#endif

	REG32(0xb80006D4)=REG_6D4;
	REG32(0xb8000228)=REG_228;
	REG32(0xb800004C)=REG_04C;
#endif
	// GPIO loop back test
	result=0;
	REG32(0xb8003308)=0;
	REG32(0xb8003324)=0x00f0c000;
	REG32(0xb8003328)=0x00504000;
	if(((REG32(0xb800330c)>>30)&0x3)!=0x1)
		result|=1;
	if(((REG32(0xb8003328)>>20)&0xff)!=0x55)
		result|=1<<1;
	REG32(0xb8003328)=0x00a08000;
	if(((REG32(0xb800330c)>>30)&0x3)!=0x2)
		result|=1<<2;
	if(((REG32(0xb8003328)>>20)&0xff)!=0xaa)
		result|=1<<3;	
	REG32(0xb8003324) = 0;
	if(result){
		return FAIL;
	}else{
		return PASS;
	}}
