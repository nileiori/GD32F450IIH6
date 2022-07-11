#include "Description.h"
#include "string.h"

#include "AUartAdapter.h"

typedef enum
{
	UART1,
}EUartType;


#define UART_SUB_PRIORITY_0 0x00
#define UART_SUB_PRIORITY_1 0x01
#define UART_SUB_PRIORITY_2 0x02
#define UART_SUB_PRIORITY_3 0x03

#define MAX_RECV_LEN_BUFFER 255
#define MAX_SEND_LEN_BUFFER 50
#define MAX_SEND_CNT_QUEUE  10

#define UART_MAX_CHANNEL_NUMBER 1

typedef struct
{
	u8 storeIndex;
	u8 sendIndex;
	u8 resCount;
	u8 count[MAX_SEND_CNT_QUEUE];
	u8 buffer[MAX_SEND_CNT_QUEUE][MAX_SEND_LEN_BUFFER];
}TSendQueue;

typedef struct
{
	EUartType uartType;
	u8 sendBuffer[MAX_SEND_LEN_BUFFER];
	u8 recvBuffer[MAX_RECV_LEN_BUFFER];
	u8 cacheBuffer[MAX_RECV_LEN_BUFFER];
	u8 recvLength;
	u8 recvStatus;
	TSendQueue sendQueue;
}TUartData;



TUartData g_UartData;


u16 GetStopBits(EUartStopbits stopBits)
{
	uint16_t ret = USART_StopBits_1;
	switch (stopBits)
	{
	case UART_STOPBIT_ONE:
		ret = USART_StopBits_1;
		break;
	case UART_STOPBIT_TWO:
		ret = USART_StopBits_2;
		break;
	default:
		break;
	}

	return ret;
}

u16 GetParityBits(EUartParitybits parityBits)
{
	uint16_t ret = USART_Parity_No;
	switch (parityBits)
	{
	case UART_PARITY_NONE:
		ret = USART_Parity_No;
		break;
	case UART_PARITY_ODD:
		ret = USART_Parity_Odd;
		break;
	case UART_PARITY_EVEN:
		ret = USART_Parity_Even;
		break;
	default:
		break;
	}

	return ret;
}

u32 GetBaudrate(EUartBaudrate baudRate)
{
	u32 _baudRate = 115200;
	switch (baudRate)
	{
	case UART_BAUDRATE_2400BPS:
		_baudRate = 2400;
		break;
	case UART_BAUDRATE_4800BPS:
		_baudRate = 4800;
		break;
	case UART_BAUDRATE_9600BPS:
		_baudRate = 9600;
		break;
	case UART_BAUDRATE_19200BPS:
		_baudRate = 19200;
		break;
	case UART_BAUDRATE_38400BPS:
		_baudRate = 38400;
		break;
	case UART_BAUDRATE_57600BPS:
		_baudRate = 57600;
		break;
	case UART_BAUDRATE_115200BPS:
		_baudRate = 115200;
		break;
	case UART_BAUDRATE_230400BPS:
		_baudRate = 230400;
		break;
	case UART_BAUDRATE_384000BPS:
		_baudRate = 384000;
		break;
	case UART_BAUDRATE_460800BPS:
		_baudRate = 460800;
		break;
	case UART_BAUDRATE_7200BPS:
		_baudRate = 7200;
		break;
	case UART_BAUDRATE_921600BPS:
		_baudRate = 921600;
		break;
	case UART_BAUDRATE_2500000BPS:
		_baudRate = 2500000;
		break;
	default:
		break;
	}

	return _baudRate;
}

void USART1_Init(EUartBaudrate baudrate, EUartParitybits parityBits, EUartStopbits stopBits)
{
	// Config UART1 GPIO TX: PA9  RX: PA10
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// Config UART1
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	USART_InitStructure.USART_BaudRate = GetBaudrate(baudrate);
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = GetStopBits(stopBits);
	USART_InitStructure.USART_Parity = GetParityBits(parityBits);
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);

	USART_ITConfig(USART1, USART_IT_TC, DISABLE);
	USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
	USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
	USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = USER_INT_PRIORITY_UART;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = UART_SUB_PRIORITY_0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
	USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);

	USART_Cmd(USART1, ENABLE);

	// Config Send DMA
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream7_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = USER_INT_PRIORITY_UART;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = UART_SUB_PRIORITY_1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	DMA_DeInit(DMA2_Stream7);
	DMA_InitStructure.DMA_Channel = DMA_Channel_4;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&USART1->DR);
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)g_UartData[UART1].sendBuffer;
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStructure.DMA_BufferSize = 0;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA2_Stream7, &DMA_InitStructure);
	DMA_ITConfig(DMA2_Stream7, DMA_IT_TC, ENABLE);

	// Config Recv DAM
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
	DMA_DeInit(DMA2_Stream2);
	DMA_InitStructure.DMA_Channel = DMA_Channel_4;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&USART1->DR);
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)g_UartData[UART1].recvBuffer;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize = MAX_RECV_LEN_BUFFER;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA2_Stream2, &DMA_InitStructure);
	DMA_Cmd(DMA2_Stream2, ENABLE);

	memset(&g_UartData[UART1], 0, sizeof(TUartData));

}

