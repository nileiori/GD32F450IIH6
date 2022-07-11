
#include "Description.h"
#include "CanAdapter.h"
#include "string.h"

Can_IntHandler g_can_IntHandler[2] = { 0 };
TCanConfig g_can1_config;
TCanConfig g_can2_config;

void Can_GPIO_Setup()
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	/* Enable GPIO clock */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	/* Connect CAN pins to AF9 */
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_CAN1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF_CAN1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_CAN2);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource12, GPIO_AF_CAN2);

	/* Configure CAN RX and TX pins */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_6 | GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void Can_Config_Setup(ECanChannel ch, TCanConfig config)
{

	int i = 0;

	CAN_InitTypeDef CAN_InitStructure;
	CAN_FilterInitTypeDef  CAN_FilterInitStructure;
	CAN_TypeDef* can;
	NVIC_InitTypeDef  NVIC_InitStructure;

	if (ch == CAN_1)
	{
		can = CAN1;
	}
	else
	{
		can = CAN2;
	}

	if (CAN1 == can)
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
	}
	else
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN2, ENABLE);
	}

	CAN_DeInit(can);

	/* CAN cell init */
	CAN_InitStructure.CAN_TTCM = DISABLE;
	CAN_InitStructure.CAN_ABOM = DISABLE;
	CAN_InitStructure.CAN_AWUM = DISABLE;
	CAN_InitStructure.CAN_NART = DISABLE;
	CAN_InitStructure.CAN_RFLM = DISABLE;
	CAN_InitStructure.CAN_TXFP = DISABLE;
	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;
	CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;

	/* CAN Baudrate = 1 MBps (CAN clocked at 45 MHz APB1) */
	CAN_InitStructure.CAN_BS1 = CAN_BS1_6tq;
	CAN_InitStructure.CAN_BS2 = CAN_BS2_8tq;
	CAN_InitStructure.CAN_Prescaler = 3;
	CAN_Init(can, &CAN_InitStructure);

	/* CAN filter init */



	if (config.filterUsedNum > 0)
	{
		CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
		CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
		CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;
		CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;

		for (i = 0; i < config.filterUsedNum; i++)
		{
			if (can == CAN1)
			{
				CAN_FilterInitStructure.CAN_FilterNumber = config.filter[i].filterNo;
			}
			else
			{
				CAN_FilterInitStructure.CAN_FilterNumber = 14 + config.filter[i].filterNo;
			}

			CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;
			CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;
			CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;
			CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;

			if (config.filter[i].idType == IDType_Standard)
			{
				CAN_FilterInitStructure.CAN_FilterIdHigh = (config.filter[i].filterID << 5) & 0xFFFF;
				CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;
				CAN_FilterInitStructure.CAN_FilterIdLow &= ~(1 << 2);
				CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0xFFE0;
				CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0006;
			}
			else
			{
				CAN_FilterInitStructure.CAN_FilterIdHigh = (config.filter[i].filterID >> 13) & 0xFFFF;
				CAN_FilterInitStructure.CAN_FilterIdLow = (config.filter[i].filterID << 3) & 0xFFFF;
				CAN_FilterInitStructure.CAN_FilterIdLow |= (1 << 2);
				CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0xFFFF;
				CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0xFFFE;
			}

			if (config.filter[i].frameType == FrameType_Remote)
			{
				CAN_FilterInitStructure.CAN_FilterIdLow |= (1 << 1);
			}
			else
			{
				CAN_FilterInitStructure.CAN_FilterIdLow &= ~(1 << 1);
			}

			CAN_FilterInit(&CAN_FilterInitStructure);

		}
	}
	else
	{
		if (can == CAN1)
		{
			CAN_FilterInitStructure.CAN_FilterNumber = 0;
		}
		else
		{
			CAN_FilterInitStructure.CAN_FilterNumber = 14;
		}

		CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
		CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
		CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;
		CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;
		CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;
		CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;
		CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;
		CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
		CAN_FilterInit(&CAN_FilterInitStructure);
	}




	if (config.bUseInterrupt)
	{
		CAN_ITConfig(can, CAN_IT_FMP0, ENABLE);
		if (can == CAN1)
			NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX0_IRQn;
		else
			NVIC_InitStructure.NVIC_IRQChannel = CAN2_RX0_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = USER_INT_PRIORITY_OTHER;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
	}


}

void Can_FormatFilter(TFilter* filter, u8 num, ECanFrameType frameType, ECanIdType idType, u32 id)
{
	filter->filterNo = num;
	filter->idType = idType;
	filter->frameType = frameType;
	filter->filterID = id;
}

