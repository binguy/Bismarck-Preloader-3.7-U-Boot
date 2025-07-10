#ifndef _UART_H_
#define _UART_H_

#define O_DLL (0x0)
#define O_THR (0x0)
#define O_RBR (0x0)
#define O_DLH (0x4)
#define O_IER (0x4)
#define O_IIR (0x8)
	#define IIR_FIFO16_MASK (0xe0)
#define O_FCR (0x8)
	#define FCR_FIFO_EN (0x01)
	#define FCR_RBR_RESET (0x02)
	#define FCR_THR_RESET (0x04)
	#define FCR_RBR_TRIG_HI (0xc0)
#define O_ADD (0x8)
#define O_LCR (0xc)
	#define LCR_7B (0x2)
	#define LCR_8B (0x3)
	#define LCR_1S (0 << 2)
	#define LCR_2S (1 << 2)
	#define LCR_NO_PARITY (0 << 3)
	#define LCR_BRK_SIG (1 << 6)
	#define LCR_DLAB_EN (1 << 7)
#define O_MCR (0x10)
	#define MCR_DTR (0x1)
	#define MCR_RTS (0x2)
	#define MCR_AFE (0x20)
	#define MCR_CLK_LX (0x40)
#define O_LSR (0x14)
	#define LSR_DR (0x01)
	#define LSR_OE (0x02)
	#define LSR_PE (0x04)
	#define LSR_FE (0x08)
	#define LSR_THRE (0x20)
	#define LSR_TEMT (0x40)
	#define LSR_RFE (0x80)

#ifndef REG8
	#define REG8(addr) (*((volatile uint8_t *)addr))
#endif
#define UART_ADDR(id, off) (0xb8002000 + id*0x100 + off)
#define UART_REG(id, off) REG8(UART_ADDR(id, off))

typedef unsigned int uint32_t;
typedef unsigned char uint8_t;

typedef struct {
	uint32_t uid;
	uint32_t baud;
} uart_parameter_t;

extern const uart_parameter_t uart_info;
int uart_tstc(void);
int uart_getc(void);
int uart_getc_nb(void);
void uart_putc(char c);
void uart_putcn(char ch);
UTIL_FAR int uart_init(u32_t brate, u32_t bus_mhz);

#endif