void USART1_DMA_SendData(uint8_t* data, uint16_t size)
{
	u8 i = 0;
	u8 intCnt = size / MAX_SEND_LEN_BUFFER;
	u8 resCnt = size % MAX_SEND_LEN_BUFFER;

	TSendQueue* sendQueue = &g_UartData[UART1].sendQueue;

	if (size == 0) return;

	for (i = 0; i < intCnt; i++)
	{
		if (sendQueue->storeIndex >= MAX_SEND_CNT_QUEUE)
			sendQueue->storeIndex = 0;
		memcpy(sendQueue->buffer[sendQueue->storeIndex], data + i * MAX_SEND_LEN_BUFFER, MAX_SEND_LEN_BUFFER);
		sendQueue->count[sendQueue->storeIndex] = MAX_SEND_LEN_BUFFER;
		sendQueue->storeIndex++;
	}

	if (resCnt > 0)
	{
		if (sendQueue->storeIndex >= MAX_SEND_CNT_QUEUE)
			sendQueue->storeIndex = 0;
		memcpy(sendQueue->buffer[sendQueue->storeIndex], data + i * MAX_SEND_LEN_BUFFER, resCnt);
		sendQueue->count[sendQueue->storeIndex] = resCnt;
		sendQueue->storeIndex++;
	}

	if (sendQueue->resCount == 0)
	{
		memcpy(g_UartData[UART1].sendBuffer, sendQueue->buffer[sendQueue->sendIndex], sendQueue->count[sendQueue->sendIndex]);
		DMA_SetCurrDataCounter(DMA2_Stream7, sendQueue->count[sendQueue->sendIndex]);
		sendQueue->sendIndex++;
		DMA_Cmd(DMA2_Stream7, ENABLE);
	}

	sendQueue->resCount += intCnt + 1;
}

void DMA2_Stream7_IRQHandler(void)
{
	TSendQueue* sendQueue = &g_UartData[UART1].sendQueue;

	if (DMA_GetITStatus(DMA2_Stream7, DMA_IT_TCIF7) != RESET)
	{
		DMA_ClearFlag(DMA2_Stream7, DMA_FLAG_TCIF7);
		DMA_Cmd(DMA2_Stream7, DISABLE);
		USART_ITConfig(USART1, USART_IT_TC, ENABLE);

		if (sendQueue->resCount > 0) sendQueue->resCount--;
		if (sendQueue->sendIndex >= MAX_SEND_CNT_QUEUE) sendQueue->sendIndex = 0;

		if (sendQueue->resCount > 0)
		{
			memcpy(g_UartData[UART1].sendBuffer, sendQueue->buffer[sendQueue->sendIndex], sendQueue->count[sendQueue->sendIndex]);
			DMA_SetCurrDataCounter(DMA2_Stream7, sendQueue->count[sendQueue->sendIndex]);
			sendQueue->sendIndex++;
			DMA_Cmd(DMA2_Stream7, ENABLE);
		}
	}
}

uint8_t USART1_RX_Finish_IRQ()
{
	uint16_t len = 0;

	USART1->SR;
	USART1->DR;
	DMA_Cmd(DMA2_Stream2, DISABLE);
	DMA_ClearFlag(DMA2_Stream2, DMA_FLAG_TCIF2);

	len = MAX_RECV_LEN_BUFFER - dma_transfer_number_get(DMA0, DMA_CH7);
	memcpy(g_UartData[UART1].cacheBuffer, g_UartData[UART1].recvBuffer, len);
	DMA_SetCurrDataCounter(DMA2_Stream2, MAX_RECV_LEN_BUFFER);
	DMA_Cmd(DMA2_Stream2, ENABLE);
	return len;
}

void USART1_IRQHandler(void)
{
	if (USART_GetITStatus(USART1, USART_IT_TC) != RESET)
	{
		USART_ITConfig(USART1, USART_IT_TC, DISABLE);
	}

	if (USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)
	{
		g_UartData[UART1].recvLength = USART1_RX_Finish_IRQ();
		if (g_UartData[UART1].recvLength > 0)
		{
			g_UartData[UART1].recvStatus = 0x01;
		}
	}
}


u16 USART1_DMA_RecvData(u8* buf, u16 len)
{
	u16  recvLen = 0;

	if (g_UartData[UART1].recvStatus)
	{
		g_UartData[UART1].recvStatus = 0x00;
		recvLen = g_UartData[UART1].recvLength;
		memcpy(buf, g_UartData[UART1].recvBuffer, recvLen);
	}

	return recvLen;
}


void FUART_TxInit(EUartTxPort txPort, EUartBaudrate baudrate, EUartParitybits parityBits, EUartStopbits stopBits)
{
	if (txPort == UART_TXPORT_RS232_1)
	{
		USART1_Init(baudrate, parityBits, stopBits);
	}
}

void FUART_RxInit(EUartRxPort rxPort, EUartBaudrate baudrate, EUartParitybits parityBits, EUartStopbits stopBits)
{
	if (rxPort == UART_RXPORT_RS232_1)
	{
		USART1_Init(baudrate, parityBits, stopBits);
	}
}

void FUART_SendData(EUartTxPort txPort, uint8_t* data, uint16_t size)
{
	if (txPort == UART_TXPORT_RS232_1)
	{
		USART1_DMA_SendData(data, size);
	}
}

u16 FUART_RecvData(EUartRxPort rxPort, uint8_t* data, uint16_t size)
{
	if (rxPort == UART_RXPORT_RS232_1)
	{
		return USART1_DMA_RecvData(data, size);
	}
	return 0;
}
