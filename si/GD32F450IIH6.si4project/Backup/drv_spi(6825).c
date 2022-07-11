/*
 * File      : drv_spi.c
 * Change Logs:
 * Date           Author       	Notes
 * 2022-05-25     zwb        	first implementation.
 */

#include "drv_spi.h"
#include "drv_gpio.h"
#include "pin_numbers_def.h"

#define ARR_LEN(__N)      (sizeof(__N) / sizeof(__N[0]))


static struct ins_spi_configuration spi_cfg =
{
    .mode = INS_SPI_MODE_0 | INS_SPI_MSB,
    .data_width = 8,
    .max_hz  = 5000000ul	/* ch378 max freq 30M(spi) */
};

static struct gd32_spi_cs  spi_cs =
{
	.spi_periph = SPI3,
    .GPIOx = GPIOG,
    .GPIO_Pin = GPIO_PIN_14
};

/* private rt-thread spi ops function */

static uint32_t spi_configure(uint32_t spi_periph, struct ins_spi_configuration* configuration)
{

    spi_parameter_struct spi_init_struct;

    /* data_width */
    if(configuration->data_width <= 8)
    {
        spi_init_struct.frame_size = SPI_FRAMESIZE_8BIT;
    }
    else if(configuration->data_width <= 16)
    {
        spi_init_struct.frame_size = SPI_FRAMESIZE_16BIT;
    }
    else
    {
        return INS_EIO;
    }

    /* baudrate */
    {
        rcu_clock_freq_enum spi_src;
        uint32_t spi_apb_clock;
        uint32_t max_hz;

        max_hz = configuration->max_hz;

        if (spi_periph == SPI1 || spi_periph == SPI2)
        {
            spi_src = CK_APB1;
        }
        else
        {
            spi_src = CK_APB2;
        }

        spi_apb_clock = rcu_clock_freq_get(spi_src);

        if(max_hz >= spi_apb_clock / 2)
        {
            spi_init_struct.prescale = SPI_PSC_2;
        }
        else if (max_hz >= spi_apb_clock / 4)
        {
            spi_init_struct.prescale = SPI_PSC_4;
        }
        else if (max_hz >= spi_apb_clock / 8)
        {
            spi_init_struct.prescale = SPI_PSC_8;
        }
        else if (max_hz >= spi_apb_clock / 16)
        {
            spi_init_struct.prescale = SPI_PSC_16;
        }
        else if (max_hz >= spi_apb_clock / 32)
        {
            spi_init_struct.prescale = SPI_PSC_32;
        }
        else if (max_hz >= spi_apb_clock / 64)
        {
            spi_init_struct.prescale = SPI_PSC_64;
        }
        else if (max_hz >= spi_apb_clock / 128)
        {
            spi_init_struct.prescale = SPI_PSC_128;
        }
        else
        {
            /*  min prescaler 256 */
            spi_init_struct.prescale = SPI_PSC_256;
        }
    } /* baudrate */

    switch(configuration->mode & INS_SPI_MODE_3)
    {
        case INS_SPI_MODE_0:
            spi_init_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE;
            break;

        case INS_SPI_MODE_1:
            spi_init_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_2EDGE;
            break;

        case INS_SPI_MODE_2:
            spi_init_struct.clock_polarity_phase = SPI_CK_PL_HIGH_PH_1EDGE;
            break;

        case INS_SPI_MODE_3:
            spi_init_struct.clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE;
            break;
    }

    /* MSB or LSB */
    if(configuration->mode & INS_SPI_MSB)
    {
        spi_init_struct.endian = SPI_ENDIAN_MSB;
    }
    else
    {
        spi_init_struct.endian = SPI_ENDIAN_LSB;
    }

    spi_init_struct.trans_mode = SPI_TRANSMODE_FULLDUPLEX;
    spi_init_struct.device_mode = SPI_MASTER;
    spi_init_struct.nss = SPI_NSS_SOFT;

    spi_crc_off(spi_periph);

    /* init SPI */
    spi_init(spi_periph, &spi_init_struct);
    /* Enable SPI_MASTER */
    spi_enable(spi_periph);

    return INS_EOK;
};
extern void delay_1us(uint32_t count);
uint32_t spi_xfer(struct gd32_spi_cs * gd32_spi_cs,
                  struct ins_spi_configuration* configuration, struct ins_spi_message* message)
{

