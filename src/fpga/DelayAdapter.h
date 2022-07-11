
#ifndef __DELAY_ADAPTER_H
#define __DELAY_ADAPTER_H

#include "stm32f4xx.h"


extern void Delay_Init(void);
extern void Delay_us(uint64_t nus);
extern void Delay_ms(uint64_t nms);
extern void Delay_s(uint64_t ns);

#endif

