/*
 * File      : drv_gpio.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2015, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author            Notes
 * 2017-10-20     ZYH            the first version
 * 2018-04-23     misonyo        port to gd32f4xx
 * 2022-05-25     zwb        		 cut to gd32f4xx
 */

#include "drv_gpio.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"


#define GD32F4XX_PIN_NUMBERS 176 //[144, 176]

#define __GD32_PIN(index, port, pin) { 0, RCU_##GPIO##port, GPIO##port, GPIO_PIN_##pin, EXTI_SOURCE_GPIO##port, EXTI_SOURCE_PIN##pin}
#define __GD32_PIN_DEFAULT {-1, (rcu_periph_enum)0, 0, 0, 0}

/* GD32 GPIO driver */
struct pin_index
{
    int16_t index;
    rcu_periph_enum clk;
    uint32_t gpio_periph;
    uint32_t pin;
    uint8_t port_src;
    uint8_t pin_src;
};

static const struct pin_index pins[] =
{
    #if GD32F4XX_PIN_NUMBERS == 144
    __GD32_PIN_DEFAULT,
    __GD32_PIN(1, E, 2),
    __GD32_PIN(2, E, 3),
    __GD32_PIN(3, E, 4),
    __GD32_PIN(4, E, 5),
    __GD32_PIN(5, E, 6),
    __GD32_PIN_DEFAULT,
    __GD32_PIN(7, C, 13),
    __GD32_PIN(8, C, 14),
    __GD32_PIN(9, C, 15),
    __GD32_PIN(10, F, 0),
    __GD32_PIN(11, F, 1),
    __GD32_PIN(12, F, 2),
    __GD32_PIN(13, F, 3),
    __GD32_PIN(14, F, 4),
    __GD32_PIN(15, F, 5),
    __GD32_PIN_DEFAULT,
    __GD32_PIN_DEFAULT,
    __GD32_PIN(18, F, 6),
    __GD32_PIN(19, F, 7),
    __GD32_PIN(20, F, 8),
    __GD32_PIN(21, F, 9),
    __GD32_PIN(22, F, 10),
    __GD32_PIN_DEFAULT,
    __GD32_PIN_DEFAULT,
    __GD32_PIN_DEFAULT,
    __GD32_PIN(26, C, 0),
    __GD32_PIN(27, C, 1),
    __GD32_PIN(28, C, 2),
    __GD32_PIN(29, C, 3),
    __GD32_PIN_DEFAULT,
    __GD32_PIN_DEFAULT,
    __GD32_PIN_DEFAULT,
    __GD32_PIN_DEFAULT,
    __GD32_PIN(34, A, 0),
    __GD32_PIN(35, A, 1),
    __GD32_PIN(36, A, 2),
    __GD32_PIN(37, A, 3),
    __GD32_PIN_DEFAULT,
    __GD32_PIN_DEFAULT,
    __GD32_PIN(40, A, 4),
    __GD32_PIN(41, A, 5),
    __GD32_PIN(42, A, 6),
    __GD32_PIN(43, A, 7),
    __GD32_PIN(44, C, 4),
    __GD32_PIN(45, C, 5),
    __GD32_PIN(46, B, 0),
    __GD32_PIN(47, B, 1),
    __GD32_PIN(48, B, 2),
    __GD32_PIN(49, F, 11),
    __GD32_PIN(50, F, 12),
    __GD32_PIN_DEFAULT,
    __GD32_PIN_DEFAULT,
    __GD32_PIN(53, F, 13),
    __GD32_PIN(54, F, 14),
    __GD32_PIN(55, F, 15),
    __GD32_PIN(56, G, 0),
    __GD32_PIN(57, G, 1),
    __GD32_PIN(58, E, 7),
    __GD32_PIN(59, E, 8),
    __GD32_PIN(60, E, 9),
    __GD32_PIN_DEFAULT,
    __GD32_PIN_DEFAULT,
    __GD32_PIN(63, E, 10),
    __GD32_PIN(64, E, 11),
    __GD32_PIN(65, E, 12),
    __GD32_PIN(66, E, 13),
    __GD32_PIN(67, E, 14),
    __GD32_PIN(68, E, 15),
    __GD32_PIN(69, B, 10),
    __GD32_PIN(70, B, 11),
    __GD32_PIN_DEFAULT,
    __GD32_PIN_DEFAULT,
    __GD32_PIN(73, B, 12),
    __GD32_PIN(74, B, 13),
    __GD32_PIN(75, B, 14),
    __GD32_PIN(76, B, 15),
    __GD32_PIN(77, D, 8),
    __GD32_PIN(78, D, 9),
    __GD32_PIN(79, D, 10),
    __GD32_PIN(80, D, 11),
    __GD32_PIN(81, D, 12),
    __GD32_PIN(82, D, 13),
    __GD32_PIN_DEFAULT,
    __GD32_PIN_DEFAULT,
    __GD32_PIN(85, D, 14),
    __GD32_PIN(86, D, 15),
    __GD32_PIN(87, G, 2),
    __GD32_PIN(88, G, 3),
    __GD32_PIN(89, G, 4),
    __GD32_PIN(90, G, 5),
    __GD32_PIN(91, G, 6),
    __GD32_PIN(92, G, 7),
    __GD32_PIN(93, G, 8),
    __GD32_PIN_DEFAULT,
    __GD32_PIN_DEFAULT,
    __GD32_PIN(96, C, 6),
    __GD32_PIN(97, C, 7),
    __GD32_PIN(98, C, 8),
    __GD32_PIN(99, C, 9),
    __GD32_PIN(100, A, 8),
    __GD32_PIN(101, A, 9),
    __GD32_PIN(102, A, 10),
    __GD32_PIN(103, A, 11),
    __GD32_PIN(104, A, 12),
    __GD32_PIN(105, A, 13),
    __GD32_PIN_DEFAULT,
    __GD32_PIN_DEFAULT,
    __GD32_PIN_DEFAULT,
    __GD32_PIN(109, A, 14),
    __GD32_PIN(110, A, 15),
    __GD32_PIN(111, C, 10),
    __GD32_PIN(112, C, 11),
    __GD32_PIN(113, C, 12),
    __GD32_PIN(114, D, 0),
    __GD32_PIN(115, D, 1),
    __GD32_PIN(116, D, 2),
    __GD32_PIN(117, D, 3),
    __GD32_PIN(118, D, 4),
    __GD32_PIN(119, D, 5),
    __GD32_PIN_DEFAULT,
    __GD32_PIN_DEFAULT,
    __GD32_PIN(122, D, 6),
    __GD32_PIN(123, D, 7),
    __GD32_PIN(124, G, 9),
    __GD32_PIN(125, G, 10),
    __GD32_PIN(126, G, 11),
    __GD32_PIN(127, G, 12),
    __GD32_PIN(128, G, 13),
    __GD32_PIN(129, G, 14),
    __GD32_PIN_DEFAULT,
    __GD32_PIN_DEFAULT,
    __GD32_PIN(132, G, 15),
    __GD32_PIN(133, B, 3),
    __GD32_PIN(134, B, 4),
    __GD32_PIN(135, B, 5),
    __GD32_PIN(136, B, 6),
    __GD32_PIN(137, B, 7),
    __GD32_PIN_DEFAULT,
    __GD32_PIN(139, B, 8),
    __GD32_PIN(140, B, 9),
    __GD32_PIN(141, E, 0),
    __GD32_PIN(142, E, 1),
    __GD32_PIN_DEFAULT,
    __GD32_PIN_DEFAULT
    #endif
    #if GD32F4XX_PIN_NUMBERS == 176
    __GD32_PIN_DEFAULT,
    __GD32_PIN(1, E, 2),
    __GD32_PIN(2, E, 3),
    __GD32_PIN(3, E, 4),
    __GD32_PIN(4, E, 5),
    __GD32_PIN(5, E, 6),
    __GD32_PIN_DEFAULT,
    __GD32_PIN(7, I, 8),
    __GD32_PIN(8, C, 13),
    __GD32_PIN(9, C, 14),
    __GD32_PIN(10, C, 15),
    __GD32_PIN(11, I, 9),
    __GD32_PIN(12, I, 10),
    __GD32_PIN(13, I, 11),
    __GD32_PIN_DEFAULT,
    __GD32_PIN_DEFAULT,
    __GD32_PIN(16, F, 0),
    __GD32_PIN(17, F, 1),
    __GD32_PIN(18, F, 2),
    __GD32_PIN(19, F, 3),
    __GD32_PIN(20, F, 4),
    __GD32_PIN(21, F, 5),
    __GD32_PIN_DEFAULT,
    __GD32_PIN_DEFAULT,
    __GD32_PIN(24, F, 6),
    __GD32_PIN(25, F, 7),
    __GD32_PIN(26, F, 8),
    __GD32_PIN(27, F, 9),
    __GD32_PIN(28, F, 10),
    __GD32_PIN(29, H, 0),
    __GD32_PIN(30, H, 1),
    __GD32_PIN_DEFAULT,
    __GD32_PIN(32, C, 0),
    __GD32_PIN(33, C, 1),
    __GD32_PIN(34, C, 2),
    __GD32_PIN(35, C, 3),
    __GD32_PIN_DEFAULT,
    __GD32_PIN_DEFAULT,
    __GD32_PIN_DEFAULT,
    __GD32_PIN_DEFAULT,
    __GD32_PIN_DEFAULT,
    __GD32_PIN(41, A, 0),
    __GD32_PIN(42, A, 1),
    __GD32_PIN(43, A, 2),
    __GD32_PIN(44, H, 2),
    __GD32_PIN(45, H, 3),
    __GD32_PIN(46, H, 4),
    __GD32_PIN(47, H, 5),
    __GD32_PIN(48, A, 3),
    __GD32_PIN_DEFAULT,
    __GD32_PIN_DEFAULT,
    __GD32_PIN(51, A, 4),
    __GD32_PIN(52, A, 5),
    __GD32_PIN(53, A, 6),
    __GD32_PIN(54, A, 7),
    __GD32_PIN(55, C, 4),
    __GD32_PIN(56, C, 5),
    __GD32_PIN(57, B, 0),
    __GD32_PIN(58, B, 1),
    __GD32_PIN(59, B, 2),
    __GD32_PIN(60, F, 11),
    __GD32_PIN(61, F, 12),
    __GD32_PIN_DEFAULT,
    __GD32_PIN_DEFAULT,
    __GD32_PIN(64, F, 13),
    __GD32_PIN(65, F, 14),
    __GD32_PIN(66, F, 15),
    __GD32_PIN(67, G, 0),
    __GD32_PIN(68, G, 1),
    __GD32_PIN(69, E, 7),
    __GD32_PIN(70, E, 8),
    __GD32_PIN(71, E, 9),
    __GD32_PIN_DEFAULT,
    __GD32_PIN_DEFAULT,
    __GD32_PIN(74, E, 10),
    __GD32_PIN(75, E, 11),
    __GD32_PIN(76, E, 12),
    __GD32_PIN(77, E, 13),
    __GD32_PIN(78, E, 14),
    __GD32_PIN(79, E, 15),
    __GD32_PIN(80, B, 10),
    __GD32_PIN(81, B, 11),
    __GD32_PIN_DEFAULT,
    __GD32_PIN_DEFAULT,
    __GD32_PIN(84, H, 6),
    __GD32_PIN(85, H, 7),
    __GD32_PIN(86, H, 8),
    __GD32_PIN(87, H, 9),
    __GD32_PIN(88, H, 10),
    __GD32_PIN(89, H, 11),
    __GD32_PIN(90, H, 12),
    __GD32_PIN_DEFAULT,
    __GD32_PIN_DEFAULT,
    __GD32_PIN(93, B, 12),
    __GD32_PIN(94, B, 13),
    __GD32_PIN(95, B, 14),
    __GD32_PIN(96, B, 15),
    __GD32_PIN(97, D, 8),
    __GD32_PIN(98, D, 9),
    __GD32_PIN(99, D, 10),
    __GD32_PIN(100, D, 11),
    __GD32_PIN(101, D, 12),
    __GD32_PIN(102, D, 13),
    __GD32_PIN_DEFAULT,
    __GD32_PIN(104, D, 14),
    __GD32_PIN(105, D, 15),
    __GD32_PIN(106, G, 2),
    __GD32_PIN(107, G, 3),
    __GD32_PIN(108, G, 4),
    __GD32_PIN(109, G, 5),
    __GD32_PIN(110, G, 6),
    __GD32_PIN(111, G, 7),
    __GD32_PIN(112, G, 8),
    __GD32_PIN_DEFAULT,
    __GD32_PIN_DEFAULT,
    __GD32_PIN(115, C, 6),
    __GD32_PIN(116, C, 7),
    __GD32_PIN(117, C, 8),
    __GD32_PIN(118, C, 9),
    __GD32_PIN(119, A, 8),
    __GD32_PIN(120, A, 9),
    __GD32_PIN(121, A, 10),
    __GD32_PIN(122, A, 11),
    __GD32_PIN(123, A, 12),
    __GD32_PIN(124, A, 13),
    __GD32_PIN_DEFAULT,
    __GD32_PIN_DEFAULT,
    __GD32_PIN_DEFAULT,
    __GD32_PIN(128, H, 13),
    __GD32_PIN(129, H, 14),
    __GD32_PIN(130, H, 15),
    __GD32_PIN(131, I, 0),
    __GD32_PIN(132, I, 1),
    __GD32_PIN(133, I, 2),
    __GD32_PIN(134, I, 3),
    __GD32_PIN_DEFAULT,
    __GD32_PIN_DEFAULT,
    __GD32_PIN(137, A, 14),
    __GD32_PIN(138, A, 15),
    __GD32_PIN(139, C, 10),
    __GD32_PIN(140, C, 11),
    __GD32_PIN(141, C, 12),
    __GD32_PIN(142, D, 0),
    __GD32_PIN(143, D, 1),
    __GD32_PIN(144, D, 2),
    __GD32_PIN(145, D, 3),
    __GD32_PIN(146, D, 4),
    __GD32_PIN(147, D, 5),
    __GD32_PIN_DEFAULT,
    __GD32_PIN_DEFAULT,
    __GD32_PIN(150, D, 6),
    __GD32_PIN(151, D, 7),
    __GD32_PIN(152, G, 9),
    __GD32_PIN(153, G, 10),
    __GD32_PIN(154, G, 11),
    __GD32_PIN(155, G, 12),
    __GD32_PIN(156, G, 13),
    __GD32_PIN(157, G, 14),
    __GD32_PIN_DEFAULT,
    __GD32_PIN_DEFAULT,
    __GD32_PIN(160, G, 15),
    __GD32_PIN(161, B, 3),
    __GD32_PIN(162, B, 4),
    __GD32_PIN(163, B, 5),
    __GD32_PIN(164, B, 6),
    __GD32_PIN(165, B, 7),
    __GD32_PIN_DEFAULT,
    __GD32_PIN(167, B, 8),
    __GD32_PIN(168, B, 9),
    __GD32_PIN(169, E, 0),
    __GD32_PIN(170, E, 1),
    __GD32_PIN_DEFAULT,
    __GD32_PIN_DEFAULT,
    __GD32_PIN_DEFAULT,
    __GD32_PIN(174, I, 4),
    __GD32_PIN(175, I, 5),
    __GD32_PIN(176, I, 6),
    __GD32_PIN(177, I, 7)
    #endif

};

