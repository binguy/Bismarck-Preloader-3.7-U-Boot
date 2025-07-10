#include <soc.h>
#include <cg/cg.h>
#include <uart/uart.h>
#include <spi_nand/spi_nand_ctrl.h>
#include <nor_spi/nor_spif_register.h>
#include <dram/memcntlr_reg.h>

#define CG_EXP_CPU_WALK_MHZ (0)

#if defined(_MIPS_ARCH_5281)
#	define C0_COUNT_MUL 1
#elif defined(_MIPS_ARCH_MIPS32R2)
# define C0_COUNT_MUL 2
#else
#	error "EE: unknown CPU ARCH\n"
#endif


__attribute__ ((unused))
static void hang(char *msg) {
	puts(msg);
	while (1) {
		;
	}
	return;
}

static int round_div(const u32_t x, const u32_t y) {
	return ((x + (y/2)) / y);
}

static int roundup_div(const u32_t x, const u32_t y) {
	return (x + y - 1) / y;
}

static int _rtl9603cvd_cg_is_fpga() {
	return !!(cg_info_query.lx_mhz < 125);
}

static void _rtl9603cvd_cg_udelay(uint32_t us) {
	volatile int cnt = 0;

	while (cnt++ < us*500) ;

	return;
}

static void _rtl9603cvd_cg_enable_cache_miss_cnt(void) {
	/* [11:5] = event = 01101 = store/load misses */
	__asm_mtc0(0x800001bf, CP0_PERFCTL0, 0);
	__asm_mtc0(0x800001bf, CP0_PERFCTL0, 2);
	__asm_mtc0(0x0, CP0_PERFCTL0, 1);
	__asm_mtc0(0x0, CP0_PERFCTL0, 3);
	return;
}

static void _rtl9603cvd_cg_check_then_disable_cache_miss_cnt(uint32_t *smiss, uint32_t *lmiss) {
	*smiss = __asm_mfc0(CP0_PERFCTL0, 1);
	*lmiss = __asm_mfc0(CP0_PERFCTL0, 3);

	__asm_mtc0(0x80000000, CP0_PERFCTL0, 0);
	__asm_mtc0(0x80000000, CP0_PERFCTL0, 2);
	__asm_mtc0(0x0, CP0_PERFCTL0, 1);
	__asm_mtc0(0x0, CP0_PERFCTL0, 3);

	return;
}

static uint32_t _rtl9603cvd_cg_disable_bp(void) {
	uint32_t config7_bak;

	config7_bak = __asm_mfc0($16, 7);
	__asm_mtc0((config7_bak | 0xe), $16, 7);

	return config7_bak;
}

static void _rtl9603cvd_cg_restore_bp(uint32_t bak) {
	__asm_mtc0(bak, $16, 7);

	return;
}

/********************************
	LX configuration
********************************/
static int rtl9603cvd_cg_get_lx_mhz(cg_dev_freq_t *target) {
	int freq;
	int div;

	if (_rtl9603cvd_cg_is_fpga()) {
		/* there must be a value, since 0 is blocked during cg_config_lx_mhz() */
		freq = target->lx_mhz;
	} else {
		div = RFLD_PHY_RG5X_PLL(ck200m_lx_pll) + 5;
		freq = round_div(1000, div);
	}

	return freq;
}

static void rtl9603cvd_cg_set_lx_mhz(cg_dev_freq_t *target) {
	int div;
	if (_rtl9603cvd_cg_is_fpga()) {
		/* does nothing */
	} else {
		div = round_div(1000, target->lx_mhz);
		if (div < 5) div = 5;
		if (div > 8) div = 8;
		RMOD_PHY_RG5X_PLL(ck200m_lx_pll, div - 5);
	}

	return;
}

static int rtl9603cvd_cg_config_lx_mhz(cg_dev_freq_t *target) {
	if (target->lx_mhz) {
		rtl9603cvd_cg_set_lx_mhz(target);
#if defined(OTTO_PROJECT_FPGA)
	} else {
		hang("EE: unknown LX freq. on FPGA\n");
#endif
	}

	return rtl9603cvd_cg_get_lx_mhz(target);
}

