
#ifndef __EXTI_ADAPTER_H
#define __EXTI_ADAPTER_H

#include "stm32f4xx.h"


typedef enum EEXTI_Type
{
    EXIT_FPGA = 0,
    
}Exti_Type;

typedef void (*Exti_Hander)(void);

extern void Exti_Init(Exti_Type extiType, Exti_Hander hander);

#endif