struct pin_irq_map
{
    uint16_t pinbit;
    IRQn_Type irqno;
};
static const struct pin_irq_map pin_irq_map[] =
{
    {GPIO_PIN_0, EXTI0_IRQn},
    {GPIO_PIN_1, EXTI1_IRQn},
    {GPIO_PIN_2, EXTI2_IRQn},
    {GPIO_PIN_3, EXTI3_IRQn},
    {GPIO_PIN_4, EXTI4_IRQn},
    {GPIO_PIN_5, EXTI5_9_IRQn},
    {GPIO_PIN_6, EXTI5_9_IRQn},
    {GPIO_PIN_7, EXTI5_9_IRQn},
    {GPIO_PIN_8, EXTI5_9_IRQn},
    {GPIO_PIN_9, EXTI5_9_IRQn},
    {GPIO_PIN_10, EXTI10_15_IRQn},
    {GPIO_PIN_11, EXTI10_15_IRQn},
    {GPIO_PIN_12, EXTI10_15_IRQn},
    {GPIO_PIN_13, EXTI10_15_IRQn},
    {GPIO_PIN_14, EXTI10_15_IRQn},
    {GPIO_PIN_15, EXTI10_15_IRQn},
};

struct rt_pin_irq_hdr
{
    int16_t        pin;
    uint16_t       mode;
    void (*hdr)(void *args);
    void             *args;
    void             *priority;
};

