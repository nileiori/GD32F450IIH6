/********************************************************************************\
* timer.h	  	v1.00
* Copyright	2019 by ThinkStrapDownScience Technology Company
* All rights reserved. Property of ThinkStrapDownScience Technology Company
* Designed by: Enolly
* Data: 2019.1.9
* Decription: Only define the common timer 2~5
\********************************************************************************/


#ifndef __TIM_ADAPTER_H
#define __TIM_ADAPTER_H

#include "stm32f4xx.h"


typedef enum ETimerType
{
    ETimerType_Timer2,
    ETimerType_Timer3,
    ETimerType_Timer4,
    ETimerType_Timer5,

}_Timer_Type;

typedef void (*Timer_Hander)(void);

extern void Timer_Init(_Timer_Type type, u32 interval, Timer_Hander hander); 


#endif

