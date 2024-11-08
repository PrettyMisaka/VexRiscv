//#include "stddefs.h"
// #include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "murax.h"
#include "uart_ring/uart_ring.h"
#include "sdcard/bsp_spi_sdcard.h"
#include "sdcard/sdcard_test.h"
#include "pff3a/pff.h"
#include "pff3a/pff_user.h"

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

static uint8_t cmd_ls_flag = 0;
void cmd_ls(){
	cmd_ls_flag = 1;
}

static uint8_t cmd_cd_flag = 0;
static uint8_t cmd_cd_str[20];
void cmd_cd(){
	cmd_cd_flag = 1;
	char *p = ringGetString(0);
	int n = strlen(p);
	if(p[n-1] == '\r'){
		memcpy(cmd_cd_str,p,n-1);
		cmd_cd_str[n-1] = 0;
	}else{
		memcpy(cmd_cd_str,p,n);
		cmd_cd_str[n] = 0;
	}
}

static uint8_t cmd_load_flag = 0;
static uint8_t cmd_load_str[20];
void cmd_load(){
	cmd_load_flag = 1;
	char *p = ringGetString(0);
	int n = strlen(p);
	if(p[n-1] == '\r'){
		memcpy(cmd_load_str,p,n-1);
		cmd_load_str[n-1] = 0;
	}else{
		memcpy(cmd_load_str,p,n);
		cmd_load_str[n] = 0;
	}
}

extern void crtStartDDR3(void);
void cmd_start_ddr3(){
	UART->STATUS &= ~0x2;
	interruptCtrl_mask_set(TIMER_INTERRUPT,0x0);
    println("enter the dram!");
	asm volatile("csrw mtvec, %0" : : "r" ((uint32_t)0x40030020));
	crtStartDDR3();
}

FRESULT sdret;                /* 文件操作结果 */
FATFS fs;					  /* FatFs文件系统对象 */
DIR dir;
FILINFO dir_info;
char current_path[30] = "/";
char target_path[30] = "/";

void main() {
	asm volatile("csrw mtvec, %0" : : "r" ((uint32_t)trap_entry));
    println("hello world tang primer 20k!");
	UART->STATUS |= 0x2;
    GPIO_A->OUTPUT_ENABLE = 0x0000000F;
	GPIO_A->OUTPUT = 0x00000000;
    const int nleds = 4;
	const int nloops = 100000;
	_timer_init();
	ringBuff_Total_Init(puartRingParameter);
	ring_state ret = cmdBuff_Push( puartRingParameter,"helloworld STRING\n",cmd_helloworld);
	cmdBuff_Push( puartRingParameter,"memory-test\r\n",cmd_ddr3_test);
	cmdBuff_Push( puartRingParameter,"memory-log STRING\n",cmd_ddr3_mem_log);
	cmdBuff_Push( puartRingParameter,"hdmi-flush\r\n",cmd_hdmi_flush);
	cmdBuff_Push( puartRingParameter,"help\r\n",cmd_help);
	cmdBuff_Push( puartRingParameter,"ls\r\n",cmd_ls);
	cmdBuff_Push( puartRingParameter,"cd STRING\n",cmd_cd);
	cmdBuff_Push( puartRingParameter,"load STRING\n",cmd_load);
	cmdBuff_Push( puartRingParameter,"start-ddr3\r\n",cmd_start_ddr3);

	int i = 0;
	*(uint32_t*)0x48001000 = 0xffffffff; //2 1
	// systick_delayms(100);
	// sd_card_init();
	println("done");

	sdret = pf_mount(&fs);
	if(sdret == FR_NO_FILESYSTEM){
		println("FR_NO_FILESYSTEM");
	}else if(sdret!=FR_OK){
		println("FR_ERR");
	}else{
		println("pf_mount success!");
	}

	println("enter ringBuffHandleFun");
	while(1){
		ringBuffHandleFun(puartRingParameter);
		systick_delayms(50);
		GPIO_A->OUTPUT = ~GPIO_A->OUTPUT;
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
		if(cmd_ls_flag){
			cmd_ls_flag = 0;
			sdret = pf_opendir(&dir,current_path);
			if(sdret != FR_OK){
				print("opendir ");
				print(current_path);
				print(" err:");
				printhex(sdret);
				println("");
			}else{
				print(current_path);
				println(":");
				while(1){
					sdret = pf_readdir(&dir,&dir_info);
					if(sdret != FR_OK || dir_info.fname[0] == 0){
						break;
					}else{
						print(dir_info.fname);
						if (dir_info.fattrib & AM_DIR) {
							print("/");
						}
						print(" ");
					}
				}
				println("");
			}
		}
		if(cmd_cd_flag){
			cmd_cd_flag = 0;
			int n = strlen(target_path);
			if(strncmp(cmd_cd_str,"../",3)==0){
				if(n == 1 && target_path[0] == '/')
					goto lab_log_current_path;

				do{
					n--;
				}while(target_path[n] != '/');
				target_path[n] = 0;

				if(n == 0){
					target_path[n] = '/';
					target_path[n+1] = 0;
					n++;
				}

				strncpy(current_path,target_path,n+2);
			}else{
				int s_n = strlen(cmd_cd_str);

				if(n != 1 || target_path[0] != '/'){
					target_path[n] = '/';
					n++;
				}

				memcpy(&target_path[n],cmd_cd_str,s_n);
				target_path[s_n + n] = 0;

				sdret = pf_opendir(&dir,target_path);
				if(sdret != FR_OK){
					print("error path! code:");
					printhex(sdret);
					println("");
					println(target_path);

					strncpy(target_path,current_path,n+1);
					continue;
				}

				strncpy(current_path,target_path,s_n+n+1);
			}
lab_log_current_path:
			print("cd ");
			println(current_path);
		}
		if(cmd_load_flag){
			cmd_load_flag = 0;
			int n = strlen(target_path);

			if(n != 1 || target_path[0] != '/'){
				target_path[n] = '/';
				n++;
			}

			int s_n = strlen(cmd_load_str);
			memcpy(&target_path[n],cmd_load_str,s_n);
			target_path[s_n + n] = 0;

			sdret = pf_open(target_path);
			if(sdret != FR_OK){
				print("error path! code:");
				printhex(sdret);
				println("");
				println(target_path);

				goto lab_load_restore_path;
			}
			print("open ");
			print(target_path);
			println(" success!");
			pf_loadfile2dram();

			*(uint32_t*)0x47ff1000 = 0xffffffff;
			*(uint32_t*)0x47ff2000 = 0xffffffff;
			*(uint32_t*)0x47ff3000 = 0xffffffff;
			*(uint32_t*)0x47ff4000 = 0xffffffff;

lab_load_restore_path:
			strncpy(target_path,current_path,s_n+n+1);
		}
	};
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