/********************************
	DRAM configuration
********************************/
static int rtl9603cvd_cg_get_mem_mhz(const cg_dev_freq_t *target) {
	int freq;

	if (_rtl9603cvd_cg_is_fpga()) {
		/* there must be a value, since 0 is blocked during cg_config_lx_mhz() */
		freq = target->mem_mhz;
	} else {
		freq = ((RFLD(MEM_PLL_CTRL3, crt_n_code) + 3) * 25) / 2;
	}

	return freq;
}

static const mem_pll_info_t *_rtl9603cvd_cg_get_predefined_mem_pll_para(const int freq) {
	extern mem_pll_info_t rtl9603cvd_ddr2_pll[];
	extern mem_pll_info_t rtl9603cvd_ddr3_pll[];
	extern mem_pll_info_t rtl9603cvd_ddr2_mcm_pll[];
	extern mem_pll_info_t rtl9603cvd_ddr3_mcm_pll[];
	const mem_pll_info_t *pll_preset[2][2] = {
		{rtl9603cvd_ddr2_pll, rtl9603cvd_ddr2_mcm_pll},
		{rtl9603cvd_ddr3_pll, rtl9603cvd_ddr3_mcm_pll}
	};
	const mem_pll_info_t *pll_info;

	pll_info = pll_preset[_is_ddr3()][_is_mcm()];

	while (freq < pll_info->mhz) {
		pll_info++;
	}

	return pll_info;
}

static void rtl9603cvd_cg_set_mem_mhz(const cg_dev_freq_t *target) {
	const mem_pll_info_t *pll_info;
	MEM_PLL_CTRL3_T mem_pll_ctrl3;

	/* bit-by-bit toggling crt_n_code */
	u32_t tobe;
	u32_t cur_reg, temp, shift;
	u32_t min_ncode, max_ncode, next_cnt;
	u8_t next_time[8];

	if (_rtl9603cvd_cg_is_fpga()) {
		return;
	}

	pll_info = _rtl9603cvd_cg_get_predefined_mem_pll_para(target->mem_mhz);

	mem_pll_ctrl3.v = pll_info->pll3;

	/* apply setting */
	CG_MEM_PLL_OE_DIS();
	_rtl9603cvd_cg_udelay(200);

	RVAL(MEM_PLL_CTRL0) = pll_info->pll0;
	RVAL(MEM_PLL_CTRL1) = pll_info->pll1;
	RVAL(MEM_PLL_CTRL2) = pll_info->pll2;

	/* bit-by-bit toggling crt_n_code */
	tobe = mem_pll_ctrl3.f.crt_n_code;
	next_cnt = 0;
	min_ncode = 9;
	max_ncode = 85;

	for (shift=0; shift<8; shift++) {
		cur_reg = RFLD(MEM_PLL_CTRL3, crt_n_code);
		if (((cur_reg>>shift)&1) != ((tobe>>shift)&1)) {
			temp = (cur_reg & (~(1<<shift))) | ((!((cur_reg>>shift)&1))<<shift);
			if ((temp<min_ncode) || (temp>max_ncode)) {
				next_time[next_cnt++] = shift;
			} else {
				mem_pll_ctrl3.f.crt_n_code = temp;
				RVAL(MEM_PLL_CTRL3) = mem_pll_ctrl3.v;
			}
		}
	}

	while (next_cnt>0) {
		cur_reg = RFLD(MEM_PLL_CTRL3, crt_n_code);
		shift = next_time[--next_cnt];
		if ((cur_reg>>shift) != (tobe>>shift)) {
			temp = (cur_reg & (~(1<<shift))) | ((!((cur_reg>>shift)&1))<<shift);
			if ((temp<min_ncode) || (temp>max_ncode)) {
				printf("EE: Range error, temp=%d\n", temp);
			} else {
				mem_pll_ctrl3.f.crt_n_code = temp;
				RVAL(MEM_PLL_CTRL3) = mem_pll_ctrl3.v;
			}
		}
	}

	if (tobe != RFLD(MEM_PLL_CTRL3, crt_n_code)) {
		for (shift=0; shift<8; shift++) {
			cur_reg = RFLD(MEM_PLL_CTRL3, crt_n_code);
			if (((cur_reg>>shift)&1) != ((tobe>>shift)&1)) {
				temp = (cur_reg & (~(1<<shift))) | ((!((cur_reg>>shift)&1))<<shift);
				mem_pll_ctrl3.f.crt_n_code = temp;
				RVAL(MEM_PLL_CTRL3) = mem_pll_ctrl3.v;
			}
		}
	}

	_rtl9603cvd_cg_udelay(5);
	CG_MEM_PLL_OE_EN();
	_rtl9603cvd_cg_udelay(200);

	return;
}

