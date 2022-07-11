
#include "ExtiAdapter.h"
#include "Description.h"

#define SENSOR_SUB_PRIORITY_0 0x00
#define SENSOR_SUB_PRIORITY_1 0x01
#define SENSOR_SUB_PRIORITY_2 0x02
#define SENSOR_SUB_PRIORITY_3 0x03


#define MAX_EXTI_HANDER 20

Exti_Hander g_exti_hander[MAX_EXTI_HANDER] = { 0x00 };

void Exti_Init(Exti_Type extiType, Exti_Hander hander)
{
	EXTI_InitTypeDef EXTI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	switch (extiType)
	{
	case EXIT_FPGA: //PE1
		/* Enable GPIOB clock */
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
		/* Enable SYSCFG clock */
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

		/* Configure PE1 pin as input floating */
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
		GPIO_Init(GPIOE, &GPIO_InitStructure);

		/* Connect EXTI Line1 to PE1 pin */
		SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource1);

		/* Configure EXTI Line4 */
		EXTI_InitStructure.EXTI_Line = EXTI_Line1;
		EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
		EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
		EXTI_InitStructure.EXTI_LineCmd = ENABLE;
		EXTI_Init(&EXTI_InitStructure);

		/* Enable and set EXTI Line1 Interrupt to the lowest priority */
		NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = USER_INT_PRIORITY_SENSOR;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = SENSOR_SUB_PRIORITY_0;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);

		break;
	default:
		break;
	}

	g_exti_hander[extiType] = hander;
}


void EXTI1_IRQHandler(void)
{
	if (EXTI_GetITStatus(EXTI_Line1) != RESET)
	{
		if (g_exti_hander[EXIT_FPGA])
		{
			g_exti_hander[EXIT_FPGA]();
		}
		EXTI_ClearITPendingBit(EXTI_Line1); //clear the flag
	}
}

