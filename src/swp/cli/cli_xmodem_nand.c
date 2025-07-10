/*	
 * Copyright 2001-2010 Georges Menie (www.menie.org)
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the University of California, Berkeley nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* this code needs standard functions memcpy() and memset()
   and input/output functions _inbyte() and _outbyte().

   the prototypes of the input/output functions are:
     int _inbyte(unsigned short timeout); // msec timeout
     void _outbyte(int c);

 */
#include <cli/crc16.c>
#include <cli/cli_xmodem_update.h>
#include <cli/cli_util.h>
#include <cli/cli_access.h>

#ifdef OTTO_FLASH_NOR
    #error "Error: SPI NOR Flash can not use this function"
#endif

#ifndef SECTION_CLI_UTIL
    #define SECTION_CLI_UTIL
#endif

u8_t SECTION_CLI_XMODEM_DATA bbi_table[8]={0}; 

#define SOH  0x01
#define STX  0x02
#define EOT  0x04
#define ACK  0x06
#define NAK  0x15
#define CAN  0x18
#define CTRLZ 0x1A

#define DLY_1S 1000
#define MAXRETRANS 25

SECTION_CLI_XMODEM_TEXT
int _inbyte(int to_ms) {
	while (!tstc()) {
		udelay(1000);
		if ((--to_ms) == 0) {
			return -2;
		}
	};
	return *((volatile u8_t *)UART_BASE_ADDR);
}

SECTION_CLI_XMODEM_TEXT 
static void _outbyte(u8_t c) {
	putc(c);
	return;
}

SECTION_CLI_XMODEM_TEXT 
static int check(int crc, const unsigned char *buf, int sz) {
	if (crc) {
		unsigned short crc = crc16_ccitt(buf, sz);
		unsigned short tcrc = (buf[sz]<<8)+buf[sz+1];
		if (crc == tcrc)
			return 1;
	}else {
		int i;
		unsigned char cks = 0;
		for (i = 0; i < sz; ++i) {
			cks += buf[i];
		}
		if (cks == buf[sz])
		return 1;
	}

	return 0;
}

SECTION_CLI_XMODEM_TEXT 
static void flushinput(void)
{
	while (_inbyte(((DLY_1S)*3)>>1) >= 0);
}

SECTION_CLI_XMODEM_TEXT
static int cli_xmodemReceive(u32_t flash_dest, int destsz, u32_t page_size_with_spare)
{
	unsigned char xbuff[1030]; /* 1024 for XModem 1k + 3 head chars + 2 crc + nul */
	unsigned char *p;
	int bufsz, crc = 0;
	unsigned char trychar = 'C';
	unsigned char packetno = 1;
	int i, c, len = 0;
	int retry, retrans = MAXRETRANS;
    u32_t bytes_in_buf=0;    
   
	for(;;) {
		for( retry = 0; retry < 16; ++retry) {
			if (trychar) _outbyte(trychar);
			if ((c = _inbyte((DLY_1S)<<1)) >= 0) {
				switch (c) {
				case SOH:
					bufsz = 128;
					goto start_recv;
				case STX:
					bufsz = 1024;
					goto start_recv;
				case EOT:
					flushinput();
					_outbyte(ACK);
					return len; /* normal end */
				case CAN:
					if ((c = _inbyte(DLY_1S)) == CAN) {
						flushinput();
						_outbyte(ACK);
						return -1; /* canceled by remote */
					}
					break;
				default:
					break;
				}
			}
		}
		if (trychar == 'C') { trychar = NAK; continue; }
		flushinput();
		_outbyte(CAN);
		_outbyte(CAN);
		_outbyte(CAN);
		return -2; /* sync error */

	start_recv:
		if (trychar == 'C') crc = 1;
		trychar = 0;
		p = xbuff;
		*p++ = c;
		for (i = 0;  i < (bufsz+(crc?1:0)+3); ++i) {
			if ((c = _inbyte(DLY_1S)) < 0) goto reject;
			*p++ = c;
		}

		if (xbuff[1] == (unsigned char)(~xbuff[2]) && 
			(xbuff[1] == packetno || xbuff[1] == (unsigned char)packetno-1) &&
			check(crc, &xbuff[3], bufsz)) {
			if (xbuff[1] == packetno)	{
				register int count = destsz - len;
				if (count > bufsz) count = bufsz;
				if (count > 0) {
                    inline_memcpy((w_buf_v+bytes_in_buf), &xbuff[3], count);
                    bytes_in_buf+=count;
                        
                    if(bytes_in_buf >= page_size_with_spare){
                        while(bbi_table[((u32_t)flash_dest>>6)]) flash_dest++;

                        cli_flash_info->_model_info->_page_write(cli_flash_info, w_buf_v, (u32_t)flash_dest);
                        cli_flash_info->_model_info->_page_read_ecc(cli_flash_info, r_buf_v, (u32_t)flash_dest, ecc_buf);
                        
                        for(i=0 ; i<(page_size_with_spare/4); i++){
                            if(*(w_buf_u32+i) != *(r_buf_u32+i)) return -100;
                        }

                        bytes_in_buf -= page_size_with_spare;
                        flash_dest++;
                        inline_memcpy((w_buf_v), (w_buf_v+page_size_with_spare), bytes_in_buf);                        
                    }
                    len += count;
				}
				++packetno;
				retrans = MAXRETRANS+1;
			}
			if (--retrans <= 0) {
				flushinput();
				_outbyte(CAN);
				_outbyte(CAN);
				_outbyte(CAN);
				return -3; /* too many retry error */
			}
			_outbyte(ACK);
			continue;
		}
	reject:
		flushinput();
		_outbyte(NAK);
	}
}


