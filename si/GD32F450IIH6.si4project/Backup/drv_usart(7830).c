#include "gd32f4xx.h"
#include "stdio.h"
#include "string.h"
#include "serial.h"
//#include "m_queue.h"


//extern pQueue	m_queue;
uint8_t uart4_rx_finish_irq(void);

static uint32_t gd32_usart_configure(struct serial_device *serial)
{
    struct gd32_usart *usart       = serial->usart;
    struct serial_configure cfg  = serial->config;

    // USART configure
    //usart_deinit(usart->usart_device);
    usart_baudrate_set(usart->usart_device, cfg.baud_rate);
    usart_word_length_set(usart->usart_device, cfg.data_bits);
    usart_parity_config(usart->usart_device, cfg.parity);
    usart_stop_bit_set(usart->usart_device, cfg.stop_bits);
    //usart_data_first_config(usart->usart_device, cfg.bit_order);
    //usart_invert_config(usart->usart_device, (usart_invert_enum)cfg.invert);
    //usart_overrun_enable(usart->usart_device);
    usart_receive_config(usart->usart_device, USART_RECEIVE_ENABLE);
    usart_transmit_config(usart->usart_device, USART_TRANSMIT_ENABLE);
    #if defined(INS_USING_UART4_DMA0)
    /* USART DMA enable*/
    usart_dma_receive_config(UART4, USART_DENR_ENABLE);
    usart_dma_transmit_config(UART4, USART_DENT_ENABLE);

    usart_flag_clear(UART4, USART_FLAG_TC);
    usart_interrupt_enable(UART4, USART_INT_IDLE);
    #endif
    usart_flag_clear(usart->usart_device, USART_FLAG_RBNE);
    usart_flag_clear(usart->usart_device, USART_FLAG_TC);
    usart_enable(usart->usart_device);

    return INS_EOK;
}

uint32_t gd32_usart_set_baud(uint32_t usart_device,uint32_t baud_rate,uint8_t parity,uint8_t data_bits,uint8_t stop_bits)
{
    // USART configure
    usart_disable(usart_device);
    usart_baudrate_set(usart_device, baud_rate);
    usart_word_length_set(usart_device, data_bits);
    usart_parity_config(usart_device, parity);
    usart_stop_bit_set(usart_device, stop_bits);
	usart_enable(usart_device);

    return INS_EOK;
}

static uint32_t gd32_usart_control(struct serial_device *serial, int cmd, void *arg)
{
    struct gd32_usart* usart = serial->usart;
    uint32_t ctrl_arg = (uint32_t)(arg);

    switch (cmd)
    {
        case DEVICE_CTRL_CLR_INT:
            /* disable rx irq */
            UART_DISABLE_IRQ(usart->irqn);
            /* disable interrupt */
            usart_interrupt_disable(usart->usart_device, USART_INT_RBNE);
            break;

        case DEVICE_CTRL_SET_INT:
            /* enable rx irq */
            //UART_ENABLE_IRQ(usart->irqn);
            nvic_irq_enable(usart->irqn, usart->irq_pre_priority, usart->irq_sub_priority);
            /* enable interrupt */
            usart_interrupt_enable(usart->usart_device, USART_INT_RBNE);
            break;

        case DEVICE_CTRL_CONFIG :
            if (ctrl_arg == DEVICE_FLAG_DMA_RX)
            {
            }

            break;

    }

    return INS_EOK;
}

static int gd32_usart_putc(struct serial_device *serial, char ch)
{
    struct gd32_usart *usart = serial->usart;

    usart_data_transmit(usart->usart_device, ch);

    while((usart_flag_get(usart->usart_device, USART_FLAG_TC) == RESET));

    return 1;
}

static int gd32_usart_getc(struct serial_device *serial)
{
    int data;

    struct gd32_usart *usart = serial->usart;

    data = -1;

    if (usart_flag_get(usart->usart_device, USART_FLAG_RBNE) != RESET)
        data = usart_data_receive(usart->usart_device);

    return data;
}

#if defined(INS_USING_UART4)
static const struct usart_ops gd32_usart_ops =
{
    gd32_usart_configure,
    gd32_usart_control,
    gd32_usart_putc,
    gd32_usart_getc
};

/* UART4 device driver structure */
struct gd32_usart usart4 =
{
    UART4,
    UART4_IRQn,
    0,
    0
};
struct serial_device serial4 =
{
    &usart4,
    &gd32_usart_ops,
    SERIAL_CONFIG_DEFAULT,
    NULL,
    NULL
};

