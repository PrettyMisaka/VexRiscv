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
		GPIO_A->OUTPUT = ~GPIO_A->OUTPUT;
		// delay_ms(250);
		task_delay(250);
    // kernel_task_status_log();
		println("led_blind delay 500");
		// delay_ms(250);
		task_delay(250);
	}
}

_TASK_FUN uart_tx()
{
	println("enter uart_tx");
	while(1)
	{
		println("uart task delay 1000ms");
		task_delay(1000);
		// delay_ms(1000);//30s
		// delay(270000*10);
	}
}

_TASK_FUN hdmi_flash()
{
	#define HDMI_H 1280l
	#define HDMI_W 720l
	#define HDMI_BLOCK 80
	//					      384000
	#define HDMI_FB_BASE (0x47c00000-160+20)

	*(uint32_t*)0x48001000 = 0xffffffff; //2 1
	volatile uint8_t cnt = 0;
	volatile uint32_t color = 0xffffffff;
	while(1)
	{
		uint32_t *p = (uint32_t*)(HDMI_FB_BASE+(20+(20*HDMI_H)<<2));
		for(int i = 0; i < 680; i++){
			for(int j = 0; j < 1240; j++){
				*p = color;
				p++;
			}
			p += (HDMI_H - 1240);
		}			

		switch (cnt%5)
		{
		case 0: color = 0xffffffff; break;
		case 1: color = 0xff000000; break;
		case 2: color = 0xff0000ff; break;
		case 3: color = 0xff00ff00; break;
		case 4: color = 0xffff0000; break;
		default:
			break;
		}

		print("cnt:");
		printhex(cnt%5);
		print(",color:0x");
		printhex(color);
		println("");
		cnt++;

		// task_delay(1000);
	}
}

void main() {
	sd_card_init();
	
	kernel_init();

    // println("hello world tang primer 20k!");

	kernel_task_init("uart",uart_tx);

	kernel_task_init("led_t",led_blind);

	kernel_task_init("hdmi",hdmi_flash);

	kernel_start();

	while(1);
}