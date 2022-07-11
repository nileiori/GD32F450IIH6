#ifndef __DRV_USART_H__
#define __DRV_USART_H__

#include "config.h"
#include "insdef.h"

#ifdef cplusplus
extern "C" {
#endif


#define UART_ENABLE_IRQ(n)            NVIC_EnableIRQ((n))
#define UART_DISABLE_IRQ(n)           NVIC_DisableIRQ((n))

#define INS_USING_UART4
#define INS_USING_UART4_DMA0


void        gd32_hw_usart_init(void);

uint32_t   gd32_usart_read(void *buffer, uint32_t size);

uint32_t   gd32_usart_write(uint8_t *buffer, uint32_t size);

void        usart_tx_fifo_send(uint8_t *buffer, uint32_t size);

void 		usart_transmit_dma(uint8_t *pData, uint16_t Size);


#ifdef cplusplus
}
#endif

#endif




