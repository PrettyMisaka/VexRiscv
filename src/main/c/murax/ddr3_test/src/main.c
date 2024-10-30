//#include "stddefs.h"
// #include <stdint.h>
#include <stdio.h>
#include <stdarg.h>

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
	interruptCtrl_mask_set(TIMER_INTERRUPT,0x1);
	prescaler_set(TIMER_PRESCALER,81 - 1);
	timer_limit_set(TIMER_A,1000 - 1);
	timer_autoreload_en(TIMER_A);
	timer_ticks_en(TIMER_A);
    println("timer init success!");
}

void main() {
	asm volatile("csrw mtvec, %0" : : "r" ((uint32_t)trap_entry));
    println("hello world tang primer 20k!");
	UART->STATUS |= 0x2;
    GPIO_A->OUTPUT_ENABLE = 0x0000000F;
	GPIO_A->OUTPUT = 0x00000001;
    const int nleds = 4;
	const int nloops = 100000;
	_timer_init();

	volatile uint32_t* ddr_base;
	volatile uint32_t wr_val = 0x40000000;
	*(uint32_t*)0x48001000 = 0xffffffff; //2 1
	int i = 0;
	delay(270000);
	println("done");

	println("clear hdmi out");
	for( ddr_base = (uint32_t*)0x47c00000; (uint32_t)ddr_base < 0x48000000 ;){
		*ddr_base = 0xff888888;
		// *ddr_base = (((uint32_t)ddr_base & 0xfffffff0) == 0x400007F0)?wr_val|0x80000000:wr_val;
		ddr_base += 1;
	}
	println("clear hdmi done!");
	uint32_t x, y;
	ddr_base = (uint32_t*)HDMI_FB_BASE;
	uint32_t x_offset, y_offset;

	volatile uint8_t cnt = 0;
	volatile uint32_t color = 0xffffffff;

	while(1){
	for(x_offset = 0; x_offset < 4; x_offset++){
		for(y_offset = 0; y_offset < 4; y_offset++){
	// for(x_offset = 0; x_offset < 16; x_offset++){
	// 	for(y_offset = 0; y_offset < 9; y_offset++){
			
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

			// hdmi_fill(x_offset*HDMI_H/4,y_offset*HDMI_W/4,HDMI_H/4,HDMI_W/4,color);
			// hdmi_fill(x_offset*HDMI_BLOCK,y_offset*HDMI_BLOCK,HDMI_BLOCK,HDMI_BLOCK,color);
			hdmi_fill(20,20,1240,680,color);

			cnt++;
			// delay(540000);
			systick_delayms(50);
		}
	}
	}

	while(1){
		uint32_t *p = (uint32_t*)0x44000000;

		for(int i = 0; i < HDMI_BLOCK; i++){
			for(int j = 0; j < HDMI_BLOCK; j++){
				*p = 0xff888888;
				p++;
			}
			p += (HDMI_H - HDMI_BLOCK);
		}

			systick_delayms(100);
	}

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
