#ifndef __UDP_ADAPTER_H
#define __UDP_ADAPTER_H

#include "stm32f4xx.h"

typedef struct TNETPARA
{
    u8 macAddress[6];
    u8 ipAddress[4];
    u8 subMask[4];
    u8 gateWay[4];
    u8 dnsAddress[4];
    
    u8 socketNum;
    u16 portNum;

}TNetPara;

typedef struct TUDPDATA
{
    u8 ipAddress[4];
    u16 portNum;
    u8 data[1024];
    u16 len;
    
}TUdpData;

extern void Udp_Init(void);
extern void Udp_WriteData(TUdpData data);
extern u32 Udp_ReadData(TUdpData *data);

#endif