struct rt_pin_irq_hdr pin_irq_hdr_tab[] =
{
    {-1, 0, NULL, NULL, NULL},
    {-1, 0, NULL, NULL, NULL},
    {-1, 0, NULL, NULL, NULL},
    {-1, 0, NULL, NULL, NULL},
    {-1, 0, NULL, NULL, NULL},
    {-1, 0, NULL, NULL, NULL},
    {-1, 0, NULL, NULL, NULL},
    {-1, 0, NULL, NULL, NULL},
    {-1, 0, NULL, NULL, NULL},
    {-1, 0, NULL, NULL, NULL},
    {-1, 0, NULL, NULL, NULL},
    {-1, 0, NULL, NULL, NULL},
    {-1, 0, NULL, NULL, NULL},
    {-1, 0, NULL, NULL, NULL},
    {-1, 0, NULL, NULL, NULL},
    {-1, 0, NULL, NULL, NULL},
};

#define ITEM_NUM(items) sizeof(items) / sizeof(items[0])
const struct pin_index *get_pin(uint8_t pin)
{
    const struct pin_index *index;

    if (pin < ITEM_NUM(pins))
    {
        index = &pins[pin];

        if (index->index == -1)
            index = NULL;
    }
    else
    {
        index = NULL;
    }

    return index;
};

