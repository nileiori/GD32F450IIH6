#include "stdlib.h"
#include "m_queue.h"



int QueueIsEmpty( pQueue Q )
{
    return Q->Size == 0;
}

int QueueIsFull( pQueue Q )
{
    return Q->Size == Q->Capacity;
}

void QueueMakeEmpty(pQueue Q)
{
    Q->Size = 0;
    Q->Front = 1;
    Q->Rear = 0;
}

pQueue QueueCreate(int MaxElements,uint8_t* pData)
{
    pQueue Q;

    Q = (pQueue)malloc( sizeof(struct QueueRecord));
    if( Q == NULL )
        return NULL;

    Q->Array = pData;

    if( Q->Array == NULL )
        return NULL;

    Q->Capacity = MaxElements;

    QueueMakeEmpty( Q );

    return Q;
}

int QueueSucc(int Value, pQueue Q)
{
    if( ++Value == Q->Capacity )
        Value = 0;
    return Value;
}

void QueueIn(uint8_t X, pQueue Q)
{
    if(QueueIsFull(Q))
    {
        return ;
    }
    else
    {
        Q->Size++;
        Q->Rear = QueueSucc( Q->Rear, Q );
        Q->Array[ Q->Rear ] = X;
    }
}

void QueueInBuffer(uint8_t *pBuffer, int BufferLen, pQueue Q)
{
    int i = 0;
    uint8_t X;

    if((Q->Capacity - Q->Size) > (BufferLen))
    {
        for(i=0; i<BufferLen; i++)
        {
            X = *(pBuffer+i);
            Q->Size++;
            Q->Rear = QueueSucc( Q->Rear, Q );
            Q->Array[ Q->Rear ] = X;
        }
    }
}

uint8_t QueueFrontAndOut2(pQueue Q, uint8_t *pFlag)
{
    uint8_t X = 0;

    if(QueueIsEmpty(Q))
    {
        *pFlag = 0;
        return X;
    }

    Q->Size--;
    X = Q->Array[ Q->Front ];
    Q->Front = QueueSucc( Q->Front, Q );

    *pFlag = 1;
    return X;
}

int  QueueOutBuffer(uint8_t *pBuffer, pQueue Q)
{
    int count = 0;
    uint8_t *p = NULL;

    if(NULL != p)
    {
        return 0;
    }

    p = pBuffer;

    while( 1 )
    {
        if(QueueIsEmpty(Q))
        {
            break;
        }
        else
        {
            Q->Size--;
            *p++ = Q->Array[ Q->Front ];
            Q->Front = QueueSucc( Q->Front, Q );
            count++;
        }

    }
    return count;
}