    /* take CS */
    if(message->cs_take)
    {
    	gpio_bit_set(gd32_spi_cs->GPIOx, gd32_spi_cs->GPIO_Pin);
    	
        gpio_bit_reset(gd32_spi_cs->GPIOx, gd32_spi_cs->GPIO_Pin);
      	
    }

    if(configuration->data_width <= 8)
    {
        const uint8_t * send_ptr = message->send_buf;
        uint8_t * recv_ptr = message->recv_buf;
        uint32_t size = message->length;

        while(size--)
        {
            uint8_t data = 0xFF;

            if(send_ptr != NULL)
            {
                data = *send_ptr++;
            }

            // Todo: replace register read/write by gd32f4 lib
            //Wait until the transmit buffer is empty
            while(RESET == spi_i2s_flag_get(SPI3, SPI_FLAG_TBE));

            // Send the byte
            spi_i2s_data_transmit(gd32_spi_cs->spi_periph, data);
			
            //Wait until a data is received
            while(RESET == spi_i2s_flag_get(SPI3, SPI_FLAG_RBNE));

            // Get the received data
            data = spi_i2s_data_receive(gd32_spi_cs->spi_periph);

            if(recv_ptr != NULL)
            {
                *recv_ptr++ = data;
            }
        }

    }
    else if(configuration->data_width <= 16)
    {
        const uint16_t * send_ptr = message->send_buf;
        uint16_t * recv_ptr = message->recv_buf;
        uint32_t size = message->length;

        while(size--)
        {
            uint16_t data = 0xFF;

            if(send_ptr != NULL)
            {
                data = *send_ptr++;
            }

            //Wait until the transmit buffer is empty
            while(RESET == spi_i2s_flag_get(gd32_spi_cs->spi_periph, SPI_FLAG_TBE));

            // Send the byte
            spi_i2s_data_transmit(gd32_spi_cs->spi_periph, data);

            //Wait until a data is received
            while(RESET == spi_i2s_flag_get(gd32_spi_cs->spi_periph, SPI_FLAG_RBNE));

            // Get the received data
            data = spi_i2s_data_receive(gd32_spi_cs->spi_periph);

            if(recv_ptr != NULL)
            {
                *recv_ptr++ = data;
            }
        }
    }

    /* release CS */
    if(message->cs_release)
    {
    	delay_1us(2);
        gpio_bit_set(gd32_spi_cs->GPIOx, gd32_spi_cs->GPIO_Pin);
    }

    return message->length;
};

uint32_t spi_xfer_send(struct gd32_spi_cs * gd32_spi_cs,
                  struct ins_spi_configuration* configuration, struct ins_spi_message* message)
{

    /* take CS */
    if(message->cs_take)
    {
    	gpio_bit_set(gd32_spi_cs->GPIOx, gd32_spi_cs->GPIO_Pin);
    	
        gpio_bit_reset(gd32_spi_cs->GPIOx, gd32_spi_cs->GPIO_Pin);
      	
    }
	
    if(configuration->data_width <= 8)
    {
        const uint8_t * send_ptr = message->send_buf;
        uint32_t size = message->length;

        while(size--)
        {
            uint8_t data = 0xFF;

            if(send_ptr != NULL)
            {
                data = *send_ptr++;
            }

            // Todo: replace register read/write by gd32f4 lib
            //Wait until the transmit buffer is empty
            while(RESET == spi_i2s_flag_get(gd32_spi_cs->spi_periph, SPI_FLAG_TBE));

            // Send the byte
            spi_i2s_data_transmit(gd32_spi_cs->spi_periph, data);

        }

    }
    else if(configuration->data_width <= 16)
    {
        const uint16_t * send_ptr = message->send_buf;
        uint32_t size = message->length;

        while(size--)
        {
            uint16_t data = 0xFF;

            if(send_ptr != NULL)
            {
                data = *send_ptr++;
            }

            //Wait until the transmit buffer is empty
            while(RESET == spi_i2s_flag_get(gd32_spi_cs->spi_periph, SPI_FLAG_TBE));

            // Send the byte
            spi_i2s_data_transmit(gd32_spi_cs->spi_periph, data);

        }
    }

    /* release CS */
    if(message->cs_release)
    {
    	delay_1us(2);
        gpio_bit_set(gd32_spi_cs->GPIOx, gd32_spi_cs->GPIO_Pin);
    }

    return message->length;
};

