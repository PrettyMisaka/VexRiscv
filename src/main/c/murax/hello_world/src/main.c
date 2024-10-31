//#include "stddefs.h"
// #include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "murax.h"
#include "uart_ring/uart_ring.h"
extern void trap_entry(void);
	
	#define HDMI_H 1280l
	#define HDMI_W 720l
	#define HDMI_BLOCK 80
	//					      384000
	#define HDMI_FB_BASE (0x47c00000-160+20)
	// #define HDMI_FB_BASE (0x47c00000-16)

void hdmi_fill(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color){
	uint32_t *p = (uint32_t*)(HDMI_FB_BASE+(x+(y*HDMI_H)<<2));
	for(int i = 0; i < h; i++){
		for(int j = 0; j < w; j++){
			*p = color;
			p++;
		}
		p += (HDMI_H - w);
	}
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

static uartRingParameterTypedef uartRingParameter, *puartRingParameter = &uartRingParameter;

char cmd_helloworld_str[21];
void cmd_helloworld(){
	println("hello world!");
	char *p = ringGetString(0);
	memcpy(cmd_helloworld_str,p,20);
	cmd_helloworld_str[21] = 0;
	println(cmd_helloworld_str);
	// print(" to ");
	// p = ringGetString(1);
	// memcpy(cmd_helloworld_str,p,20);
	// println(cmd_helloworld_str);
}


static uint8_t cmd_ddr3_test_flag = 0;
void cmd_ddr3_test(){
	cmd_ddr3_test_flag = 1;
}

static uint8_t cmd_ddr3_mem_log_flag = 0;
static uint32_t cmd_ddr3_mem_log_begin_addr = 0x40030000;
void cmd_ddr3_mem_log(){
	cmd_ddr3_mem_log_flag = 1;
	char *p = ringGetString(0);
	uint32_t _tmp = 0;
	for(int i = 0; i < 8; i++){
		uint8_t u8;
		if(*p>='0'&&*p<='9'){
			u8 = (*p - '0');
		}else if(*p>='a'&&*p<='f'){
			u8 = (*p - 'a' + 10);
		}else{
			return;
		}
		_tmp += u8 << ((7-i)*4);
		p++;
	}
	cmd_ddr3_mem_log_begin_addr = _tmp;
}

static uint8_t cmd_hdmi_flush_flag = 0;
void cmd_hdmi_flush(){
	cmd_hdmi_flush_flag = 1;
}

static uint8_t cmd_help_flag = 0;
void cmd_help(){
	cmd_help_flag = 1;
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
	ringBuff_Total_Init(puartRingParameter);
	ring_state ret = cmdBuff_Push( puartRingParameter,"helloworld STRING\n",cmd_helloworld);
	cmdBuff_Push( puartRingParameter,"memory-test\r\n",cmd_ddr3_test);
	cmdBuff_Push( puartRingParameter,"memory-log STRING\n",cmd_ddr3_mem_log);
	cmdBuff_Push( puartRingParameter,"hdmi-flush\r\n",cmd_hdmi_flush);
	cmdBuff_Push( puartRingParameter,"help\r\n",cmd_help);

	int i = 0;
	*(uint32_t*)0x48001000 = 0xffffffff; //2 1
	systick_delayms(100);
	println("done");
	println("enter ringBuffHandleFun");
	while(1){
		ringBuffHandleFun(puartRingParameter);
		systick_delayms(50);
		if(cmd_ddr3_test_flag){
			cmd_ddr3_test_flag = 0;
			volatile uint32_t* ddr_base;
			volatile uint32_t wr_val = 0x40000000;
			println("memory-test start!");

			for( ddr_base = (uint32_t*)0x40030000,wr_val = (uint32_t)0; (uint32_t)ddr_base < 0x48000000;){
				*ddr_base = wr_val;
				// *ddr_base = (((uint32_t)ddr_base & 0xfffffff0) == 0x400007F0)?wr_val|0x80000000:wr_val;
				ddr_base += 1;
				if(((uint32_t)ddr_base & 0x00FFFFFF) == 0x00000000){
					print(" ");
					printhex((uint32_t)ddr_base);
				}
			}
			println("");
			println("set 0x40030000 ~ 0x48000000 val 0");

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
			println("");
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
			println("");
			if((uint32_t)ddr_base == 0x48000000)
				print("ddr3 test 0x40000000 ~ 0x48000000 success!\r\n");
		}
		if(cmd_ddr3_mem_log_flag){
			cmd_ddr3_mem_log_flag = 0;
			volatile uint32_t* ddr_base;
			uint32_t cnt = 0;
			for( ddr_base = (uint32_t*)cmd_ddr3_mem_log_begin_addr; (uint32_t)ddr_base < cmd_ddr3_mem_log_begin_addr+0x1000;){
				if((cnt & 0x7) == 0){
					print("0x");
					printhex((uint32_t)ddr_base);
					print(":");
				}
				printhex(*ddr_base);
				print(" ");
				if((cnt & 0x7) == 0x7){
					println("");
				}
				cnt++;
				// *ddr_base = (((uint32_t)ddr_base & 0xfffffff0) == 0x400007F0)?wr_val|0x80000000:wr_val;
				ddr_base += 1;
			}
		}
		if(cmd_hdmi_flush_flag){
			cmd_hdmi_flush_flag = 0;

			volatile uint32_t* ddr_base;
			volatile uint32_t wr_val = 0x40000000;

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

			// while(1){
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
			// }

		}
		if(cmd_help_flag){
			cmd_help_flag = 0;
			println(">cmd:");
			for(int i = 0; i < puartRingParameter->cmdBuffLength; i++){
				print("    ");
				print(puartRingParameter->cmdBuffArr[i].cmd_String);
				if(puartRingParameter->cmdBuffArr[i].dataTypeArr[0] != RING_DATATYPE_NULL){
					println("");
				}
			}
		}
	};

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

static char ringBuffStr[RING_BUFF_LENGTH];

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

	uint32_t rxbuf_len;

	do{
		ringBuff_Push(puartRingParameter,UART->DATA);

		rxbuf_len = uart_readOccupancy(UART);
	}while(rxbuf_len > 0);

	// ringShowBuff(puartRingParameter,ringBuffStr);
	// print("ring buff:");
	// println(ringBuffStr);
	
	UART->STATUS |= 0x2;
}
