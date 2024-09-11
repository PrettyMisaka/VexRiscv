#ifndef INTERRUPTCTRL_H_
#define INTERRUPTCTRL_H_

#include <stdint.h>

typedef struct
{
  volatile uint32_t PENDINGS;
  volatile uint32_t MASKS;
} InterruptCtrl_Reg;

static inline void interruptCtrl_init(InterruptCtrl_Reg* reg){
	reg->MASKS = 0;
	reg->PENDINGS = 0xFFFFFFFF;
}

static inline void interruptCtrl_mask_set(InterruptCtrl_Reg* reg, uint32_t mask){
	reg->MASKS = mask;
}

static inline void interruptCtrl_clear(InterruptCtrl_Reg* reg){
	reg->PENDINGS = 0xFFFFFFFF;
}

static inline uint32_t interruptCtrl_pendins_get(InterruptCtrl_Reg* reg){
	return reg->PENDINGS;
}

#endif /* INTERRUPTCTRL_H_ */
