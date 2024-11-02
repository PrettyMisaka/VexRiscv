#ifndef TIMERCTRL_H_
#define TIMERCTRL_H_

#include <stdint.h>


typedef struct
{
  volatile uint32_t CLEARS_TICKS; //0
  volatile uint32_t LIMIT;        //4
  volatile uint32_t VALUE;        //8
  volatile uint32_t AUTORELOAD;   //c
} Timer_Reg;

static inline void timer_init(Timer_Reg *reg){
	reg->CLEARS_TICKS  = 0;
	reg->VALUE = 0;
}

static inline void timer_clear(Timer_Reg *reg){
	reg->VALUE = 0;
}

static inline void timer_ticks_en(Timer_Reg *reg){
	reg->CLEARS_TICKS = 2;
}

static inline void timer_autoreload_en(Timer_Reg *reg){
	reg->AUTORELOAD = 1;
}

static inline void timer_limit_set(Timer_Reg *reg, uint16_t val){
	reg->LIMIT = val;
}

static inline uint32_t timer_value_get(Timer_Reg *reg){
	return reg->VALUE;
}


#endif /* TIMERCTRL_H_ */