static int rtl9603cvd_cg_config_mem_mhz(const cg_dev_freq_t *target) {
	if (target->mem_mhz) {
		rtl9603cvd_cg_set_mem_mhz(target);
#if defined(OTTO_PROJECT_FPGA)
	} else {
		hang("EE: unknown DRAM freq. on FPGA\n");
#endif
	}

	return rtl9603cvd_cg_get_mem_mhz(target);
}

/********************************
	SPI-F configuration
********************************/
typedef struct {
	u16_t max_mhz;
	u8_t div_start;
	u8_t div_end;
} spif_pll_para_t;
/* sizeof(SPIF_PLL_PARA_T) > 4; can't be passed as return value */
skc35_assert(sizeof(spif_pll_para_t) <= 4);

static spif_pll_para_t
rtl9603cvd_cg_get_spif_pll_para(const cg_dev_freq_t *target) {
	const char is_nor = !RFLD_MCR(boot_sel);

	/* source of SPI NOR/NAND PLL freq.
		 pll_max   N|        N&
		 FPGA      mem_mhz   lx_mhz
		 ASIC      1000      lx_mhz

		 allowed divisors of above PLL.
		 pll_div   N|        N&
		 FPGA      1         1
		 ASIC      4~7       1
	*/
	spif_pll_para_t pll = {
		.div_start = 1,
		.div_end = 1,
		.max_mhz = target->lx_mhz,
	};

	if (is_nor) {
		if (_rtl9603cvd_cg_is_fpga()) {
			pll.max_mhz = target->mem_mhz;
		} else {
			pll.max_mhz = 1000; //gphy = 1GHz
			pll.div_start = 4;
			pll.div_end = 7;
		}
	}

	return pll;
}

static void rtl9603cvd_cg_set_spif_mhz(cg_dev_freq_t *target) {
	const int exp_mhz = target->spif_mhz;

	int best_mhz, best_div;
	int spif_mhz, spif_div;
	int pll_mhz, pll_div;
	spif_pll_para_t pll;

	pll = rtl9603cvd_cg_get_spif_pll_para(target);

	best_mhz = 0;
	best_div = 10;

	for (pll_div=pll.div_start; pll_div<=pll.div_end; pll_div++) {
		pll_mhz = round_div(pll.max_mhz, pll_div);

		/* cntlr. allows div. to 16; but div. > 10 produces impractical low speed so ignored */
		for (spif_div=2; spif_div<=10; spif_div+=2) {
			spif_mhz = round_div(pll_mhz, spif_div);

			if ((spif_mhz > best_mhz) && (spif_mhz <= exp_mhz)) {
				best_mhz = spif_mhz;
				best_div = spif_div;
				if (best_mhz == exp_mhz) {
					goto set_pll_n_cntlr_div;
				}
			}
		}
	}
	pll_div--;

set_pll_n_cntlr_div:
	/* pll_div > 1 means it is NOR on ASIC. */
	if (pll_div > 1) {
		RMOD_PHY_RG5X_PLL(ck250m_spif_pll, (pll_div - pll.div_start));
		RMOD_SFRDR(
			io3_delay, 0xf,
			io2_delay, 0xf,
			io1_delay, 0xf,
			io0_delay, 0xf);
	}

	/* SNOF spi_clk_div will be set after U-Boot SNOF init. */
	RMOD_SNAFCFR(
		spi_clk_div, best_div/2-1,
		pipe_lat, 1);

	target->nor_pll_mhz = round_div(pll.max_mhz, pll_div);

	return;
}

