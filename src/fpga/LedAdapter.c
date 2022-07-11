#include "LedAdapter.h"

void Led_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure; 
    
    /* GPIOC Peripheral clock enable */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

    /* Configure PC7 in output pushpull mode */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	/* Configure PC8 in output pushpull mode */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    
}

void Led_Ctrl(ELedType type, LedState onoff)
{ 
    if(LED_1 == type)
    {
        GPIO_WriteBit(GPIOC, GPIO_Pin_7, (BitAction)onoff);
    }
    
}

void RS485_Ctrl(BitAction onoff)
{ 
    GPIO_WriteBit(GPIOC, GPIO_Pin_8, (BitAction)onoff);    
}

void Led_Blink(ELedType type)
{
    static u8 bink_state_led1 = 0x00;  
    if(LED_1 == type)
    {
        bink_state_led1 = (~bink_state_led1)&0x01;
        GPIO_WriteBit(GPIOC, GPIO_Pin_7, (BitAction)bink_state_led1);        
    }
}