void gd32_pin_mode(uint32_t pin, uint32_t mode)
{
    const struct pin_index *index;
    index = get_pin(pin);

    if (index == NULL)
    {
        return;
    }

    /* GPIO Periph clock enable */
    rcu_periph_clock_enable(index->clk);

    switch(mode)
    {
        case PIN_MODE_OUTPUT:
            /* output setting */
            gpio_mode_set(index->gpio_periph, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, index->pin);
            gpio_output_options_set(index->gpio_periph, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, index->pin);
            break;

        case PIN_MODE_OUTPUT_OD:
            /* output setting: od. */
            gpio_mode_set(index->gpio_periph, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, index->pin);
            gpio_output_options_set(index->gpio_periph, GPIO_OTYPE_OD, GPIO_OSPEED_MAX, index->pin);
            break;

        case PIN_MODE_INPUT:
            /* input setting: not pull. */
            gpio_mode_set(index->gpio_periph, GPIO_MODE_INPUT, GPIO_PUPD_NONE, index->pin);
            break;

        case PIN_MODE_INPUT_PULLUP:
            /* input setting: pull up. */
            gpio_mode_set(index->gpio_periph, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, index->pin);
            break;

        case PIN_MODE_INPUT_PULLDOWN:
            /* input setting: pull down. */
            gpio_mode_set(index->gpio_periph, GPIO_MODE_INPUT, GPIO_PUPD_PULLDOWN, index->pin);
            break;

        //case PIN_MODE_AF:
        //     /* alternate function mode */
        //     gpio_mode_set(index->gpio_periph, GPIO_MODE_AF, GPIO_PUPD_NONE, index->pin);
        //     break;
        default:
            break;
    }

}

