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

void _timer_init(){
	interruptCtrl_mask_set(TIMER_INTERRUPT,0x1);
	prescaler_set(TIMER_PRESCALER,27 - 1);
	timer_limit_set(TIMER_A,1000 - 1);
	timer_autoreload_en(TIMER_A);
	timer_ticks_en(TIMER_A);
    println("timer init success!");
}

void main() {
    println("hello world tang primer 20k!");
	UART->STATUS |= 0x2;
    GPIO_A->OUTPUT_ENABLE = 0x0000000F;
	GPIO_A->OUTPUT = 0x00000001;
    const int nleds = 4;
	const int nloops = 1000000;
	_timer_init();
	sd_card_init();
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

	if(interruptCtrl_pendins_get(TIMER_INTERRUPT)){
		interruptCtrl_clear(TIMER_INTERRUPT);
		systick_1ms_flag = 1;
		// printhex(timer_value_get(TIMER_A));
		// timer_clear(TIMER_A);
		// println("enter timer irq success!");
		return;
	}

	uint32_t uart_statis = UART->STATUS;

	UART->STATUS &= ~0x2;

	// uint32_t mcause;
    // asm volatile ("csrr %0, mcause" : "=r" (mcause));
	// print("mcause register value:0x");
	// printhex(mcause);
	// println(".");

	print("uart status value:0x");
	printhex(uart_statis);
	println(".");

	uint32_t rxbuf_len;

	char rx_buf[17];
	uint16_t i = 0;

	do{
		rx_buf[i] = UART->DATA;

		rxbuf_len = uart_readOccupancy(UART);

		i++;

	}while(rxbuf_len > 0);

	rx_buf[i] = 0;

	print(rx_buf);
	
	UART->STATUS |= 0x2;
}