#if defined(INS_USING_UART4_DMA0)
#define UART4_DATA_ADDRESS      ((uint32_t)&USART_DATA(UART4))
#define MAX_RECV_LEN_BUFFER 255
#define MAX_SEND_LEN_BUFFER 50
#define MAX_SEND_CNT_QUEUE  10

typedef struct
{
    uint8_t storeIndex;
    uint8_t sendIndex;
    uint8_t resCount;
    uint8_t count[MAX_SEND_CNT_QUEUE];
    uint8_t buffer[MAX_SEND_CNT_QUEUE][MAX_SEND_LEN_BUFFER];
} TSendQueue;

typedef struct
{
    //EUartType uartType;
    uint8_t sendBuffer[MAX_SEND_LEN_BUFFER];
    uint8_t recvBuffer[MAX_RECV_LEN_BUFFER];
    uint8_t cacheBuffer[MAX_RECV_LEN_BUFFER];
    uint8_t recvLength;
    uint8_t recvStatus;
    TSendQueue sendQueue;
} TUartData;

TUartData g_UartData;

uint8_t uart4_rx_finish_irq(void)
{
    uint16_t len = 0;

    USART_STAT0(UART4);
    USART_DATA(UART4);
    dma_channel_disable(DMA0, DMA_CH0);
    len = MAX_RECV_LEN_BUFFER - dma_transfer_number_get(DMA0, DMA_CH0);
    memcpy(g_UartData.cacheBuffer, g_UartData.recvBuffer, len);
    dma_transfer_number_config(DMA0, DMA_CH0, MAX_RECV_LEN_BUFFER);
    dma_channel_enable(DMA0, DMA_CH0);
    return len;

}

void dma_nvic_config(void)
{
    //nvic_irq_enable(DMA0_Channel0_IRQn, 0, 0);
    nvic_irq_enable(DMA0_Channel7_IRQn, 0, 0);
}

void dma_config(void)
{
    dma_single_data_parameter_struct dma_init_struct;

    /* deinitialize DMA0 channel7(USART4 TX) */
    dma_single_data_para_struct_init(&dma_init_struct);
    dma_deinit(DMA0, DMA_CH7);
    dma_init_struct.direction = DMA_MEMORY_TO_PERIPH;
    dma_init_struct.memory0_addr = (uint32_t)g_UartData.sendBuffer;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;
    dma_init_struct.number = 0;
    dma_init_struct.periph_addr = UART4_DATA_ADDRESS;
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
    dma_single_data_mode_init(DMA0, DMA_CH7, &dma_init_struct);

    /* configure DMA mode */
    dma_circulation_disable(DMA0, DMA_CH7);
    dma_channel_subperipheral_select(DMA0, DMA_CH7, DMA_SUBPERI4);
    /* enable DMA0 channel7 transfer complete interrupt */
    dma_interrupt_enable(DMA0, DMA_CH7, DMA_CHXCTL_FTFIE);
    /* enable DMA0 channel7 */
    //dma_channel_enable(DMA0, DMA_CH7);

    /* deinitialize DMA0 channel0 (UART4 RX) */
    dma_deinit(DMA0, DMA_CH0);
    dma_init_struct.direction = DMA_PERIPH_TO_MEMORY;
    dma_init_struct.memory0_addr = (uint32_t)g_UartData.recvBuffer;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.number = MAX_RECV_LEN_BUFFER;
    dma_init_struct.periph_addr = UART4_DATA_ADDRESS;
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
    dma_single_data_mode_init(DMA0, DMA_CH0, &dma_init_struct);

    /* configure DMA mode */
    dma_circulation_disable(DMA0, DMA_CH0);
    dma_channel_subperipheral_select(DMA0, DMA_CH0, DMA_SUBPERI4);
    /* enable DMA0 channel0 transfer complete interrupt */
    dma_interrupt_enable(DMA0, DMA_CH0, DMA_CHXCTL_FTFIE);
    /* enable DMA1 channel0 */
    dma_channel_enable(DMA0, DMA_CH0);

    memset(&g_UartData, 0, sizeof(TUartData));
}