static int rtl9603cvd_cg_get_spif_mhz(cg_dev_freq_t *target) {
	spif_pll_para_t pll;

	const char is_nor = !RFLD_MCR(boot_sel);
	uint32_t pll_mhz;
	uint32_t div;

	pll = rtl9603cvd_cg_get_spif_pll_para(target);
	div = 1;

	if (!_rtl9603cvd_cg_is_fpga()) {
		if (is_nor) {
			div = RFLD_PHY_RG5X_PLL(ck250m_spif_pll) + 4;
		} else {
			/* pll.max_mhz of SNAF comes from LX freq., which is already divided by lx_pll_div */
			/* div = RFLD(PHY_RG5X_PLL, ck200m_lx_pll_div) + 5; */
		}
	}
	pll_mhz = round_div(pll.max_mhz, div);

	/* cg_set_spif_mhz() put the same div to both cntlr. so just read one is enough. */
	return round_div(pll_mhz, (RFLD_SNAFCFR(spi_clk_div)+1)*2);
}

static int rtl9603cvd_cg_config_spif_mhz(cg_dev_freq_t *target) {
	if (target->spif_mhz) {
		rtl9603cvd_cg_set_spif_mhz(target);
	}

	return rtl9603cvd_cg_get_spif_mhz(target);
}

/********************************
	SRAM configuration
********************************/
static int rtl9603cvd_cg_get_sram_mhz(const cg_dev_freq_t *target) {
	int freq;

	if (_rtl9603cvd_cg_is_fpga()) {
		freq = target->mem_mhz;
	} else {
		freq = 400 + (RFLD(PLLCCR, reg_sel_500m) * 100);
		freq = freq / (2 - RFLD(SYSPLLCTR, sram_no_div2));
	}

	return freq;
}

static void rtl9603cvd_cg_set_sram_mhz(const cg_dev_freq_t *target) {
	int sram_no_div2;
	int sram_sel_500m;

	if (_rtl9603cvd_cg_is_fpga()) {
		return;
	}

	if ((target->sram_mhz == 200) ||
			(target->sram_mhz == 250) ||
			(target->sram_mhz == 400) ||
			(target->sram_mhz == 500)) {
		/* Allow 500, 400, 250, or 200.
			 200: 0_1100_1000
			 250: 0_1111_1010
			 400: 1_1001_0000
			 500: 1_1111_0100
			 bit8 for div2 or not; bit5 for 400M or 500M
		*/
		sram_no_div2 = (target->sram_mhz >> 8) & 1;
		sram_sel_500m = (target->sram_mhz >> 5) & 1;

		RMOD(SYSPLLCTR, sram_no_div2, sram_no_div2);
		RMOD(PLLCCR, reg_sel_500m, sram_sel_500m);
	}

	return;
}

static int rtl9603cvd_cg_config_sram_mhz(const cg_dev_freq_t *target) {
	if (target->sram_mhz) {
		rtl9603cvd_cg_set_sram_mhz(target);
	}

	return rtl9603cvd_cg_get_sram_mhz(target);
}

/********************************
	CPU configuration
********************************/
static int rtl9603cvd_cg_probe_cpu_mhz(void) {
	u32_t reg_backup[2];
	u32_t cpu_cnt, i;

	i=0;
	/* 1st iter. to fill cache; 2nd iter for actual measurement. */
	do {
		/* set timer 0 to 1MHz and count 0.1s */
		reg_backup[0] = REG32(0xb8003200);
		reg_backup[1] = REG32(0xb8003208);
		REG32(0xb8003208) = 0;
		REG32(0xb800320c) = (1 << 16);
		REG32(0xb8003200) = 100000;
		REG32(0xb8003208) = cg_info_query.lx_mhz;

		/* start to count */
		REG32(0xb8003208) |= (1 << 28);
		cpu_cnt = CPU_GET_CP0_CYCLE_COUNT();
		while (REG32(0xb800320c) == 0) {
			;
		}
		cpu_cnt = (CPU_GET_CP0_CYCLE_COUNT() - cpu_cnt) * C0_COUNT_MUL;

		/* restore timer backup */
		REG32(0xb8003200) = reg_backup[0];
		REG32(0xb8003208) = reg_backup[1];
	} while (i++<2);

	return (cpu_cnt*10) / (1000 * 1000);
}

