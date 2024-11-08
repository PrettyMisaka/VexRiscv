#include "uart.h"
#include "gpio.h"

extern void crtStartRam(void);
extern void crtStartDDR3(void);
extern void trap_entry(void);

void trap_handler(void)
{
	UART->STATUS &= ~0x2;

    static uint8_t uart_rx_buf[5] = {0,0,0,0,0};
    static uint32_t *puart_rx_val = (uint32_t*)uart_rx_buf;
    static char i = 4;
    static uint8_t uart_buf_len = 0;

    //max len       0x8000000
    //write addr    0x80000000 | addr
    //write len     0x40000000 | len
    //set succ -> tx addr+len

    static uint8_t status = 0;
    static uint32_t *burning_base_addr = (uint32_t *)0x80000000;
    static uint32_t burning_len = 0;

	do{
		uart_rx_buf[i-1] = UART->DATA;
		i--;
        if(i == 0){
            i = 4;
            switch (status){
            case 0:
                uint32_t addr_head = *puart_rx_val & 0xF0000000;
                if(addr_head){
                    burning_base_addr = (uint32_t *)(*puart_rx_val);
                    status = 1;
                }else
                    printu32(0xfffffff8);
                break;
            case 1:
                if(*puart_rx_val & 0x40000000){
                    burning_len = *puart_rx_val & 0x0fffffff;
                    status = 2;
                    printu32(burning_len + (uint32_t)burning_base_addr);
                    burning_len = burning_len >> 2;
                }else{
                    status = 0;
                    printu32(0xfffffff4);
                }
                printu32((uint32_t)burning_base_addr);
                printu32((uint32_t)burning_len);
                break;
            case 2:
                *burning_base_addr = *puart_rx_val;
                burning_base_addr += 1;
                burning_len--;
                if(burning_len == 0){
                    status = 0;
                    printu32(0xffffffff);
                    // printu32((uint32_t)burning_base_addr);
                }
                break;
            default:
                break;
            }
        }

		uart_buf_len = uart_readOccupancy(UART);

	}while(uart_buf_len > 0);


	UART->STATUS |= 0x2;
}

void main()
{
    GPIO_A->OUTPUT_ENABLE = 0xF0;
    print("[boot]");
    if(GPIO_A->INPUT & 0x80000000){
        print("enter the burning mode!\r\n");

        GPIO_A->OUTPUT = 0xF0;

        asm volatile("csrw mtvec, %0" : : "r" ((uint32_t)trap_entry));

        //开UART中断
        UART->STATUS |= 0x2;

        while(1);
    }
    
    GPIO_A->OUTPUT = 0x00;


    print("enter the ");
    if(GPIO_A->INPUT & 0x40000000){
        print("ram!\r\n");
        asm volatile("csrw mtvec, %0" : : "r" ((uint32_t)0x80000020));
        crtStartRam();
    } else{
        print("dram!\r\n");
        asm volatile("csrw mtvec, %0" : : "r" ((uint32_t)0x40030020));
        crtStartDDR3();
    }
}