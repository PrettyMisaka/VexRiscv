//#include "stddefs.h"
// #include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
// #include "gImage_misaka_rgb8888.h"

#include "lvgl/lvgl.h" 
#include "lvgl/examples/porting/lv_port_disp_template.h"
#include "lvgl/examples/porting/lv_port_indev_template.h"

#include "lvgl/demos/benchmark/lv_demo_benchmark.h" 
#include "lvgl/demos/stress/lv_demo_stress.h" 

#include "murax.h"
extern void trap_entry(void);
	
	#define HDMI_H 1280l
	#define HDMI_W 720l
	#define HDMI_BLOCK 80
	//					      384000
	#define HDMI_FB_BASE (0x47c00000-160+20)
	// #define HDMI_FB_BASE (0x47c00000-16)

void hdmi_fill(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color){
	uint32_t *p = (uint32_t*)(HDMI_FB_BASE+(x+(y*HDMI_H)<<2));

	// print("p:0x");
	// printhex((uint32_t)p);
	// print(",x:");
	// printhex(x);
	// print(",y:");
	// printhex(y);
	// print(",y*HDMI_H:0x");
	// printhex(y*HDMI_H);

	for(int i = 0; i < h; i++){
		for(int j = 0; j < w; j++){
			*p = color;
			p++;
		}
		p += (HDMI_H - w);
	}
	
	// print(",_p:0x");
	// printhex((uint32_t)p);
	// println("");
}

void delay(uint32_t loops){
	for(int i=0;i<loops;i++){
		int tmp = GPIO_A->OUTPUT;
	}
}

void ddr3_reg_log()
{
	print(" ddr3 reg:0x");
	printhex(*(uint32_t*)0x48000000);
	print(",0x");
	printhex(*(uint32_t*)0x48000004);
	print(",0x");
	printhex(*(uint32_t*)0x48000008);
	print(",0x");
	printhex(*(uint32_t*)0x4800000c);
	print(",0x");
	printhex(*(uint32_t*)0x48000010);
	println("");
}

void get_ddr3_ram_data(uint32_t begin, uint32_t end)
{
	volatile uint32_t* ddr_base;
	for(ddr_base = (uint32_t*)begin; (uint32_t)ddr_base < end;){
		// if(*ddr_base != wr_val){
			// print("error data!");
			printhex((uint32_t)ddr_base);
			print(":");
			printhex(*ddr_base);

			ddr3_reg_log();
			// break;
		// }
		// wr_val += 4;
		ddr_base += 1;
		if(((uint32_t)ddr_base & 0x00FFFFFF) == 0x00000000){
			print("current addr:");
			printhex((uint32_t)ddr_base);
		}
	}
}

void _timer_init(){
    println("enter timer init!");
	timer_clear(TIMER_A);
	timer_autoreload_en(TIMER_A);
	prescaler_set(TIMER_PRESCALER,81 - 1);
	timer_limit_set(TIMER_A,1000 - 1);
	timer_ticks_en(TIMER_A);
	interruptCtrl_mask_set(TIMER_INTERRUPT,0x1);
    println("timer init success!");
}

void main() {
	asm volatile("csrw mtvec, %0" : : "r" ((uint32_t)trap_entry));
    println("hello world tang primer 20k!");
	// UART->STATUS |= 0x2;
    GPIO_A->OUTPUT_ENABLE = 0x0000000F;
	GPIO_A->OUTPUT = 0x00000001;
    const int nleds = 4;
	const int nloops = 100000;

	*(uint32_t*)0x48001000 = 0xffffffff; //2 1
	int i = 0;

	_timer_init();
	lv_log_register_print_cb(println);
	
	lv_init();
	println("lv_init success");
	lv_port_disp_init();
	println("lv_port_disp_init success");
    // LV_USE_THEME_DEFAULT使能为1会导致0x40030000内存发生错误
	// while(1);
	// lv_port_indev_init(); 
	// println("lv_port_indev_init success");
	lv_demo_benchmark();
	// lv_demo_stress();
	println("lv_demo_benchmark");

	println("done");

    while(1){
		lv_timer_handler();
		// systick_delayms(5);
    }
}

void irqCallback(){

	if(interruptCtrl_pendins_get(TIMER_INTERRUPT)){
		interruptCtrl_clear(TIMER_INTERRUPT);
		systick_1ms_flag = 1;
			// GPIO_A->OUTPUT = ~GPIO_A->OUTPUT;
		lv_tick_inc(1);
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