static int rtl9603cvd_cg_get_cpu_mhz(cg_dev_freq_t *target) {
	int freq;
	if (_rtl9603cvd_cg_is_fpga()) {
		freq = target->cpu0_mhz;
		if (freq == 0) {
			freq = rtl9603cvd_cg_probe_cpu_mhz();
		}
	} else {
		freq = round_div(
			((RFLD(SYSPLLCTR, ocp0_pll_div) + 2) * 50),
			((1 << RFLD(PLL1, reg_en_div2_cpu0)) * (1 << RFLD(CMUGCR_OC0_9603CVD, freq_div)))
			);
	}
	return freq;
}

static void rtl9603cvd_cg_set_cpu_mhz(cg_dev_freq_t *target) {
	short cpu0_exp_pll_mhz;
	uint8_t cpu0_pll_div;
	uint8_t cmu_freq_div;
	uint8_t cmu_mode;
	uint8_t reg_en_div2_cpu0;

	if (_rtl9603cvd_cg_is_fpga()) {
		return;
	}

	cmu_freq_div = 0;
	cpu0_exp_pll_mhz = target->cpu0_mhz;

	while ((cpu0_exp_pll_mhz < 500) && (cmu_freq_div < 7)) {
		cpu0_exp_pll_mhz *= 2;
		cmu_freq_div++;
	}
	cmu_mode = !!(cmu_freq_div);

	if (cpu0_exp_pll_mhz < 500) {
		cpu0_exp_pll_mhz = 500;
	}

	reg_en_div2_cpu0 = !!(cpu0_exp_pll_mhz < 800);
	if (reg_en_div2_cpu0) {
		cpu0_pll_div = (cpu0_exp_pll_mhz / 25) - 2;
	} else {
		cpu0_pll_div = (cpu0_exp_pll_mhz / 50) - 2;
	}

	cpu0_exp_pll_mhz = (cpu0_pll_div+2)*25*(1 << !reg_en_div2_cpu0);
#if (CG_EXP_CPU_WALK_MHZ == 1)
	printf(
		"DD: PLL: %dMHz(FAC: %d, DIV2: %d); CMU MODE: %d, DIV: %d",
		cpu0_exp_pll_mhz,
		cpu0_pll_div,
		reg_en_div2_cpu0,
		cmu_mode,
		cmu_freq_div
		);
#endif

	/* reg. programming. */
	RMOD(CMUGCR_OC0_9603CVD, cmu_mode, CMU_MODE_DISABLE);
	_rtl9603cvd_cg_udelay(10);

	/* switch CPU clk to stable LX; already cache-lock, need not worry about slow bit */
	RMOD_SYS_STS(cf_ckse_ocp0, 0);
	_rtl9603cvd_cg_udelay(1);

	RMOD(SYSPLLCTR, ocp0_pll_div, cpu0_pll_div);
	RMOD(PLL1, reg_en_div2_cpu0, reg_en_div2_cpu0);
	_rtl9603cvd_cg_udelay(40);

	/* switch back CPU clk */
	RMOD_SYS_STS(cf_ckse_ocp0, 1);
	_rtl9603cvd_cg_udelay(1);

	/* DD requests to set 'freq_div' and 'cmu_mode' sequentially. */
	RMOD(CMUGCR_OC0_9603CVD, freq_div, cmu_freq_div);
	RMOD(CMUGCR_OC0_9603CVD, cmu_mode, cmu_mode);

	return;
}

static int rtl9603cvd_cg_config_cpu_mhz(cg_dev_freq_t *target) {
#if (CG_EXP_CPU_WALK_MHZ == 1)
	int exp_cpu_mhz;
	rtl9603cvd_cg_dev_freq_t exp_target;
	for (exp_cpu_mhz=1; exp_cpu_mhz<10; exp_cpu_mhz++) {
		printf("II: exp: %dMHz -> ", exp_cpu_mhz);
		exp_target.cpu0_mhz = exp_cpu_mhz;
		exp_target.cpu0_mhz = rtl9603cvd_cg_set_cpu_mhz(&exp_target);
		printf(" = %dMHz\n", exp_target.cpu0_mhz);
	}

	for (exp_cpu_mhz=10; exp_cpu_mhz<100; exp_cpu_mhz+=5) {
		printf("II: exp: %dMHz -> ", exp_cpu_mhz);
		exp_target.cpu0_mhz = exp_cpu_mhz;
		exp_target.cpu0_mhz = rtl9603cvd_cg_set_cpu_mhz(&exp_target);
		printf(" = %dMHz\n", exp_target.cpu0_mhz);
	}

	for (exp_cpu_mhz=100; exp_cpu_mhz<1201; exp_cpu_mhz+=25) {
		printf("II: exp: %dMHz -> ", exp_cpu_mhz);
		exp_target.cpu0_mhz = exp_cpu_mhz;
		exp_target.cpu0_mhz = rtl9603cvd_cg_set_cpu_mhz(&exp_target);
		printf(" = %dMHz\n", exp_target.cpu0_mhz);
	}
#endif

	if (target->cpu0_mhz) {
		rtl9603cvd_cg_set_cpu_mhz(target);
	}

	return rtl9603cvd_cg_get_cpu_mhz(target);
}

