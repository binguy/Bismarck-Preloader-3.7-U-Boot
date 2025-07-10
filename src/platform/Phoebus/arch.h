#define _lplr_soc_t (*(soc_t*)(OTTO_LPLR_ENTRY+OTTO_HEADER_OFFSET))
#define _lplr_bios (_lplr_soc_t.bios)
#define _lplr_header (_lplr_bios.header)

#define _soc_cid_sct ((_soc.cid>>16)&0xFFFF)
#define _soc_cid_series (_soc.cid&0xFFFF)

/* SID for APro, APro Gen2, RTL9603C-VD */
#define PLR_SID_APRO       0x7
#define PLR_SID_APRO_GEN2  0x1
#define PLR_SID_9603CVD    0x8

/* sram init*/
#define SRAM_CTRL_INIT         \
sram_init:\
	li  t0, 0xB8001300; \
	lui t1, %hi(OTTO_SRAM_START); \
	lui t3, 0x1FFF; \
	and t1, t1, t3; \
	ori t1, t1, 0x1; /*Set (Un)mapping enable bit*/ \
	li  t3, 0xB8004000; \
	li  t2, 9; \
	sw  t1, 0(t3); /*mapping*/ \
	sw  t2, 4(t3); /*mapping*/ \
	lui t3, 0x1FFF; \
	and t1, t1, t3; \
	ori t1, t1, 0x1; /*Set (Un)mapping enable bit*/ \
	sw  t1, 0(t0); /*unmapping*/ \
	sw  t2, 4(t0); /*unmapping size of default sram controller setting*/

/* FUNC - declare global function */
#define GFUNC(symbol)														\
	.text;																				\
	.globl symbol;																\
	.align 4;																			\
	.ent symbol;																	\
symbol:

/* END - mark end of function */
#define END(symbol)															\
	.end symbol


#define SYSTEM_RESET() do { \
		REG32(0xb8003268) = 0; \
		REG32(0xb8003268) = 0x80000000; \
	} while (0)
