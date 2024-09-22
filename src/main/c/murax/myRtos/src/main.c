//#include "stddefs.h"
// #include <stdint.h>
#include <stdio.h>
#include <stdarg.h>

#include "murax.h"

#define DELAY_MS_C 452*2/KERNEL_TASK_NAME_MAXLEN

#define delay_ms(ms) delay(ms*452)

// 4clk
volatile void delay(uint32_t loops){
	while(loops--){
		GPIO_A->OUTPUT_ENABLE = ~GPIO_A->OUTPUT_ENABLE;
	}
}

_TASK_FUN led_blind()
{
	GPIO_A->OUTPUT = 0xffffffff;
	println("enter led_blind");
	while(1)
	{
		// task_delay(250);
		GPIO_A->OUTPUT = ~GPIO_A->OUTPUT;
		delay_ms(250);
    // kernel_task_status_log();
		println("led_blind delay 500");
		delay_ms(250);
		// task_delay(250);
	}
}

_TASK_FUN uart_tx()
{
	println("enter uart_tx");
	while(1)
	{
		println("uart task delay 1000ms");
		// task_delay(100000);
		delay_ms(1000);//30s
		// delay(270000*10);
	}
}

void main() {
	kernel_init();

    // println("hello world tang primer 20k!");

	kernel_task_init("uart",uart_tx);

	kernel_task_init("led",led_blind);

	kernel_start();

	while(1);
}