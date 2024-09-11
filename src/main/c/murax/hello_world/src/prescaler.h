#ifndef PRESCALERCTRL_H_
#define PRESCALERCTRL_H_

#include <stdint.h>


typedef struct
{
  volatile uint32_t LIMIT;
} Prescaler_Reg;

static void prescaler_init(Prescaler_Reg* reg){

}

static void prescaler_set(Prescaler_Reg* reg, uint16_t val){
  reg->LIMIT = val;
}

#endif /* PRESCALERCTRL_H_ */