#define BLOCK_PAGE_ADDR(bid, pid) ((0x00<<16)|(bid<<6)|(pid))

static char _erf_msg[] SECTION_SDATA = {"Erase fail\n"};
static char _bbi_msg[] SECTION_SDATA = {"BBI\n"};
static char _ebw_msg[] SECTION_SDATA = {"WW: blk:%d =>"};

SECTION_CLI_XMODEM_TEXT
int erase_blocks(u32_t page_size_with_spare)
{    
    u32_t bid, bad_cnt=0;
    u32_t page_size = (page_size_with_spare>2112)?PAGE_SIZE_4K:PAGE_SIZE_2K;
    
    for(bid=0 ; bid<=MAX_ERASE_BLK_INDEX; bid++){
        s32_t ret = cli_flash_info->_model_info->_block_erase(cli_flash_info, BLOCK_PAGE_ADDR(bid,0));
        if(ret != 0){
            bad_cnt++;
            bbi_table[bid] = 1;
            printf(_ebw_msg,bid);puts(_erf_msg);
            continue;
        }

        cli_flash_info->_model_info->_page_read(cli_flash_info, r_buf_v, BLOCK_PAGE_ADDR(bid,0));   
        if(0xFFFFFFFF != *(r_buf_u32+(page_size/4))){
            bad_cnt++;
            bbi_table[bid] = 1;
            printf(_ebw_msg,bid);puts(_bbi_msg);
            continue;
        }        
    }
    if(bad_cnt == (MAX_ERASE_BLK_INDEX+1)) return -1;
    return 0;
}

static char _start_msg[] SECTION_SDATA = {"II: Sending loader through XMODEM ...\n"};
static char _erase_msg[] SECTION_SDATA = {"EE: Erase Fail. Exist PLR-CLI.\n"};
static char _pass_msg[] SECTION_SDATA = {"II: Transmission succeeds, %d bytes.\nII: Reset soon ...\n"};
static char _fail_msg[] SECTION_SDATA = {"EE: Transmission fails, status: %d.\nWW: Reset may FAIL\n"};

UTIL_FAR SECTION_CLI_XMODEM_TEXT
void cli_xmodem_2_nand(u32_t esz_Byte, u32_t page_size_with_spare)
{
    if(erase_blocks(page_size_with_spare) < 0){
        puts(_erase_msg);
        SWBREAK();
        return;
    }

    puts(_start_msg);
    u32_t start_addr = 0;
    int st = cli_xmodemReceive(start_addr, esz_Byte, page_size_with_spare);
    if(st < 0) printf(_fail_msg, st);
    else printf(_pass_msg, st);

    SYSTEM_RESET();    
    while(1);
}

SECTION_CLI_UTIL
void change_sp_to_sram(void)
{
    u32_t old_sp;
    asm volatile(
        "move %0, $29"
        : "=r"(old_sp)
        : 
        : "$29"
     );    
    inline_memcpy(NEW_STACK_AT_SRAM, old_sp, (OTTO_PLR_STACK_DEF-old_sp));
    asm volatile(
        "move $29, %0"
        :
        : "r"(NEW_STACK_AT_SRAM) 
        : "$29"
    );
}

SECTION_CLI_UTIL cli_cmd_ret_t
cli_xmodem_update_nand_loader(const void *user, u32_t argc, const char *argv[])
{
    // Command: "xmodem"
#if defined (OTTO_FLASH_SPI_NAND)
    u32_t page_size_with_spare = (SNAF_PAGE_SIZE(cli_flash_info)+SNAF_SPARE_SIZE(cli_flash_info));
    u32_t Byte_per_blk = page_size_with_spare*SNAF_NUM_OF_PAGE_PER_BLK(cli_flash_info);
#else
    u32_t page_size_with_spare = (ONFI_PAGE_SIZE(cli_flash_info)+ONFI_SPARE_SIZE(cli_flash_info));
    u32_t Byte_per_blk = page_size_with_spare*ONFI_NUM_OF_PAGE_PER_BLK(cli_flash_info);
#endif

    u32_t esz_Byte = Byte_per_blk*(MAX_ERASE_BLK_INDEX+1);
    change_sp_to_sram();
    cli_xmodem_2_nand(esz_Byte, page_size_with_spare);

    while(1); //Should not run to here
    return CCR_OK;
}

cli_top_node(xmodem, cli_xmodem_update_nand_loader);
    cli_add_help(xmodem, "xmodem: Command for updating NAND Preloader iamge");

