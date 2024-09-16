#ifndef UART_H_
#define UART_H_

#include "stdint.h"
#define UART            ((Uart_Reg*)(0xF0010000))

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

#endif /* UART_H_ */


