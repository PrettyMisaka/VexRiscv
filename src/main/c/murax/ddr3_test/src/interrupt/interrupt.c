#include "interrupt.h"
#include "../const.h"
#include "../uart.h"

volatile int systick_1ms_flag = 0;

inline void interruptCtrl_init(InterruptCtrl_Reg* reg)
{
	reg->MASKS = 0;
	reg->PENDINGS = 0xFFFFFFFF;
}

inline void interruptCtrl_mask_set(InterruptCtrl_Reg* reg, uint32_t mask)
{
	reg->MASKS = mask;
}

inline void interruptCtrl_clear(InterruptCtrl_Reg* reg)
{
	reg->PENDINGS = 0xFFFFFFFF;
}

inline uint32_t interruptCtrl_pendins_get(InterruptCtrl_Reg* reg)
{
	return reg->PENDINGS;
}

void systick_delayms(int ms)
{
	timer_clear(TIMER_A);
	int cnt = 0;
	systick_1ms_flag = 0;
	while(cnt != ms){
        if(systick_1ms_flag){
            cnt++;
            systick_1ms_flag = 0;
        }
	}
}