bool Can_Init(void)
{
	TCanConfig config;
	// Initialize GPIO
	Can_GPIO_Setup();
	// Initialize CAN1
	config.ch = CAN_1;
	config.filterUsedNum = 2;
	config.bUseInterrupt = false;
	// Filter 
	Can_FormatFilter(&config.filter[0], 0, FrameType_Data, IDType_Extend, 0x12);
	Can_FormatFilter(&config.filter[1], 1, FrameType_Data, IDType_Standard, 0x18);

	Can_Config_Setup(CAN_1, config);

	//Initialize CAN2
	config.ch = CAN_2;
	config.filterUsedNum = 3;
	config.bUseInterrupt = false;
	// Filter
	Can_FormatFilter(&config.filter[0], 0, FrameType_Data, IDType_Extend, 0x12);
	Can_FormatFilter(&config.filter[1], 1, FrameType_Data, IDType_Standard, 0x18);
	Can_FormatFilter(&config.filter[2], 2, FrameType_Data, IDType_Extend, 0x1ABB);

	Can_Config_Setup(CAN_2, config);

	return true;
}

bool Can_BindHander(ECanChannel ch, Can_IntHandler hander)
{
	g_can_IntHandler[ch] = hander;
	return true;
}

bool Can_SetConfig(ECanChannel ch, TCanConfig config)
{
	Can_Config_Setup(ch, config);
	return true;
}

bool Can_Config(Can_IntHandler hander, TCanConfig config)
{
	Can_Config_Setup(config.ch, config);
	g_can_IntHandler[config.ch] = hander;
	return true;
}

bool Can_SendData(ECanChannel ch, TCanData data)
{
	CanTxMsg txMsg;

	if (data.idType == IDType_Standard)
	{
		txMsg.IDE = CAN_Id_Standard;
		txMsg.StdId = data.id;
	}
	else
	{
		txMsg.IDE = CAN_Id_Extended;
	}

	if (data.frameType == FrameType_Data)
	{
		txMsg.RTR = CAN_RTR_Data;
		txMsg.ExtId = data.id;
	}
	else
	{
		txMsg.RTR = CAN_RTR_Remote;
	}

	txMsg.DLC = data.len;
	memcpy(txMsg.Data, data.data, 8);

	if (ch == CAN_1)
	{
		CAN_Transmit(CAN1, &txMsg);
	}
	else
	{
		CAN_Transmit(CAN2, &txMsg);
	}

	return true;
}

void Can_FormatData(CanRxMsg rxMsg, TCanData* data)
{
	data->len = rxMsg.DLC;
	memcpy(data->data, rxMsg.Data, 8);

	if (rxMsg.IDE == CAN_Id_Standard)
	{
		data->id = rxMsg.StdId;
		data->idType = IDType_Standard;
	}
	else
	{
		data->id = rxMsg.ExtId;
		data->idType = IDType_Extend;
	}

	if (rxMsg.RTR == CAN_RTR_Data)
	{
		data->frameType = FrameType_Data;
	}
	else
	{
		data->frameType = FrameType_Remote;
	}
}

bool Can_RecvData(ECanChannel ch, TCanData* data)
{
	CanRxMsg rxMsg;
	bool retVal = false;

	if (ch == CAN_1)
	{
		if (CAN_GetFlagStatus(CAN1, CAN_FLAG_FMP0) == SET)
		{
			CAN_Receive(CAN1, 0, &rxMsg);
			Can_FormatData(rxMsg, data);
			retVal = true;
		}
	}
	else
	{
		if (CAN_GetFlagStatus(CAN2, CAN_FLAG_FMP0) == SET)
		{
			CAN_Receive(CAN2, 0, &rxMsg);
			Can_FormatData(rxMsg, data);
			retVal = true;
		}
	}
	return retVal;
}



void CAN1_RX0_IRQHandler()
{
	CanRxMsg rxMsg;
	TCanData data;

	if (CAN_GetFlagStatus(CAN1, CAN_FLAG_FMP0) == SET)
	{
		CAN_Receive(CAN1, 0, &rxMsg);
		Can_FormatData(rxMsg, &data);
		if (g_can_IntHandler[CAN_1])
		{
			g_can_IntHandler[CAN_1](data);
		}
	}
}


void CAN2_RX0_IRQHandler()
{
	CanRxMsg rxMsg;
	TCanData data;

	if (CAN_GetFlagStatus(CAN2, CAN_FLAG_FMP0) == SET)
	{
		CAN_Receive(CAN2, 0, &rxMsg);
		Can_FormatData(rxMsg, &data);
		if (g_can_IntHandler[CAN_2])
		{
			g_can_IntHandler[CAN_2](data);
		}
	}
}
