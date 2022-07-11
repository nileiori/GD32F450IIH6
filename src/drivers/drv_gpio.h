#ifndef GPIO_H__
#define GPIO_H__


#ifdef __cplusplus
extern "C" {
#endif

#include "gd32f4xx.h"
#include "gd32f4xx_exti.h"
#include "insdef.h"

typedef struct
{
    uint8_t nvic_irq_pre_priority;
    uint8_t nvic_irq_sub_priority;
} irq_priority;

#define PIN_LOW                 0x00
#define PIN_HIGH                0x01

#define PIN_MODE_OUTPUT         	0x00
#define PIN_MODE_OUTPUT_PULLUP      0x01
#define PIN_MODE_OUTPUT_PULLDOWN    0x02
#define PIN_MODE_INPUT          	0x03
#define PIN_MODE_INPUT_PULLUP   	0x04
#define PIN_MODE_INPUT_PULLDOWN 	0x05
#define PIN_MODE_OUTPUT_OD      	0x06
#define PIN_MODE_AF      			0x07

#define PIN_IRQ_MODE_RISING             0x00
#define PIN_IRQ_MODE_FALLING            0x01
#define PIN_IRQ_MODE_RISING_FALLING     0x02
#define PIN_IRQ_MODE_HIGH_LEVEL         0x03
#define PIN_IRQ_MODE_LOW_LEVEL          0x04

#define PIN_IRQ_DISABLE                 0x00
#define PIN_IRQ_ENABLE                  0x01

#define PIN_IRQ_PIN_NONE                -1

#ifdef __cplusplus
}
#endif

void gd32_pin_mode(uint32_t pin, uint32_t mode);
void gd32_pin_write(uint32_t pin, uint32_t value);
int gd32_pin_read(uint32_t pin);
uint32_t pin_irq_install(uint32_t pin, uint32_t mode,
                         uint32_t irqMode,
                         void (*hdr)(void *args),
                         void  *args,
                         void *priority);
uint32_t gd32_pin_irq_enable(uint32_t pin, uint32_t enabled);

#endif
