#if defined(CONFIG_UNDER_UBOOT)
#include <common.h>
#include <command.h>
#else
#include <soc.h>
#include <util.h>
#include <uart/uart.h>
#endif

void uart_show_stat(void);

short console_auto_set(void) {
	int j, n;
	const short lx_freq[] = {
		250, 225, 204, 200, 175, 150, 125, 100,
		80, 75, 66, 63, 60, 50, 40, 36,
		35, 34, 33, 32, 30, 27, 25, 16,
		8
	};
	char ans[4];

	j = -1;
	n = 0;

 uart_init_start:
	j = (j+1) % sizeof(lx_freq);
	n++;

	uart_init(uart_info.baud, lx_freq[j]);

	/* query terminal status */
	uart_putc('\x1b');
	uart_putc('[');
	uart_putc('5');
	uart_putc('n');

	ans[0] = uart_getc_nb();
	ans[1] = uart_getc_nb();
	ans[2] = uart_getc_nb();
	ans[3] = uart_getc_nb();
	if ((ans[0] == '\x1b') &&
			(ans[1] == '[') &&
			(ans[2] == '0') &&
			(ans[3] == 'n')) {
		;
	} else {
		goto uart_init_start;
 	}

	uart_show_stat();
	return lx_freq[j];
}

#if defined(CONFIG_UNDER_UBOOT)
int do_caset(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]) {
	return console_auto_set();
}

U_BOOT_CMD(caset, 1 , 1, do_caset,
           "caset - Console Auto-set Test.",
           "");
#endif