/********************************
	CG main flow
********************************/
void rtl9603cvd_cg_get_parameters(void) {
	inline_memcpy(&cg_info_query, &rtl9603cvd_cg_proj_freq, sizeof(cg_dev_freq_t));
	return;
}

static void rtl9603cvd_cg_xlat_n_assign(void) {
	uint32_t is_se_dram, is_se_spif, is_se_sram;
	uint32_t oc_div, oc_bus_mhz;
	uint32_t smiss, lmiss;

	uint32_t config7_bak;

	/* Set to slow but safty timing. */
	RMOD(
		CMUCR_OC0_9603CVD,
		se_sram_rom, 1,
		se_sram_rom_slp, 1);
	RMOD(
		SCATS,
		sram_to_oc0, 1,
		oc0_to_sram, 2
		);
	while (RFLD(SCATS, oc0_to_sram) != 2) {
		;
	}

	uint32_t s2lpw, s2o0pw, o02rpw, o02spw;

	config7_bak = _rtl9603cvd_cg_disable_bp();

	writeback_invalidate_dcache_all();

	fetch_lock_dcache_range(CPU_GET_STACK_PTR()-256, CPU_GET_STACK_PTR()+512);
	fetch_lock_dcache_range((uint32_t)&cg_info_query, (uint32_t)&cg_info_query+64);

	fetch_lock_icache_range((uint32_t)rtl9603cvd_cg_xlat_n_assign, (uint32_t)rtl9603cvd_cg_xlat_n_assign+2048);
	fetch_lock_icache_range((uint32_t)_rtl9603cvd_cg_udelay, (uint32_t)_rtl9603cvd_cg_udelay+128);

	_rtl9603cvd_cg_enable_cache_miss_cnt();

	cg_info_query.lx_mhz = rtl9603cvd_cg_config_lx_mhz(&cg_info_query);
	cg_info_query.spif_mhz = rtl9603cvd_cg_config_spif_mhz(&cg_info_query);
	cg_info_query.cpu0_mhz = rtl9603cvd_cg_config_cpu_mhz(&cg_info_query);
	cg_info_query.sram_mhz = rtl9603cvd_cg_config_sram_mhz(&cg_info_query);

	oc_div = !_rtl9603cvd_cg_is_fpga(); //oc_div: fpga: 0; asic: 1
	/* is_se_dram = (cpu0_mhz <= mem_mhz)? 1: 0; */
	oc_bus_mhz = cg_info_query.cpu0_mhz >> oc_div;
	is_se_dram = !!(oc_bus_mhz <= cg_info_query.mem_mhz);
	is_se_spif = !!(oc_bus_mhz <= cg_info_query.nor_pll_mhz);
	is_se_sram = !!(oc_bus_mhz <= cg_info_query.sram_mhz);

	s2lpw = roundup_div(cg_info_query.sram_mhz*10, cg_info_query.lx_mhz) > 29? 1: 0;
	s2o0pw = roundup_div(cg_info_query.sram_mhz*10, oc_bus_mhz) > 9? 1: 0;
	o02rpw = roundup_div(oc_bus_mhz*10, 250) > 29? 1: 0; //ROM is fixed at 250MHz
	o02spw = roundup_div(oc_bus_mhz*10, cg_info_query.sram_mhz);
	if (o02spw > 19) {
		o02spw = 2;
	} else if (o02spw > 9) {
		o02spw = 1;
	} else {
		o02spw = 0;
	}

	RMOD(
		SCATS,
		sram_to_lx, s2lpw,
		sram_to_oc0, s2o0pw,
		oc0_to_rom, o02rpw,
		oc0_to_sram, o02spw
		);

	while (RFLD(SCATS, oc0_to_sram) != o02spw) {
		;
	}

	if (cg_info_query.lx_mhz > cg_info_query.mem_mhz) {
		RVAL(LBSBCR_9603CVD) = RVAL(LBSBCR_9603CVD) & (~0x007f010f);
	} else {
		RVAL(LBSBCR_9603CVD) = RVAL(LBSBCR_9603CVD) | 0x007f010f;
	}

	RMOD(
		CMUCR_OC0_9603CVD,
		se_spif, is_se_spif,
		se_spif_slp, is_se_spif,

		se_sram_rom, is_se_sram,
		se_sram_rom_slp, is_se_sram,

		se_dram, is_se_dram,
		se_dram_slp, is_se_dram);

	while (RFLD_SFCR(ocp0_frq_slower) != is_se_spif) {
		;
	}

	_rtl9603cvd_cg_check_then_disable_cache_miss_cnt(&smiss, &lmiss);

	invalidate_icache_all();
	writeback_invalidate_dcache_all();

	_rtl9603cvd_cg_restore_bp(config7_bak);

	uart_init(uart_info.baud, cg_info_query.lx_mhz);

	printf("II: store/load miss cnt.: %d/%d\n", smiss, lmiss);

	return;
}

