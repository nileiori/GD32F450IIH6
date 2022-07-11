
#ifndef __CAN_ADAPTER_H
#define __CAN_ADAPTER_H

#include "stm32f4xx.h"
#include "stdbool.h"

typedef enum ECANCHANNEL
{
    CAN_1 = 0, // channel 1
	CAN_2,     // channel 2
}ECanChannel;

typedef enum ECANFRAMETYPE
{
    FrameType_Data = 0,  // data frame
    FrameType_Remote = 1, // remote frame
    
}ECanFrameType;

typedef enum ECANIDTYPE
{
    IDType_Standard = 0, // standard frame
    IDType_Extend = 1,   // extend frame
    
}ECanIdType;

typedef struct TFILTER
{
    ECanFrameType frameType;
    ECanIdType idType;
    u8 filterNo;
    u32 filterID;
}TFilter;

typedef struct TCANCONFIG
{
    ECanChannel ch;         
    u8 filterUsedNum;     
    TFilter filter[14];
    bool bUseInterrupt;
    
}TCanConfig;

typedef struct TCANDATA
{
    ECanFrameType frameType; 
    ECanIdType idType;
    u32 id;
    u8 len;
    u8 data[8];
    
}TCanData;

typedef void (*Can_IntHandler)(TCanData);

extern bool Can_Init(void);

extern void Can_FormatFilter(TFilter *filter, u8 num, ECanFrameType frameType, ECanIdType idType, u32 id);
extern bool Can_BindHander(ECanChannel ch, Can_IntHandler hander);
extern bool Can_SetConfig(ECanChannel ch, TCanConfig config);
extern bool Can_Config(Can_IntHandler hander, TCanConfig config);

extern bool Can_SendData(ECanChannel ch, TCanData data);
extern bool Can_RecvData(ECanChannel ch, TCanData *data);


#endif