void gd32_pin_write(uint32_t pin, uint32_t value)
{
    const struct pin_index *index;

    index = get_pin(pin);

    if (index == NULL)
    {
        return;
    }

    gpio_bit_write(index->gpio_periph, index->pin, (bit_status)value);
}

int gd32_pin_read(uint32_t pin)
{
    int value;
    const struct pin_index *index;

    value = PIN_LOW;

    index = get_pin(pin);

    if (index == NULL)
    {
        return value;
    }

    value = gpio_input_bit_get(index->gpio_periph, index->pin);

    return value;
}

static inline int32_t bit2bitno(uint32_t bit)
{
    uint8_t i;

    for (i = 0; i < 32; i++)
    {
        if ((0x01 << i) == bit)
        {
            return i;
        }
    }

    return -1;
}

static inline const struct pin_irq_map *get_pin_irq_map(uint32_t pinbit)
{
    int32_t mapindex = bit2bitno(pinbit);

    if (mapindex < 0 || mapindex >= ITEM_NUM(pin_irq_map))
    {
        return NULL;
    }

    return &pin_irq_map[mapindex];
};

uint32_t gd32_pin_attach_irq(int32_t pin,
                             uint32_t mode,
                             void (*hdr)(void *args),
                             void *args,
                             void *priority)
{
    const struct pin_index *index;
    int32_t hdr_index = -1;

    index = get_pin(pin);

    if (index == NULL)
    {
        return INS_EINVAL;
    }

    hdr_index = bit2bitno(index->pin);

    if (hdr_index < 0 || hdr_index >= ITEM_NUM(pin_irq_map))
    {
        return INS_EINVAL;
    }

    if (pin_irq_hdr_tab[hdr_index].pin == pin &&
            pin_irq_hdr_tab[hdr_index].hdr == hdr &&
            pin_irq_hdr_tab[hdr_index].mode == mode &&
            pin_irq_hdr_tab[hdr_index].args == args &&
            pin_irq_hdr_tab[hdr_index].priority == priority)
    {
        return INS_EOK;
    }

    if (pin_irq_hdr_tab[hdr_index].pin != -1)
    {
        return INS_EFULL;
    }

    pin_irq_hdr_tab[hdr_index].pin = pin;
    pin_irq_hdr_tab[hdr_index].hdr = hdr;
    pin_irq_hdr_tab[hdr_index].mode = mode;
    pin_irq_hdr_tab[hdr_index].args = args;
    pin_irq_hdr_tab[hdr_index].priority = priority;

    return INS_EOK;
}

