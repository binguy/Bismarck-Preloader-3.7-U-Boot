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

#ifndef OTTO_FLASH_NOR
    #error "Error: This is only for SPI NOR Flash"
#endif

#ifndef SECTION_CLI_UTIL
    #define SECTION_CLI_UTIL
#endif

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
int xm_tstc(void)
{
    return ((*((volatile u32_t *)(UART_BASE_ADDR+0x14)) & 0x8F000000) == 0x01000000);
}

SECTION_CLI_XMODEM_TEXT 
void xm_putc (char c)
{
	while((*((volatile u32_t *)(UART_BASE_ADDR+0x14)) & 0x20000000)  == 0);
	*((volatile u8_t *)UART_BASE_ADDR) = c;
}

#define xm_putcn(chr) ({char ch=chr; if (ch=='\n') xm_putc('\r'); xm_putc(ch);})
#define xm_puts(s) ({const char *p=s; char c; while ((c=*(p++))!='\0') xm_putcn(c);})

SECTION_CLI_XMODEM_TEXT
int _inbyte(int to_ms) {
	while (!xm_tstc()) {
		udelay(1000);
		if ((--to_ms) == 0) {
			return -2;
		}
	};
	return *((volatile u8_t *)UART_BASE_ADDR);
}

SECTION_CLI_XMODEM_TEXT 
static void _outbyte(u8_t c) {
	xm_putc(c);
	return;
}

SECTION_CLI_XMODEM_TEXT
static int check(int crc, const u8_t *buf, int sz) {
	if (crc) {
		unsigned short crc = crc16_ccitt(buf, sz);
		unsigned short tcrc = (buf[sz]<<8)+buf[sz+1];
		if (crc == tcrc)
			return 1;
	} else {
		int i;
		u8_t cks = 0;
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
static int cli_xmodemReceive(u32_t flash_dest)
{
	u8_t xbuff[1030]; /* 1024 for XModem 1k + 3 head chars + 2 crc + nul */
	u8_t *p;
	int bufsz, crc = 0;
	u8_t trychar = 'C';
	u8_t packetno = 1;
	int i, c, len = 0;
	int retry, retrans = MAXRETRANS;

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

		if ((xbuff[1] == (u8_t)(~xbuff[2])) &&
		    (xbuff[1] == packetno || xbuff[1] == (u8_t)packetno-1) &&
		    check(crc, &xbuff[3], bufsz)) {
			if (xbuff[1] == packetno)	{
				register int count = bufsz;

				/* Just-in-time erase. */
				if (count > 0) {
					len += count;
					NORSF_PROG(flash_dest, count, &xbuff[3], 0);
					flash_dest += count;
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




static char _start_msg[] SECTION_SDATA = {"II: Sending loader through XMODEM ...\n"};
static char _pass_msg[] SECTION_SDATA = {"II: Transmission succeeds.\nII: Reset soon ...\n"};
static char _fail_msg[] SECTION_SDATA = {"EE: Transmission fails.\nWW: Reset may fail.\n"};

UTIL_FAR SECTION_CLI_XMODEM_TEXT
void cli_xmodem_2_sf(u32_t dest_addr, u32_t esz_KB)
{
    u32_t erase=dest_addr;
    u32_t i;
    for(i=0; i<(esz_KB/ERASE_UNIT_B); i++){
        NORSF_ERASE(erase, ERASE_UNIT_B, 0, 0);
        erase += ERASE_UNIT_B;
    }

    xm_puts(_start_msg);
    int st = cli_xmodemReceive(dest_addr);

    if(st<0) xm_puts(_fail_msg);
    else xm_puts(_pass_msg);
    
    SYSTEM_RESET();    
    while(1); //Print is bad, wait for reset.
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
cli_xmodem_update_nor_loader(const void *user, u32_t argc, const char *argv[])
{
    // Command: "xmodem"
    u32_t dest_addr = 0;
    u32_t esz_KB = MAX_ERASE_BYTE;
    change_sp_to_sram();
    printf("II: NOR Driver should be in SRAM!\n");
    cli_xmodem_2_sf(dest_addr, esz_KB);

    while(1); //Should not run to here, because SPI NOR flash had been erased and updated.
    return CCR_OK;
}

cli_top_node(xmodem, cli_xmodem_update_nor_loader);
    cli_add_help(xmodem, "xmodem: Command for updating SPI NOR Preloader iamge");    