uint32_t spi_xfer_recv(struct gd32_spi_cs * gd32_spi_cs,
                  struct ins_spi_configuration* configuration, struct 
ins_spi_message* message)
{

    /* take CS */
    if(message->cs_take)
    {
        gpio_bit_reset(gd32_spi_cs->GPIOx, gd32_spi_cs->GPIO_Pin);
      	
    }
	
    if(configuration->data_width <= 8)
    {
        uint8_t * recv_ptr = message->recv_buf;
        uint32_t size = message->length;

        while(size--)
        {
            uint8_t data = 0xFF;

            //Wait until a data is received
            while(RESET == spi_i2s_flag_get(gd32_spi_cs->spi_periph, SPI_FLAG_RBNE));

            // Get the received data
            data = spi_i2s_data_receive(gd32_spi_cs->spi_periph);

            if(recv_ptr != NULL)
            {
                *recv_ptr++ = data;
            }
        }

    }
    else if(configuration->data_width <= 16)
    {
        uint16_t * recv_ptr = message->recv_buf;
        uint32_t size = message->length;

        while(size--)
        {
            uint16_t data = 0xFF;

            //Wait until a data is received
            while(RESET == spi_i2s_flag_get(gd32_spi_cs->spi_periph, SPI_FLAG_RBNE));

            // Get the received data
            data = spi_i2s_data_receive(gd32_spi_cs->spi_periph);

            if(recv_ptr != NULL)
            {
                *recv_ptr++ = data;
            }
        }
    }

    /* release CS */
    if(message->cs_release)
    {
        gpio_bit_set(gd32_spi_cs->GPIOx, gd32_spi_cs->GPIO_Pin);
    }

    return message->length;
};

int spi_send(const void *send_buf, uint32_t length,uint8_t cs_take,uint8_t cs_release)
{
    struct ins_spi_message msg;

    msg.send_buf		= send_buf;
    msg.recv_buf		= NULL;
    msg.length 		= length;
    msg.cs_take		= cs_take;
    msg.cs_release 	= cs_release;
    msg.next			= NULL;

    return spi_xfer(&spi_cs, &spi_cfg, &msg);
}

int spi_recv(void *recv_buf, uint32_t length,uint8_t cs_take,uint8_t cs_release)
{
    struct ins_spi_message msg;

    msg.send_buf		= NULL;
    msg.recv_buf		= recv_buf;
    msg.length 		= length;
    msg.cs_take		= cs_take;
    msg.cs_release 	= cs_release;
    msg.next			= NULL;

    return spi_xfer(&spi_cs, &spi_cfg, &msg);
}

void hw_spi_init(uint32_t spi_periph)
{

    rcu_periph_clock_enable(RCU_GPIOG);
    rcu_periph_clock_enable(RCU_SPI3);

    /* SPI3_CLK(PG11), SPI3_MISO(PG12), SPI3_MOSI(PG13) GPIO pin configuration */
    gpio_af_set(GPIOG, GPIO_AF_6, GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13);
    gpio_mode_set(GPIOG, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13);
    gpio_output_options_set(GPIOG, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13);

    /* SPI5_CS(PG14) GPIO pin configuration */
    gpio_mode_set(GPIOG, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_14);
    gpio_output_options_set(GPIOG, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_14);
    //gd32_pin_mode(SPI3_NSS_PIN_NUM, PIN_MODE_OUTPUT);

    /* attach cs */
    gpio_bit_set(GPIOG, GPIO_PIN_14);
    //gd32_pin_write(SPI3_NSS_PIN_NUM, PIN_HIGH);

    spi_configure(spi_periph, &spi_cfg);
}

//void spi_selftest(void)
//{
//    struct ins_spi_message msg1, msg2;
//    uint8_t send_buf[8], recv_buf[8];

//    hw_spi_init();

//    msg1.send_buf	= send_buf;
//    msg1.recv_buf	= NULL;
//    msg1.length 	= 8;
//    msg1.cs_take	= 1;
//    msg1.cs_release = 0;
//    msg1.next		= &msg2;

//    msg2.send_buf	= NULL;
//    msg2.recv_buf	= recv_buf;
//    msg2.length 	= 8;
//    msg2.cs_take	= 0;
//    msg2.cs_release = 1;
//    msg2.next		= NULL;

//    spi_xfer(&spi_cs, &spi_cfg, &msg1);
//    spi_xfer(&spi_cs, &spi_cfg, &msg2);
//}

