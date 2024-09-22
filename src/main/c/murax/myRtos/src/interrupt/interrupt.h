#ifndef INTERRUPTCTRL_H_
#define INTERRUPTCTRL_H_

#include <stdint.h>
#include "../timer.h"

extern volatile int systick_1ms_flag;

typedef struct
{
  volatile uint32_t PENDINGS;
  volatile uint32_t MASKS;
} InterruptCtrl_Reg;

void interruptCtrl_init(InterruptCtrl_Reg* reg);
void interruptCtrl_mask_set(InterruptCtrl_Reg* reg, uint32_t mask);
void interruptCtrl_clear(InterruptCtrl_Reg* reg);
uint32_t interruptCtrl_pendins_get(InterruptCtrl_Reg* reg);

void systick_delayms(int ms);

#endif /* INTERRUPTCTRL_H_ */
