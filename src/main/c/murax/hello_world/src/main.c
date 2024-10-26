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

	for( uint32_t *ddr_base = (uint32_t*)0x40000000, wr_val = (uint32_t)ddr_base; (uint32_t)ddr_base < 0x40000080;){
		*ddr_base = wr_val;
		wr_val += 4;
		ddr_base += 1;
	}

	get_ddr3_ram_data(0x40000000, 0x40000080);
	//flash
	*(uint32_t*)0x40002000 = 0x40002000; //2 1
	ddr3_reg_log();
	*(uint32_t*)0x40003000 = 0x40003000; //3 2 1
	ddr3_reg_log();
	*(uint32_t*)0x40004000 = 0x40004000; //4 3 2
	ddr3_reg_log();
	*(uint32_t*)0x40005000 = 0x40005000; //5 4 3
	ddr3_reg_log();

	get_ddr3_ram_data(0x40000000, 0x40000020);
	get_ddr3_ram_data(0x40000080, 0x400000a0);

	// println("write data to 0x40000200...");
	// *(uint32_t*)0x40000200 = 0x40000200; //2 1
	// *(uint32_t*)0x40000204 = 0x40000204; //2 1
	// *(uint32_t*)0x40000208 = 0x40000208; //2 1
	// *(uint32_t*)0x4000020c = 0x4000020c; //2 1
	// ddr3_reg_log();
	// get_ddr3_ram_data(0x40000200, 0x40000220);
	// println("write data to 0x400003F0...");
	// *(uint32_t*)0x400003F0 = 0x400003F0; //2 1
	// *(uint32_t*)0x400003F4 = 0x400003F4; //2 1
	// *(uint32_t*)0x400003F8 = 0x400003F8; //2 1
	// *(uint32_t*)0x400003Fc = 0x400003Fc; //2 1
	// ddr3_reg_log();
	// get_ddr3_ram_data(0x400003F0, 0x40000400);
	//flash
	*(uint32_t*)0x40002000 = 0x40002000; //2 1
	*(uint32_t*)0x40003000 = 0x40003000; //3 2 1
	*(uint32_t*)0x40004000 = 0x40004000; //4 3 2
	*(uint32_t*)0x40005000 = 0x40005000; //5 4 3

	// get_ddr3_ram_data(0x40000200, 0x40000220);
	// while(1);

	// print("read 0x40000000:");
	// printhex(*(uint32_t*)0x40000000); //4
	// print("read 0x40000004:");
	// printhex(*(uint32_t*)0x40000004); //4
	// print("read 0x40000008:");
	// printhex(*(uint32_t*)0x40000008); //4
	// print("read 0x4000000c:");
	// printhex(*(uint32_t*)0x4000000c); //4
	// println("");
	// ddr3_reg_log();

	// while(1);

	volatile uint32_t* ddr_base;
	volatile uint32_t wr_val = 0x40000000;
	*(uint32_t*)0x48001000 = 0xffffffff; //2 1
	int i = 0;
	delay(270000);
	println("done");
	// goto lab_flash;
	// for(; (uint32_t)ddr_base < 0x41000000;){
	//0x40000280 可以正常读写
	//0x40000400 0x40000380有问题
	//0x40000300 乱码 跑飞 对0x400007FX读写会使0x4000000X出现问题
	//屏蔽0x0x40000000~0x40030000 
	for( ddr_base = (uint32_t*)0x40030000,wr_val = (uint32_t)0x0; (uint32_t)ddr_base < 0x48000000;){
		*ddr_base = wr_val;
		// *ddr_base = (((uint32_t)ddr_base & 0xfffffff0) == 0x400007F0)?wr_val|0x80000000:wr_val;
		ddr_base += 1;
		if(((uint32_t)ddr_base & 0x00FFFFFF) == 0x00000000){
			print(" ");
			printhex((uint32_t)ddr_base);
		}
	}
	for( ddr_base = (uint32_t*)0x40030000,wr_val = (uint32_t)ddr_base; (uint32_t)ddr_base < 0x48000000;){
		*ddr_base = wr_val;
		// *ddr_base = (((uint32_t)ddr_base & 0xfffffff0) == 0x400007F0)?wr_val|0x80000000:wr_val;
		wr_val += 4;
		ddr_base += 1;
		if(((uint32_t)ddr_base & 0x00FFFFFF) == 0x00000000){
			print(" ");
			printhex((uint32_t)ddr_base);
		}
	}
	print("ddr3 module write done! checking \r\n");
	for( ddr_base = (uint32_t*)0x40030000,wr_val = (uint32_t)ddr_base; (uint32_t)ddr_base < 0x48000000;){
		// *ddr_base = 0x1f;
		if(*ddr_base != wr_val){
			println("<<<");
			get_ddr3_ram_data((uint32_t)ddr_base, (uint32_t)ddr_base + 0x20);
			break;
		}
		wr_val += 4;
		ddr_base += 1;
		if(((uint32_t)ddr_base & 0x00FFFFFF) == 0x00000000){
			print(" ");
			printhex((uint32_t)ddr_base);
		}
	}
	if((uint32_t)ddr_base == 0x48000000)
		print("ddr3 test 0x40000000 ~ 0x48000000 success!\r\n");

	// get_ddr3_ram_data(0x40000000, 0x40000100);
	// get_ddr3_ram_data(0x40000100, 0x40000200);
	// get_ddr3_ram_data(0x40000800, 0x40000840);
	// get_ddr3_ram_data(0x40000900, 0x40000920);
	// get_ddr3_ram_data(0x40001000, 0x40001020);
	// get_ddr3_ram_data(0x40001100, 0x40001120);
	// get_ddr3_ram_data(0x40101000, 0x40101080);
	// get_ddr3_ram_data(0x43101000, 0x43101080);

	// while(1);
lab_flash:

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

	*(uint32_t*)0x40002000 = 0x40002000; //2 1
	*(uint32_t*)0x40003000 = 0x40003000; //3 2 1
	*(uint32_t*)0x40004000 = 0x40004000; //4 3 2
	*(uint32_t*)0x40005000 = 0x40005000; //5 4 3

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