void DMA0_Channel7_IRQHandler(void)
{
    TSendQueue* sendQueue = &g_UartData.sendQueue;

    if (dma_interrupt_flag_get(DMA0, DMA_CH7, DMA_INT_FLAG_FTF) != RESET)
    {
        dma_interrupt_flag_clear(DMA0, DMA_CH7, DMA_INT_FLAG_FTF);
        dma_channel_disable(DMA0, DMA_CH7);
        usart_interrupt_enable(UART4, USART_INT_TC);

        if (sendQueue->resCount > 0) sendQueue->resCount--;

        if (sendQueue->sendIndex >= MAX_SEND_CNT_QUEUE) sendQueue->sendIndex = 0;

        if (sendQueue->resCount > 0)
        {
            memcpy(g_UartData.sendBuffer, sendQueue->buffer[sendQueue->sendIndex], sendQueue->count[sendQueue->sendIndex]);
            dma_transfer_number_config(DMA0, DMA_CH7, sendQueue->count[sendQueue->sendIndex]);
            sendQueue->sendIndex++;
            dma_channel_enable(DMA0, DMA_CH7);
        }
    }
}

uint16_t uart4_dma_recv_data(uint8_t* buf, uint16_t len)
{
    uint16_t  recvLen = 0;

    if (g_UartData.recvStatus)
    {
        g_UartData.recvStatus = 0x00;
        recvLen = g_UartData.recvLength;
        memcpy(buf, g_UartData.recvBuffer, recvLen);
    }

    return recvLen;
}

uint32_t uart4_dma_send_data(uint8_t* data, uint16_t size)
{
    uint8_t i = 0;
    uint8_t intCnt = size / MAX_SEND_LEN_BUFFER;
    uint8_t resCnt = size % MAX_SEND_LEN_BUFFER;

    TSendQueue* sendQueue = &g_UartData.sendQueue;

    if (size == 0) return 0;

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
        memcpy(g_UartData.sendBuffer, sendQueue->buffer[sendQueue->sendIndex], sendQueue->count[sendQueue->sendIndex]);
        dma_transfer_number_config(DMA0, DMA_CH7, sendQueue->count[sendQueue->sendIndex]);
        sendQueue->sendIndex++;
        dma_channel_enable(DMA0, DMA_CH7);
    }

    sendQueue->resCount += intCnt + 1;

    return size;
}

void gd32_usart_dma_init(void)
{
    /* enable DMA0 clock */
    rcu_periph_clock_enable(RCU_DMA0);

    /*configure DMA0 interrupt*/
    dma_nvic_config();
    dma_config();

}

#endif

void usart_isr(struct serial_device *serial)
{

    struct gd32_usart *usart = serial->usart;
    serial_rx_fifo* rx_fifo;

    if(RESET != usart_interrupt_flag_get(usart->usart_device, USART_INT_FLAG_RBNE))
    {
        /* clear interrupt */
        usart_interrupt_flag_clear(usart->usart_device, USART_INT_FLAG_RBNE);
        usart_flag_clear(usart->usart_device, USART_FLAG_RBNE);
        hw_serial_isr(serial, INS_SERIAL_EVENT_RX_IND);
    }

    #if 0

    if (RESET != usart_interrupt_flag_get(usart->usart_device, USART_INT_FLAG_RBNE_ORERR))
    {
        usart_interrupt_flag_clear(usart->usart_device, USART_INT_FLAG_RBNE_ORERR);
        usart_data_receive(usart->usart_device);
        rx_fifo = (ug_serial_rx_fifo*) serial->serial_rx;
        rx_fifo->is_full = UG_FALSE;
        rx_fifo->put_index = rx_fifo->get_index = 0;
    }

    if (RESET != usart_interrupt_flag_get(usart->usart_device, USART_INT_FLAG_ERR_ORERR))
    {
        usart_interrupt_flag_clear(usart->usart_device, USART_INT_FLAG_ERR_ORERR);

        usart_data_receive(usart->usart_device);

        rx_fifo = (ug_serial_rx_fifo*) serial->serial_rx;
        rx_fifo->is_full = UG_FALSE;
        rx_fifo->put_index = rx_fifo->get_index = 0;
    }

    #else

    if (RESET != usart_flag_get(usart->usart_device, USART_FLAG_ORERR))
    {
        usart_data_receive(usart->usart_device);
        usart_flag_clear(usart->usart_device, USART_FLAG_ORERR);

        rx_fifo = (serial_rx_fifo*) serial->serial_rx;
        rx_fifo->is_full = INS_FALSE;
        rx_fifo->put_index = rx_fifo->get_index = 0;
    }

    if (RESET != usart_flag_get(usart->usart_device, USART_FLAG_NERR))
    {
        usart_data_receive(usart->usart_device);
        usart_flag_clear(usart->usart_device, USART_FLAG_NERR);

        rx_fifo = (serial_rx_fifo*) serial->serial_rx;
        rx_fifo->is_full = INS_FALSE;
        rx_fifo->put_index = rx_fifo->get_index = 0;
    }

    #endif
}

