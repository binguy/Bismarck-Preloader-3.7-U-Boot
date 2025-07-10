#include <soc.h>
#include <uart/uart.h>

#define UID uart_info.uid

typedef struct {
	uint32_t frame_period_c;
	uint32_t base_clk_mhz;
	uint32_t r_to_cnt;
	uint32_t t_to_cnt;
} uart_stat_t;

static uart_stat_t ustat;

static void
uart_set_baud(uint32_t uid, uint32_t baudrate, uint32_t base_clk_mhz) {
	uint32_t bclk_hz = base_clk_mhz * 1000 * 1000;
	uint32_t div, add, is_hs_uart;
	uint8_t lcr_bak;

	lcr_bak = UART_REG(uid, O_LCR);
	UART_REG(uid, O_LCR) = (lcr_bak | LCR_DLAB_EN);
	is_hs_uart = ((UART_REG(uid, O_IIR) & IIR_FIFO16_MASK) == 0);

	if (is_hs_uart) {
		div = bclk_hz / (16 * baudrate);
		add = (bclk_hz + (baudrate / 2)) / baudrate - (16 * div);
		UART_REG(uid, O_ADD) = add;
	} else {
		div = (bclk_hz + (8 * baudrate)) / (16 * baudrate);
	}

	UART_REG(uid, O_DLL) = (div) & 0xff;
	UART_REG(uid, O_DLH) = (div >> 8) & 0xff;
	UART_REG(uid, O_LCR) = lcr_bak;
	return;
}

static uint32_t
_uart_init(uint32_t uid, uint32_t baudrate, uint32_t base_clk_mhz) {
	uint32_t base;
	ustat.r_to_cnt = 0;
	ustat.t_to_cnt = 0;
	ustat.base_clk_mhz = base_clk_mhz;

	UART_REG(uid, O_IER) = 0;

	uart_set_baud(uid, baudrate, base_clk_mhz);

	UART_REG(uid, O_LCR) = (LCR_8B | LCR_1S | LCR_NO_PARITY);
	UART_REG(uid, O_MCR) = (MCR_DTR | MCR_RTS | MCR_CLK_LX);
	UART_REG(uid, O_FCR) = (FCR_RBR_TRIG_HI | FCR_FIFO_EN);
	UART_REG(uid, O_FCR) = (FCR_RBR_TRIG_HI | FCR_THR_RESET | FCR_RBR_RESET | FCR_FIFO_EN);

	while ((UART_REG(uid, O_LSR) & (LSR_TEMT | LSR_THRE)) != (LSR_TEMT | LSR_THRE)) ;
	base = CPU_GET_CP0_CYCLE_COUNT();
	UART_REG(uid, O_THR) = 0;
	UART_REG(uid, O_THR) = 0;
	while ((UART_REG(uid, O_LSR) & (LSR_TEMT | LSR_THRE)) != (LSR_TEMT | LSR_THRE)) ;
	ustat.frame_period_c = (CPU_GET_CP0_CYCLE_COUNT() - base);

	return 0;
}

SECTION_UART void
uart_fifo_fade_out(uint32_t num) {
	int i;
	uint32_t base;
	for (i=0; i<num; i++) {
		base = CPU_GET_CP0_CYCLE_COUNT();
		while ((CPU_GET_CP0_CYCLE_COUNT() - base) > ustat.frame_period_c) {
			;
		}
		base = UART_REG(UID, O_RBR);
	}
	return;
}

SECTION_UART void
uart_show_stat(void) {
	printf("\nII: UART base freq.: %d; Frame period: %d C",
	       ustat.base_clk_mhz, ustat.frame_period_c);
	printf("\nII: RBR/THR timeout #: %d/%d\n", ustat.r_to_cnt, ustat.t_to_cnt);
	uart_fifo_fade_out(64);
	return;
}

SECTION_UART int
uart_tstc(void) {
	return (UART_REG(UID, O_LSR) & (LSR_RFE | LSR_FE | LSR_PE | LSR_OE | LSR_DR)) == LSR_DR;
}

SECTION_UART int
uart_getc(void) {
	while (!uart_tstc());

	return UART_REG(UID, O_RBR);
}

SECTION_UART int
uart_getc_nb(void) {
	uint32_t base, latency;

	base = CPU_GET_CP0_CYCLE_COUNT();
	do {
		if (uart_tstc()) {
			return UART_REG(UID, O_RBR);
		}
		latency = CPU_GET_CP0_CYCLE_COUNT() - base;
	} while(latency < ustat.frame_period_c);

	ustat.r_to_cnt++;
	return 0;
}

SECTION_UART void
uart_putc(char c) {
	while (!(UART_REG(UID, O_LSR) & LSR_THRE)) {
		;
	}

	UART_REG(UID, O_THR) = c;
	return;
}

SECTION_UART void
uart_putc_nb(char c) {
	uint32_t base;

	base = CPU_GET_CP0_CYCLE_COUNT();
	do {
		if (UART_REG(UID, O_LSR) & LSR_THRE) break;
	} while((CPU_GET_CP0_CYCLE_COUNT() - base) < ustat.frame_period_c);

	if (UART_REG(UID, O_LSR) & LSR_THRE) {
		UART_REG(UID, O_THR) = c;
	} else {
		UART_REG(UID, O_FCR) = (FCR_RBR_TRIG_HI|FCR_THR_RESET|FCR_RBR_RESET|FCR_FIFO_EN);
		ustat.t_to_cnt++;
	}
	return;
}

SECTION_UART void
uart_putcn(char ch) {
	if (ch=='\n') uart_putc('\r');
	uart_putc(ch);
	return;
}

UTIL_FAR SECTION_UART int
uart_init(u32_t brate, u32_t bus_mhz) {
	_uart_init(UID, brate, bus_mhz);
	return 0;
}
