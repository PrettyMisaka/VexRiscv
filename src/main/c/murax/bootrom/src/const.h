#ifndef __CONST_H__
#define __CONST_H__

#include "stdint.h"

#define GPIO_A          ((Gpio_Reg*)(0xF0000000))

#define TIMER_PRESCALER ((Prescaler_Reg*)(0xF0020000))
#define TIMER_INTERRUPT ((InterruptCtrl_Reg*)(0xF0020010))
#define TIMER_A         ((Timer_Reg*)(0xF0020040))
#define TIMER_B         ((Timer_Reg*)(0xF0020050))

#define UART            ((Uart_Reg*)(0xF0010000))
#define SPI             ((Spi_Reg*)(0xF0030000))

#endif