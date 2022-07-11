#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "Gd32f4xx_usart.h"
#include "serial.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"
#include "message_buffer.h"



/*
 * Serial interrupt routines
 */
uint32_t _serial_int_rx(struct serial_device *serial, uint8_t *data, int length)
{
    int size;
    serial_rx_fifo* rx_fifo;

    size = length;

    rx_fifo = (serial_rx_fifo*) serial->serial_rx;
    configASSERT(rx_fifo != NULL);

    /* read from software FIFO */
    while (length)
    {
        int ch;

        /* disable interrupt */
//        hw_interrupt_disable();

        /* there's no data: */
        if ((rx_fifo->get_index == rx_fifo->put_index) && (rx_fifo->is_full == INS_FALSE))
        {
            /* no data, enable interrupt and break out */
//            hw_interrupt_enable();
            break;
        }

        /* otherwise there's the data: */
        ch = rx_fifo->buffer[rx_fifo->get_index];
        rx_fifo->get_index += 1;
        if (rx_fifo->get_index >= serial->config.bufsz) rx_fifo->get_index = 0;

        if (rx_fifo->is_full == INS_TRUE)
        {
            rx_fifo->is_full = INS_FALSE;
        }

        /* enable interrupt */
//        hw_interrupt_enable();

        *data = ch & 0xff;
        data ++;
        length --;
    }

    return size - length;
}

uint32_t _serial_int_tx(struct serial_device *serial, const uint8_t *data, int length)
{
    int size;

    configASSERT(serial != NULL);

    size = length;

    while (length)
    {
        serial->ops->putc(serial, *(char*)data);
        data ++;
        length --;
    }

    return size - length;
}

uint32_t _serial_read_tx_fifo(struct serial_device *serial, uint8_t *data, int length)
{
    int size;
    serial_tx_fifo* tx_fifo;

    size = length;

    tx_fifo = (serial_tx_fifo*) serial->serial_tx;
    configASSERT(tx_fifo != NULL);

    /* read from software FIFO */
    while (length)
    {
        int ch;

        /* disable interrupt */
//        hw_interrupt_disable();

        /* there's no data: */
        if ((tx_fifo->get_index == tx_fifo->put_index) && (tx_fifo->is_full == INS_FALSE))
        {
            /* no data, enable interrupt and break out */
//            hw_interrupt_enable();
            break;
        }

        /* otherwise there's the data: */
        ch = tx_fifo->buffer[tx_fifo->get_index];
        tx_fifo->get_index += 1;
        if (tx_fifo->get_index >= serial->config.bufsz) tx_fifo->get_index = 0;

        if (tx_fifo->is_full == INS_TRUE)
        {
            tx_fifo->is_full = INS_FALSE;
        }

        /* enable interrupt */
//        hw_interrupt_enable();

        *data = ch & 0xff;
        data ++;
        length --;
    }

    return size - length;
}

void _serial_write_tx_fifo(struct serial_device *serial, uint8_t *pdata, int length)
{
    serial_tx_fifo* tx_fifo;

    tx_fifo = (serial_rx_fifo*)serial->serial_tx;
    configASSERT(tx_fifo != NULL);

    /* disable interrupt */
//    hw_interrupt_disable();

    while (length--)
    {
        tx_fifo->buffer[tx_fifo->put_index] = *pdata++;
        tx_fifo->put_index += 1;
        if (tx_fifo->put_index >= serial->config.bufsz) tx_fifo->put_index = 0;

        if (tx_fifo->put_index == tx_fifo->get_index)
        {
            tx_fifo->get_index += 1;
            tx_fifo->is_full = INS_TRUE;
            if (tx_fifo->get_index >= serial->config.bufsz) tx_fifo->get_index = 0;
        }
    }
    /* enable interrupt */
//    hw_interrupt_enable();

}

static uint32_t ug_serial_open(struct serial_device *serial, uint16_t oflag)
{
    uint32_t result = INS_EOK;

    /* initialize the Rx/Tx structure according to open flag */
    if (serial->serial_rx == NULL)
    {
        if (oflag & DEVICE_FLAG_INT_RX)
        {
            serial_rx_fifo* rx_fifo;

            rx_fifo = (serial_rx_fifo*) malloc (sizeof(serial_rx_fifo) +
                                                   serial->config.bufsz);
            configASSERT(rx_fifo != NULL);
            rx_fifo->buffer = (uint8_t*) (rx_fifo + 1);
            memset(rx_fifo->buffer, 0, serial->config.bufsz);
            rx_fifo->put_index = 0;
            rx_fifo->get_index = 0;
            rx_fifo->is_full = INS_FALSE;

            serial->serial_rx = rx_fifo;
            /* configure low level device */
            serial->ops->control(serial, DEVICE_CTRL_SET_INT, (void *)DEVICE_FLAG_INT_RX);
        }
        else
        {
            serial->serial_rx = NULL;
        }
    }

    if (serial->serial_tx == NULL)
    {
        if (oflag & DEVICE_FLAG_INT_TX)
        {
            serial_tx_fifo* tx_fifo;

            tx_fifo = (serial_tx_fifo*) malloc (sizeof(serial_tx_fifo) +
                                                   serial->config.bufsz);
            configASSERT(tx_fifo != NULL);
            tx_fifo->buffer = (uint8_t*) (tx_fifo + 1);
            memset(tx_fifo->buffer, 0, serial->config.bufsz);
            tx_fifo->put_index = 0;
            tx_fifo->get_index = 0;
            tx_fifo->is_full = INS_FALSE;

            serial->serial_tx = tx_fifo;
        }
        else
        {
            serial->serial_tx = NULL;
        }
    }

    /* apply configuration */
    if (serial->ops->configure)
        result = serial->ops->configure(serial);

    return result;
}

/*
 * serial register
 */
uint32_t hw_serial_register(struct serial_device *serial)
{
    configASSERT(serial != NULL);
    struct gd32_usart *usart = serial->usart;

    usart_deinit(usart->usart_device);
	
    ug_serial_open(serial,DEVICE_FLAG_INT_RX/* | DEVICE_FLAG_INT_TX*/);

    return INS_EOK;
}

/* ISR for serial interrupt */
void hw_serial_isr(struct serial_device *serial, int event)
{
    switch (event & 0xff)
    {
    case INS_SERIAL_EVENT_RX_IND:
    {
        int ch = -1;
        serial_rx_fifo* rx_fifo;

        /* interrupt mode receive */
        rx_fifo = (serial_rx_fifo*)serial->serial_rx;
        configASSERT(rx_fifo != NULL);

        while (1)
        {
            ch = serial->ops->getc(serial);
            if (ch == -1)
            {
                break;
            }

            /* disable interrupt */
//                interrupt_disable();

            rx_fifo->buffer[rx_fifo->put_index] = ch;
            rx_fifo->put_index += 1;
            if (rx_fifo->put_index >= serial->config.bufsz) rx_fifo->put_index = 0;

            /* if the next position is read index, discard this 'read char' */
            if (rx_fifo->put_index == rx_fifo->get_index)
            {
                rx_fifo->get_index += 1;
                rx_fifo->is_full = INS_TRUE;
                if (rx_fifo->get_index >= serial->config.bufsz) rx_fifo->get_index = 0;
            }

            /* enable interrupt */
//                hw_interrupt_enable();
        }

        break;
    }
    case INS_SERIAL_EVENT_TX_DONE:
    {

        break;
    }
    }
}