void UART4_IRQHandler(void)
{
    #if defined(INS_USING_UART4_DMA0)

    if(usart_interrupt_flag_get(UART4, USART_INT_FLAG_TC) != RESET)
    {
        usart_interrupt_flag_clear(UART4, USART_INT_FLAG_TC);
        usart_interrupt_disable(UART4, USART_INT_TC);
    }

    if(usart_interrupt_flag_get(UART4, USART_INT_FLAG_IDLE) != RESET)
    {
        usart_interrupt_flag_clear(UART4, USART_INT_FLAG_IDLE);
        g_UartData.recvLength = uart4_rx_finish_irq();

        if (g_UartData.recvLength > 0)
        {
            g_UartData.recvStatus = 0x01;
        }
    }

    #else
    usart_isr(&serial4);
//	uint8_t data;
//		if(RESET != usart_interrupt_flag_get(UART4, USART_INT_FLAG_RBNE))
//		{
//			usart_interrupt_flag_clear(UART4, USART_INT_FLAG_RBNE);
//			/* receive data */
//			data = (uint8_t)usart_data_receive(UART4);
//			QueueIn(data,m_queue);
//		}
//		else if(RESET != usart_interrupt_flag_get(UART4, USART_INT_FLAG_RBNE_ORERR))
//		{
//			usart_data_receive(UART4);
//			usart_interrupt_flag_clear(UART4, USART_INT_FLAG_RBNE_ORERR);
//		}
    #endif
}

#endif

static void rcc_configuration(void)
{
    #ifdef INS_USING_UART4
    /* enable COM GPIO clock */
    rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_GPIOD);
    // enable USART clock
    rcu_periph_clock_enable(RCU_UART4);

    #endif

}

static void gpio_configuration(void)
{
    #ifdef INS_USING_UART4
    gpio_af_set(GPIOC, GPIO_AF_8, GPIO_PIN_12);
    gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_12);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_12);
    
	gpio_af_set(GPIOD, GPIO_AF_8, GPIO_PIN_2);
    gpio_mode_set(GPIOD, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_2);
    gpio_output_options_set(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_2);
    
    #endif
}

uint32_t gd32_usart_read(void *buffer, uint32_t size)
{
#if defined(INS_USING_UART4_DMA0)
	return uart4_dma_recv_data(buffer, size);
#else
    return _serial_int_rx(&serial4, buffer, size);
#endif
}

uint32_t gd32_usart_write(uint8_t *buffer, uint32_t size)
{
#if defined(INS_USING_UART4_DMA0)
	return uart4_dma_send_data(buffer, size);
#else
    return _serial_int_tx(&serial4, buffer, size);
#endif
}

void usart_tx_fifo_send(uint8_t *buffer, uint32_t size)
{
    struct gd32_usart *usart = serial4.usart;

    _serial_write_tx_fifo(&serial4, buffer, size);
    usart_interrupt_enable(usart->usart_device, USART_INT_TC);
}

void gd32_hw_usart_init(void)
{
    rcc_configuration();
    gpio_configuration();
    #if defined(INS_USING_UART4_DMA0)
    gd32_usart_dma_init();
    #endif
    hw_serial_register(&serial4);

//    usart_deinit(UART4);
// 
//    usart_baudrate_set(UART4, 115200ul);
//    usart_word_length_set(UART4, USART_WL_8BIT);
//    usart_parity_config(UART4, USART_PM_NONE);
//    usart_stop_bit_set(UART4, USART_STB_1BIT);
//    usart_receive_config(UART4, USART_RECEIVE_ENABLE);
//    usart_transmit_config(UART4, USART_TRANSMIT_ENABLE);
//    
//    nvic_irq_enable(UART4_IRQn, 2, 0);
//    /* enable interrupt */
//    usart_interrupt_enable(UART4, USART_INT_RBNE);
//    usart_enable(UART4);
}




