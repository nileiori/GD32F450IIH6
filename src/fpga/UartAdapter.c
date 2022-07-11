
#include "UartAdapter.h"
#include "AUartAdapter.h"
#include "Description.h"
#include "FPGADefine.h"
#include "string.h"

#include "AUartAdapter.h"

// Tx-Uart
#define UART_TBASE                  (FPGA_BASE_ADDR + 0x00)
#define UART_THR(com)               (volatile unsigned short*)(UART_TBASE + 0x00 + com * 0x0010) // Tx-Uart data send macro
#define UART_TBAUD(com)             (volatile unsigned short*)(UART_TBASE + 0x02 + com * 0x0010) // Tx-Uart baudrate set macro
#define UART_TCONFIG(com)           (volatile unsigned short*)(UART_TBASE + 0x04 + com * 0x0010) // Tx-Uart config(databits, stopbits, parity) set macro
// Rx-Uart
#define UART_RBASE                  (FPGA_BASE_ADDR + 0x100)
#define UART_RHR(com)               (volatile unsigned short*)(UART_RBASE + 0x00 + com * 0x0010) // Rx-Uart data send macro
#define UART_RBAUD(com)             (volatile unsigned short*)(UART_RBASE + 0x02 + com * 0x0010) // Tx-Uart baudrate set macro
#define UART_RCONFIG(com)           (volatile unsigned short*)(UART_RBASE + 0x04 + com * 0x0010) // Rx-Uart config(databits, stopbits, parity) set macro
#define UART_RFIFO_STATE(com)       (volatile unsigned short*)(UART_RBASE + 0x0c + com * 0x0010) // Rx-Uart FOFI state macro
// Mode choose
#define UART_MODE_SEL               (volatile unsigned short*)(UART_RBASE + 0x232) // R232/422 Choose

/********************************************************************************\
功能：接收串口初始化
输入：rxPort - 串口号
     baudRate - 波特率
     parityBits - 校验位
     stopBits - 停止位
     mode - RS232/422/485选择
     enable - 串口使能
返回：none
\********************************************************************************/
void Uart_RxInit(EUartRxPort rxPort, EUartBaudrate baudRate, EUartParitybits parityBits, EUartStopbits stopBits, EUartMode mode, EUartEnable enable)
{
    UCHAR sel = 0x00;
#ifndef WIN32
    if (UART_RXPORT_NULL == rxPort) return;
//    if (UART_RXPORT_RS232_1 == rxPort)
//    {
//        FUART_RxInit(rxPort, baudRate, parityBits, stopBits);
//        return;
//    }

    if (rxPort >= UART_RXPORT_COMPLEX_8)
    {
        sel = (rxPort - UART_RXPORT_COMPLEX_8) / 2;
        if (mode == UART_RS422)
        {
            *(UART_MODE_SEL) |= (1 << sel); // 422模式           
            *(UART_MODE_SEL) &= ~(1 << (sel + 4)); // 全双工模式
        }
        else if (mode == UART_RS232)
        {
            *(UART_MODE_SEL) &= ~(1 << sel); // 232模式
            *(UART_MODE_SEL) &= ~(1 << (sel + 4)); // 全双工
        }
        else if (mode == UART_RS485)
        {
            *(UART_MODE_SEL) |= (1 << sel); // 422
            *(UART_MODE_SEL) |= (1 << (sel + 4)); // 全双工
        }
        else {}
    }
    //清除接收缓冲区
    *(UART_RHR(rxPort)) = 0x0001;
    // 设置波特率
    *(UART_RBAUD(rxPort)) = baudRate;
    // 设置配置(校验位/停止位/使能)
    *(UART_RCONFIG(rxPort)) = parityBits | stopBits | enable;
#endif
}

/********************************************************************************\
功能：发送串口初始化
输入：rxPort - 串口号
     baudRate - 波特率
     parityBits - 校验位
     stopBits - 停止位
     mode - RS232/422/485选择
     enable - 串口使能
返回：none
\********************************************************************************/
void Uart_TxInit(EUartTxPort txPort, EUartBaudrate baudRate, EUartParitybits parityBits, EUartStopbits stopBits, EUartMode mode, EUartEnable enable)
{
    UCHAR sel = 0x00;
#ifndef WIN32
    if (UART_TXPORT_NULL == txPort) return;
//    if (UART_TXPORT_RS232_1 == txPort)
//    {
//        FUART_TxInit(txPort, baudRate, parityBits, stopBits);
//        return;
//    }

    if (txPort >= UART_RXPORT_COMPLEX_8)
    {
        sel = (txPort - UART_RXPORT_COMPLEX_8) / 2;
        if (mode == UART_RS422)
        {
            *(UART_MODE_SEL) |= (1 << sel); // 422模式          
            *(UART_MODE_SEL) &= ~(1 << (sel + 4)); //全双工模式
        }
        else if (mode == UART_RS232)
        {
            *(UART_MODE_SEL) &= ~(1 << sel); 
            *(UART_MODE_SEL) &= ~(1 << (sel + 4)); 
        }
        else if (mode == UART_RS485)
        {
            *(UART_MODE_SEL) |= (1 << sel); // 
            *(UART_MODE_SEL) |= (1 << (sel + 4)); // 全双工模式
        }
        else {}
    }
    // 设置波特率
    *(UART_TBAUD(txPort)) = baudRate;
    //设置配置(校验位/停止位/使能)
    *(UART_TCONFIG(txPort)) = parityBits | stopBits | enable;
#endif
}

/********************************************************************************\
功能：发送串口数据
����: txPort - 串口号
     buffer - 数据信息
     start -帧起始
     len - 帧长
����: ��
\********************************************************************************/
void Uart_SendMsg(EUartTxPort txPort, USHORT start, USHORT len, UCHAR* buffer)
{
#ifndef WIN32
    USHORT i = 0;

    if (UART_TXPORT_NULL == txPort) return;
//    if (UART_TXPORT_RS232_1 == txPort)
//    {
//        FUART_SendData(txPort, buffer + start, len);
//        return;
//    }

    for (i = start; i < len; i++)
    {
        *(UART_THR(txPort)) = *(buffer + i);
    }
#endif
}


/********************************************************************************\
����: ���մ�������
����: rxPort - ���ں�
     buffer - ������Ϣ
     len - ��Ҫ���յĳ���
����: ʵ�ʶ�ȡ����
\********************************************************************************/
USHORT Uart_RecvMsg(EUartRxPort rxPort, USHORT len, UCHAR* buffer)
{
#ifndef WIN32
    USHORT revLen = 0;
    USHORT i = 0;

    if (UART_RXPORT_NULL == rxPort) return 0;
//    if (UART_RXPORT_RS232_1 == rxPort)
//    {
//        return FUART_RecvData(rxPort, buffer, len);
//    }

    revLen = *(UART_RFIFO_STATE(rxPort));

    if (len < revLen) revLen = len;

    for (i = 0; i < revLen; i++)
    {
        buffer[i] = (UCHAR)*(UART_RHR(rxPort));
    }

    return revLen;
#else
    return 0;
#endif
}

/********************************************************************************\
����: ��ս��ջ�����
����: rxPort - ���ں�
����: ��
\********************************************************************************/
void Uart_ClearRecvBuffer(EUartRxPort rxPort)
{
#ifndef WIN32
    if (UART_RXPORT_NULL == rxPort) return;
    *(UART_RHR(rxPort)) = 0x0001;
#endif
}

