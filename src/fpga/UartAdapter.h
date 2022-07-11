
#ifndef _UART_ADAPTER_H_
#define _UART_ADAPTER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdbool.h"
#include "gd32f4xx.h"

#include "UartDefine.h"

extern void Uart_RxInit(EUartRxPort rxPort, EUartBaudrate baudRate, EUartParitybits parityBits, EUartStopbits stopBits, EUartMode mode, EUartEnable enable);
extern void Uart_TxInit(EUartTxPort txPort, EUartBaudrate baudRate, EUartParitybits parityBits, EUartStopbits stopBits, EUartMode mode, EUartEnable enable);
extern void Uart_SendMsg(EUartTxPort txPort, USHORT start, USHORT len, UCHAR* buffer);
extern uint16_t  Uart_RecvMsg(EUartRxPort rxPort, USHORT len, UCHAR* buffer);
extern void Uart_ClearRecvBuffer(EUartRxPort rxPort);

#ifdef __cplusplus
}
#endif


#endif

