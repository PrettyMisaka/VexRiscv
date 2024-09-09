//#include "stddefs.h"
// #include <stdint.h>
#include <stdio.h>
#include <stdarg.h>

#include "murax.h"

void delay(uint32_t loops){
	for(int i=0;i<loops;i++){
		int tmp = GPIO_A->OUTPUT;
	}
}

void main() {
	UART->STATUS |= 0x2;
    GPIO_A->OUTPUT_ENABLE = 0x0000000F;
	GPIO_A->OUTPUT = 0x00000001;
    const int nleds = 4;
	const int nloops = 1000000;
    println("hello world tang primer 20k!");
    while(1){
    	for(unsigned int i=0;i<nleds-1;i++){
    		GPIO_A->OUTPUT = 1<<i;
    		delay(nloops);
    	}
    	for(unsigned int i=0;i<nleds-1;i++){
			GPIO_A->OUTPUT = (1<<(nleds-1))>>i;
			delay(nloops);
		}
    }
}

void irqCallback(){

	// uint32_t uart_statis = UART->STATUS;

	UART->STATUS &= ~0x2;

	// uint32_t mcause;
    // asm volatile ("csrr %0, mcause" : "=r" (mcause));
	// print("mcause register value:0x");
	// printhex(mcause);
	// println(".");

	// print("uart status value:0x");
	// printhex(uart_statis);
	// println(".");

	uint32_t rxbuf_len;

	char rx_buf[17];
	uint16_t i = 0;

	do{
		rx_buf[i] = UART->DATA;

		rxbuf_len = uart_readOccupancy(UART);

		i++;

	}while(rxbuf_len > 0);

	rx_buf[i] = 0;

	println(rx_buf);
	
	UART->STATUS |= 0x2;
}