uint32_t gd32_pin_detach_irq(int32_t pin)
{
    const struct pin_index *index;

    int32_t hdr_index = -1;

    index = get_pin(pin);

    if (index == NULL)
    {
        return INS_EINVAL;
    }

    hdr_index = bit2bitno(index->pin);

    if (hdr_index < 0 || hdr_index >= ITEM_NUM(pin_irq_map))
    {
        return INS_EINVAL;
    }

    if (pin_irq_hdr_tab[hdr_index].pin == -1)
    {
        return INS_EOK;
    }

    pin_irq_hdr_tab[hdr_index].pin = -1;
    pin_irq_hdr_tab[hdr_index].hdr = NULL;
    pin_irq_hdr_tab[hdr_index].mode = 0;
    pin_irq_hdr_tab[hdr_index].args = NULL;

    return INS_EOK;
}

uint32_t gd32_pin_irq_enable(uint32_t pin, uint32_t enabled)
{
    const struct pin_index *index;
    const struct pin_irq_map *irqmap;
    int32_t hdr_index = -1;
    exti_trig_type_enum trigger_mode;
    irq_priority *priority;

    index = get_pin(pin);

    if (index == NULL)
    {
        return INS_EINVAL;
    }

    if (enabled == PIN_IRQ_ENABLE)
    {
        hdr_index = bit2bitno(index->pin);

        if (hdr_index < 0 || hdr_index >= ITEM_NUM(pin_irq_map))
        {
            return INS_EINVAL;
        }

        if (pin_irq_hdr_tab[hdr_index].pin == -1)
        {
            return INS_EINVAL;
        }

        irqmap = &pin_irq_map[hdr_index];

        switch (pin_irq_hdr_tab[hdr_index].mode)
        {
            case PIN_IRQ_MODE_RISING:
                trigger_mode = EXTI_TRIG_RISING;
                break;

            case PIN_IRQ_MODE_FALLING:
                trigger_mode = EXTI_TRIG_FALLING;
                break;

            case PIN_IRQ_MODE_RISING_FALLING:
                trigger_mode = EXTI_TRIG_BOTH;
                break;

            default:
                return INS_EINVAL;
        }

        rcu_periph_clock_enable(index->clk);

        priority = pin_irq_hdr_tab[hdr_index].priority;
        /* enable and set interrupt priority */
        nvic_irq_enable(irqmap->irqno, priority->nvic_irq_pre_priority, priority->nvic_irq_sub_priority);
        /* connect EXTI line to  GPIO pin */
        syscfg_exti_line_config(index->port_src, index->pin_src);

        /* configure EXTI line */
        exti_init((exti_line_enum)(index->pin), EXTI_INTERRUPT, trigger_mode);
        exti_interrupt_flag_clear((exti_line_enum)(index->pin));

    }
    else if (enabled == PIN_IRQ_DISABLE)
    {
        irqmap = get_pin_irq_map(index->pin);

        if (irqmap == NULL)
        {
            return INS_EINVAL;
        }

        nvic_irq_disable(irqmap->irqno);
    }
    else
    {
        return INS_EINVAL;
    }

    return INS_EOK;
}

