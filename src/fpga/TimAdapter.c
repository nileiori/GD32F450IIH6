
#include "TimAdapter.h"
#include "Description.h"

#define TIMER_SUB_PRIORITY_0 0x00
#define TIMER_SUB_PRIORITY_1 0x01
#define TIMER_SUB_PRIORITY_2 0x02
#define TIMER_SUB_PRIORITY_3 0x03

Timer_Hander g_Hander_TImer2 = 0;
Timer_Hander g_Hander_TImer3 = 0;
Timer_Hander g_Hander_TImer4 = 0;
Timer_Hander g_Hander_TImer5 = 0;

//定时器2中断服务程序	 
void TIM2_IRQHandler(void)
{
	if (TIM2->SR & 0X0001)//溢出中断
	{
		if (g_Hander_TImer2) g_Hander_TImer2();
	}
	TIM2->SR &= ~(1 << 0);//清除中断标志位 	    
}

//定时器3中断服务程序	 
void TIM3_IRQHandler(void)
{
	if (TIM3->SR & 0X0001)//溢出中断
	{
		if (g_Hander_TImer3) g_Hander_TImer3();
	}
	TIM3->SR &= ~(1 << 0);//清除中断标志位 	    
}

//定时器3中断服务程序	 
void TIM4_IRQHandler(void)
{
	if (TIM4->SR & 0X0001)//溢出中断
	{
		if (g_Hander_TImer4) g_Hander_TImer4();
	}
	TIM4->SR &= ~(1 << 0);//清除中断标志位 	    
}
//定时器3中断服务程序	 
void TIM5_IRQHandler(void)
{
	if (TIM5->SR & 0X0001)//溢出中断
	{
		if (g_Hander_TImer5) g_Hander_TImer5();
	}
	TIM5->SR &= ~(1 << 0);//清除中断标志位 	    
}

void Timer_Init(_Timer_Type type, u32 interval, Timer_Hander hander)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	// TIMER2~TIMER3 APB1 is 50MHz base frequence
	TIM_TimeBaseStructure.TIM_Prescaler = 9000 - 1; //预分频器 f_clk = f_timer / psc = 90,000,000 / 9000 = 10000(10Khz 0.1ms)
	TIM_TimeBaseStructure.TIM_Period = interval - 1;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = USER_INT_PRIORITY_TIMER;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;


	switch (type)
	{
	case ETimerType_Timer2:
		g_Hander_TImer2 = hander;
		// Timer
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
		TIM_DeInit(TIM2);
		TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
		TIM_ClearFlag(TIM2, TIM_FLAG_Update);
		TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
		TIM_Cmd(TIM2, ENABLE);
		// Interrupt
		NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = TIMER_SUB_PRIORITY_0;
		break;
	case ETimerType_Timer3:
		g_Hander_TImer3 = hander;
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
		TIM_DeInit(TIM3);
		TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
		TIM_ClearFlag(TIM3, TIM_FLAG_Update);
		TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
		TIM_Cmd(TIM3, ENABLE);
		// Interrupt
		NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = TIMER_SUB_PRIORITY_1;
		break;
	case ETimerType_Timer4:
		g_Hander_TImer4 = hander;
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
		TIM_DeInit(TIM4);
		TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
		TIM_ClearFlag(TIM4, TIM_FLAG_Update);
		TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
		TIM_Cmd(TIM4, ENABLE);
		// Interrupt
		NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = TIMER_SUB_PRIORITY_2;
		break;
	case ETimerType_Timer5:
		g_Hander_TImer5 = hander;
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
		TIM_DeInit(TIM5);
		TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);
		TIM_ClearFlag(TIM5, TIM_FLAG_Update);
		TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);
		TIM_Cmd(TIM5, ENABLE);
		// Interrupt
		NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = TIMER_SUB_PRIORITY_3;
		break;
	default:
		break;
	}

	NVIC_Init(&NVIC_InitStructure);
}

