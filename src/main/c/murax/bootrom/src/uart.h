#ifndef UART_H_
#define UART_H_

#include "const.h"

typedef struct
{
  volatile uint32_t DATA;				// 0x0
  volatile uint32_t STATUS;				// 0x4
  volatile uint32_t CLOCK_DIVIDER;		// 0x8
  volatile uint32_t FRAME_CONFIG;		// 0xc
} Uart_Reg;

static uint32_t uart_writeAvailability(Uart_Reg *reg){
	return (reg->STATUS >> 16) & 0xFF;
}

static uint32_t uart_readOccupancy(Uart_Reg *reg){
	return reg->STATUS >> 24;
}

static void uart_write(Uart_Reg *reg, uint32_t data){
	while(uart_writeAvailability(reg) == 0);
	reg->DATA = data;
}

static void print(const char*str){
	while(*str){
		uart_write(UART,*str);
		str++;
	}
}

static void printu32(uint32_t val){
	uart_write(UART,val>>24);
	uart_write(UART,val>>16);
	uart_write(UART,val>>8);
	uart_write(UART,val);
}

static void printhex(uint32_t val){
	static char hex_chars[] = "0123456789ABCDEF";
	static char str[8];

	int i = 0;
    while (i < 8) {
        str[i] = hex_chars[val & 0xF]; // 先填充高4位
        val >>= 4;
        i++;
        str[i] = hex_chars[val & 0xF]; // 再填充低4位
        val >>= 4;
        i++;
    }

	i--;
	// while(str[i] == '0')
	// 	i--;
	while(i >= 0){
		uart_write(UART,str[i]);
		i--;
	}

}

#endif /* UART_H_ */

