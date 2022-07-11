#ifndef __M_QUEUE_H__
#define __M_QUEUE_H__

#include "gd32f4xx.h"
#include "insdef.h"

#ifdef cplusplus
 extern "C" {
#endif 


struct QueueRecord
{
    int Capacity;
    int Front;
    int Rear;
    int Size;
    uint8_t *Array;
};
typedef struct QueueRecord *    pQueue;

pQueue QueueCreate(int MaxElements,uint8_t* pData);
void QueueIn(uint8_t X, pQueue Q);
void usart_queue_init(void);
uint8_t QueueFrontAndOut2(pQueue Q, uint8_t *pFlag);


#ifdef cplusplus
}
#endif 

#endif









