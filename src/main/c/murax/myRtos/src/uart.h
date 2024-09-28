#ifndef UART_H_
#define UART_H_

#include "const.h"

static char hex_chars[] = "0123456789ABCDEF";

typedef struct
{
  volatile uint32_t DATA;				// 0x0
  volatile uint32_t STATUS;				// 0x4
  volatile uint32_t CLOCK_DIVIDER;		// 0x8
  volatile uint32_t FRAME_CONFIG;		// 0xc
} Uart_Reg;

enum UartParity {NONE = 0,EVEN = 1,ODD = 2};
enum UartStop {ONE = 0,TWO = 1};

typedef struct {
	uint32_t dataLength;
	enum UartParity parity;
	enum UartStop stop;
	uint32_t clockDivider;
} Uart_Config;

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

static void uart_applyConfig(Uart_Reg *reg, Uart_Config *config){
	reg->CLOCK_DIVIDER = config->clockDivider;
	reg->FRAME_CONFIG = ((config->dataLength-1) << 0) | (config->parity << 8) | (config->stop << 16);
}

static void print(const char*str){
	while(*str){
		uart_write(UART,*str);
		str++;
	}
}

static void println(const char*str){
	print(str);
	uart_write(UART,'\n');
}

static void printbyte(uint8_t val){
	uart_write(UART,hex_chars[(val >> 4) & 0xF]);
	uart_write(UART,hex_chars[val & 0xF]);
}

static void printhex(uint32_t val){
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
	while(str[i] == '0')
		i--;
	while(i >= 0){
		uart_write(UART,str[i]);
		i--;
	}

}

#endif /* UART_H_ */


