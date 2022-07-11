
#ifndef __SERIAL_H__
#define __SERIAL_H__

#include "insdef.h"
#include "drv_usart.h"


#define BAUD_RATE_2400                  2400
#define BAUD_RATE_4800                  4800
#define BAUD_RATE_9600                  9600
#define BAUD_RATE_19200                 19200
#define BAUD_RATE_38400                 38400
#define BAUD_RATE_57600                 57600
#define BAUD_RATE_115200                115200
#define BAUD_RATE_230400                230400
#define BAUD_RATE_460800                460800
#define BAUD_RATE_921600                921600
#define BAUD_RATE_2000000               2000000
#define BAUD_RATE_3000000               3000000


#ifndef USART_SERIAL_RB_BUFSZ
#define USART_SERIAL_RB_BUFSZ           100
#endif

#define INS_SERIAL_EVENT_RX_IND          0x01    /* Rx indication */
#define INS_SERIAL_EVENT_TX_DONE         0x02    /* Tx complete   */
#define INS_SERIAL_EVENT_RX_DMADONE      0x03    /* Rx DMA transfer done */
#define INS_SERIAL_EVENT_TX_DMADONE      0x04    /* Tx DMA transfer done */
#define INS_SERIAL_EVENT_RX_TIMEOUT      0x05    /* Rx timeout    */

#define INS_SERIAL_DMA_RX                0x01
#define INS_SERIAL_DMA_TX                0x02

#define INS_SERIAL_RX_INT                0x01
#define INS_SERIAL_TX_INT                0x02

#define INS_SERIAL_ERR_OVERRUN           0x01
#define INS_SERIAL_ERR_FRAMING           0x02
#define INS_SERIAL_ERR_PARITY            0x03


/* Default config for serial_configure structure */
#define SERIAL_CONFIG_DEFAULT                    \
{                                                   \
    BAUD_RATE_115200,       /* 115200 bits/s */     \
    USART_WL_8BIT,          /* 8 databits */        \
    USART_STB_1BIT,         /* 1 stopbit */         \
    USART_PM_NONE,          /* No parity  */        \
    USART_MSBF_LSB,         /* LSB first sent */    \
    USART_DINV_DISABLE,     /* Normal mode */       \
    USART_SERIAL_RB_BUFSZ,  /* Buffer size */       \
    0                                               \
}


struct serial_configure
{
    uint32_t baud_rate;

    uint32_t data_bits               :4;
    uint32_t stop_bits               :2;
    uint32_t parity                  :2;
    uint32_t bit_order               :1;
    uint32_t invert                  :1;
    uint32_t bufsz                   :16;
    uint32_t reserved                :4;
};


/*
 * Serial FIFO mode 
 */
typedef struct 
{
    /* software fifo */
    uint8_t *buffer;

    uint16_t put_index, get_index;

    uint8_t  is_full;
}serial_rx_fifo,serial_tx_fifo;

struct completion
{
    uint32_t flag;

    /* suspended list */
    list_t suspended_list;
};

//struct serial_tx_fifo
//{
//    struct completion completion;
//};
#if 1
/* GD32 uart driver */
struct gd32_usart
{
    uint32_t usart_device;
    IRQn_Type irqn;
    uint8_t irq_pre_priority;
    uint8_t irq_sub_priority;
#if defined( configUse_usart_dma )
    struct gd32_usart_dma
    {
        /* dma stream */
        //dma_parameter_struct* dma_init_struct;
        /* dma channel */
        dma_channel_enum rx_ch;
        /* dma flag */
        uint32_t rx_flag;
        /* dma irq channel */
        uint8_t rx_irq_ch;
        /* setting receive len */
        ins_size_t setting_recv_len;
        /* last receive index */
        ins_size_t last_recv_index;
    } dma;
#endif
};
#else
struct gd32_usart
{
    uint32_t usart_device;           //Todo: 3bits
    IRQn_Type irqn;                 //Todo: 7bits    
    ug_uint8_t irq_pre_priority;
    ug_uint8_t irq_sub_priority;
    
    rcu_periph_enum per_clk;        //Todo: 5bits
    rcu_periph_enum tx_gpio_clk;    //Todo: 5bits
    rcu_periph_enum rx_gpio_clk;    //Todo: 5bits
    uint32_t tx_port;               //Todo: 4bits
    uint16_t tx_af;                 //Todo: 4bits
    uint16_t tx_pin;                //Todo: 4bits
    uint32_t rx_port;               //Todo: 4bits
    uint16_t rx_af;                 //Todo: 4bits
    uint16_t rx_pin;                //Todo: 4bits

    struct serial_device * serial;    
    char *device_name;
};
#endif

struct serial_device
{
    struct gd32_usart *usart;
    const struct usart_ops *ops;
    struct serial_configure   config;
    
    void *serial_rx;
    void *serial_tx;
};
typedef struct serial_device serial_t;

/**
 * uart operators
 */
struct usart_ops
{
    uint32_t (*configure)(struct serial_device *serial);
    uint32_t (*control)(struct serial_device *serial, int cmd, void *arg);

    int (*putc)(struct serial_device *serial, char c);
    int (*getc)(struct serial_device *serial);

    ins_size_t (*dma_transmit)(struct serial_device *serial, uint8_t *buf, ins_size_t size, int direction);
};

void hw_serial_isr(struct serial_device *serial, int event);

uint32_t hw_serial_register(struct serial_device *serial);

uint32_t _serial_int_rx(struct serial_device *serial, uint8_t *data, int length);

uint32_t _serial_int_tx(struct serial_device *serial, const uint8_t *data, int length);

uint32_t _serial_read_tx_fifo(struct serial_device *serial, uint8_t *data, int length);

void _serial_write_tx_fifo(struct serial_device *serial, uint8_t *pdata, int length);



#endif


