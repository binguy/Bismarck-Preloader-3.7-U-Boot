

#define EN_UART0_PAD() {\
    REG32(0xbb023014) = 0x00020050;\
}


#define UART_IO_EN() EN_UART0_PAD()