/* cg_config_mem_mhz() is excluded from cg_xlat_n_assing() to reduce
   cache-lock region. While CLI CG needs it to complete the whole
   CG flow, _cg_xlat_n_assign() wraps both func. together as a single
   entry. */
void _rtl9603cvd_cg_xlat_n_assign(void) {
	cg_info_query.mem_mhz = rtl9603cvd_cg_config_mem_mhz(&cg_info_query);
	rtl9603cvd_cg_xlat_n_assign();
	return;
}



void rtl9603cvd_cg_result_decode(void) {
	CMUGCR_OC0_9603CVD_T cmugcr_oc0;

	/* 1: 0x26='&', 0: 0x26+0x56=0x7c='|' */
	const char is_nand = RFLD_MCR(boot_sel);
	const char flash_type = 0x7c - (is_nand*0x56);
	int flash_pll_div;
	PHY_RG5X_PLL_T phy_rg5x_pll;

	cmugcr_oc0.v = RVAL(CMUGCR_OC0_9603CVD);

	printf("II: CPU:%dMHz, LX:%dMHz, MEM:%dMHz, SPIF:%dMHz, SRAM:%dMHz\n",
				 cg_info_query.cpu0_mhz, cg_info_query.lx_mhz,
				 cg_info_query.mem_mhz, cg_info_query.spif_mhz, cg_info_query.sram_mhz);

	printf("II:   CPU: PLL_DIV:%d+2, DIVN2:%d; CMU MODE:%d, DIV:%d\n",
	       RFLD(SYSPLLCTR, ocp0_pll_div),
	       RFLD(PLL1, reg_en_div2_cpu0),
				 cmugcr_oc0.f.cmu_mode,
				 cmugcr_oc0.f.freq_div);

	if (_rtl9603cvd_cg_is_fpga()) {
		/* no pll divisor in fpga */
		flash_pll_div = 1;
	} else {
		phy_rg5x_pll.v = PHY_RG5X_PLLrv;
		flash_pll_div = (
			is_nand ?
			phy_rg5x_pll.f.ck200m_lx_pll:
			phy_rg5x_pll.f.ck250m_spif_pll);
		printf(
			"II:   LXB: DIV:%d+5\n",
			phy_rg5x_pll.f.ck200m_lx_pll);
	}

	//snaf & snof cntlr. div are identical after spif cg init.
	printf(
		"II:   N%cF: PLL_DIV:%d+%d, CNTLR_DIV:(%d+1)*2\n",
		flash_type, flash_pll_div, 4+is_nand,
		RFLD_SNAFCFR(spi_clk_div));

	return;
}

void rtl9603cvd_cg_init(void) {

	if (cg_info_query.bypass_cg_init == 0) {
		_rtl9603cvd_cg_xlat_n_assign();
	}

	rtl9603cvd_cg_result_decode();
	return;
}

