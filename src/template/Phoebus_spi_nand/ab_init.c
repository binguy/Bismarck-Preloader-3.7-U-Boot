#include <soc.h>
#include <cpu/mmu_drv.h>
#include <spi_nand/spi_nand_util.h>

/* Including tlzma.h to automatically enable LZMA */
#include <lib/lzma/tlzma.h>

const char _mmu_init_fail_msg[] SECTION_RECYCLE_DATA = {"EE: no physical page was added, system stops\n"};
const char _nsu_table_init_fail_msg[] SECTION_RECYCLE_DATA = {"EE: plr mapping table init fail\n"};
const char _nsu_probe_fail_msg[] SECTION_RECYCLE_DATA = {"EE: plr probe nand flash device fail\n"};

u32_t util_ms_accumulator SECTION_SDATA = 0;
char *_lplr_vv SECTION_SDATA;
char *_lplr_bd SECTION_SDATA;
char *_lplr_tk SECTION_SDATA;

UTIL_FAR SECTION_UNS_TEXT void
__sprintf_putc(const char c, void *user) {
	//if (c=='\r') return;
	char **p=(char**)user;
	**p=c;
	++(*p);
}

/* ********************* */
/* init. level 3 starts */
#define STACK_GUARD 0xcafebeef

#define FUNCPTR_RETURN_ZERO(func) func SECTION_SDATA=(void*)ALWAYS_RETURN_ZERO

FUNCPTR_RETURN_ZERO(proto_printf_t *_proto_printf);
FUNCPTR_RETURN_ZERO(lx_timer_init_t *_lx_timer_init);
FUNCPTR_RETURN_ZERO(udelay_t *_udelay);
FUNCPTR_RETURN_ZERO(mdelay_t *_mdelay);
FUNCPTR_RETURN_ZERO(get_timer_t *_get_timer);
FUNCPTR_RETURN_ZERO(unsigned int (*_atoi)(const char *v));
FUNCPTR_RETURN_ZERO(char (*_strcpy)(char *dst, const char *src));
FUNCPTR_RETURN_ZERO(u32_t (*_strlen)(const char *s));
FUNCPTR_RETURN_ZERO(int (*_strcmp)(const char *s1, const char *s2));
FUNCPTR_RETURN_ZERO(char (*_memcpy)(void *dst, const void *src, unsigned int len));
FUNCPTR_RETURN_ZERO(char (*_mass_copy)(void *dst, const void *src, unsigned int len));
FUNCPTR_RETURN_ZERO(char (*_memset)(void *dst, char value, unsigned int len));

const symb_retrive_entry_t plr_init_utility_retrive_list[] SECTION_RECYCLE_DATA = {
	{SF_PROTO_PRINTF, &_proto_printf},
	{SF_SYS_LX_TIMER_INIT, &_lx_timer_init},
	{SF_SYS_UDELAY, &_udelay},
	{SF_SYS_MDELAY, &_mdelay},
	{SF_SYS_GET_TIMER, &_get_timer},
	{SCID_STR_ATOI, &_atoi},
	{SCID_STR_STRCPY, &_strcpy},
	{SCID_STR_STRLEN, &_strlen},
	{SCID_STR_STRCMP, &_strcmp},
	{SCID_STR_MEMCPY, &_memcpy},
	{SCID_STR_MEMSET, &_memset},
	{SCID_STR_MASSCPY, &_mass_copy},
	{ENV_VCS_VERSION, &_lplr_vv},
	{ENV_BUILD_DATE, &_lplr_bd},
	{ENV_BUILD_KIT, &_lplr_tk},
	{ENDING_SYMB_ID, VZERO}
};

SECTION_RECYCLE void
plr_funcptr_init(void) {
	// 2. put stack guard words
	extern u32_t farthest_stack_position;
	u32_t *cur_sp;
	__asm__ __volatile__  ("addiu %0, $29, -4": "=r"(cur_sp));
	u32_t *sp_end=&farthest_stack_position;
	while (cur_sp != sp_end)
		*(cur_sp--)=STACK_GUARD;

	// 3. binding
	symb_retrive_list(plr_init_utility_retrive_list, lplr_symb_list_range);

#ifdef HAS_LIB_LZMA
	symb_retrive_and_set(lplr, SF_LIB_LZMA_DECODE, _lzma_decode);
#endif

	return;
}
REG_INIT_FUNC(plr_funcptr_init, 2);
/* init. level 2 ends */
/* ********************* */




/* ********************* */
/* init. level 5 starts */
SECTION_RECYCLE static int
init_spi_nand_probe_both_list(void) {
	extern const spi_nand_probe_t *start_of_spi_nand_probe_func;
	extern const spi_nand_probe_t *end_of_spi_nand_probe_func;

	if (nsu_probe(&plr_spi_nand_flash_info, &start_of_spi_nand_probe_func, &end_of_spi_nand_probe_func)==0) return 0;

	const symbol_table_entry_t *s;
	if ((s=symb_retrive_lplr(SPI_NAND_DEVICE_PROB_LIST_START))==VZERO) return -1;
	const spi_nand_probe_t **lplr_probe_func_start=s->v.pvalue;

	if ((s=symb_retrive_lplr(SPI_NAND_DEVICE_PROB_LIST_END))==VZERO) return -1;
	const spi_nand_probe_t **lplr_probe_func_end=s->v.pvalue;

	return nsu_probe(&plr_spi_nand_flash_info, lplr_probe_func_start, lplr_probe_func_end);
}

SECTION_RECYCLE void
init_spi_nand(void) {
	if (init_spi_nand_probe_both_list()<0) {
		puts(_nsu_probe_fail_msg);
		while(1);
	}

	// basic spi-nand init and establish the plr_mapping_table
	if (nsu_init()<0) {
		puts(_nsu_table_init_fail_msg);
		while(1);
	}
}
REG_INIT_FUNC(init_spi_nand, 5);
/* init. level 5 ends */
/* ****************** */


/* ********************* */
/* init. level 10 starts */
#ifdef MMU_CHECK_INETRNAL_ERROR
SECTION_TEXT UTIL_FAR static void
show_second_banner(void)  {
	inline_puts("    .text and .ro sections work!\n");
}
#endif

SECTION_RECYCLE static void
_init_tlb(void) {
	// 1. clean up mmu driver
	u32_t end_of_mapping = OTTO_SRAM_START + OTTO_SRAM_SIZE;
	if (mmu_drv_init((u32_t)&mapped_physical_sram_start, end_of_mapping)==0) {
		puts(_mmu_init_fail_msg);
		while(1);
	}

#if (OTTO_CPU_MIPS32R2 == 1)
	/* disable exl & exl for TLB handler */
	asm_mtc0(0x00400000, CP0_STATUS);
#elif (OTTO_CPU_RLXMIPS == 1)
#	error "EE: fix me! CP0_STATUS setting for TLB handler is to be confirmed!"
#endif

	return;
}

SECTION_UNS_TEXT void
init_tlb(void) {
	_init_tlb();
	// 3. show message
	puts("II: TLB initial done:\n    .ro section works!\n");
#ifdef MMU_CHECK_INETRNAL_ERROR
	show_second_banner();
#endif
}
REG_INIT_FUNC(init_tlb, 7);
/* init. level 10 ends */
/* ******************* */