static inline void pin_irq_hdr(int irqno)
{
    if (pin_irq_hdr_tab[irqno].hdr)
    {
        pin_irq_hdr_tab[irqno].hdr(pin_irq_hdr_tab[irqno].args);
    }
}

uint32_t pin_irq_install(uint32_t pin, uint32_t mode,
                         uint32_t irqMode,
                         void (*hdr)(void *args),
                         void  *args,
                         void *priority)
{
    gd32_pin_mode(pin, mode);
    gd32_pin_attach_irq(pin, irqMode, hdr, args, priority);
    return gd32_pin_irq_enable(pin, PIN_IRQ_ENABLE);
}

void GD32_GPIO_EXTI_IRQHandler(int8_t exti_line)
{
    if(RESET != exti_interrupt_flag_get((exti_line_enum)(1 << exti_line)))
    {
        pin_irq_hdr(exti_line);
        exti_interrupt_flag_clear((exti_line_enum)(1 << exti_line));
    }
}
void EXTI0_IRQHandler(void)
{
    taskENTER_CRITICAL();
    GD32_GPIO_EXTI_IRQHandler(0);
    taskEXIT_CRITICAL();
}
void EXTI1_IRQHandler(void)
{
    taskENTER_CRITICAL();
    GD32_GPIO_EXTI_IRQHandler(1);
    taskEXIT_CRITICAL();
}
void EXTI2_IRQHandler(void)
{
    taskENTER_CRITICAL();
    GD32_GPIO_EXTI_IRQHandler(2);
    taskEXIT_CRITICAL();
}
void EXTI3_IRQHandler(void)
{
    taskENTER_CRITICAL();
    GD32_GPIO_EXTI_IRQHandler(3);
    taskEXIT_CRITICAL();
}

//extern SemaphoreHandle_t xCh378Semaphore;
extern QueueHandle_t xCh378Queue;
extern uint8_t  disk_status;
void EXTI4_IRQHandler(void)
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	
    taskENTER_CRITICAL();
    GD32_GPIO_EXTI_IRQHandler(4);
    //xSemaphoreGiveFromISR(xCh378Semaphore, &xHigherPriorityTaskWoken);
    xQueueSendFromISR(xCh378Queue, &disk_status, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    taskEXIT_CRITICAL();
}
void EXTI5_9_IRQHandler(void)
{
    taskENTER_CRITICAL();
    GD32_GPIO_EXTI_IRQHandler(5);
    GD32_GPIO_EXTI_IRQHandler(6);
    GD32_GPIO_EXTI_IRQHandler(7);
    GD32_GPIO_EXTI_IRQHandler(8);
    GD32_GPIO_EXTI_IRQHandler(9);
    taskEXIT_CRITICAL();
}
void EXTI10_15_IRQHandler(void)
{

    taskENTER_CRITICAL();
    GD32_GPIO_EXTI_IRQHandler(10);
    GD32_GPIO_EXTI_IRQHandler(11);
    GD32_GPIO_EXTI_IRQHandler(12);
    GD32_GPIO_EXTI_IRQHandler(13);
    GD32_GPIO_EXTI_IRQHandler(14);
    GD32_GPIO_EXTI_IRQHandler(15);
    taskEXIT_CRITICAL();
}

