#include "stdio.h"
#include "drv_timer.h"



void timer1_pwminputcapture_init(void)
{
    timer_ic_parameter_struct timer_icinitpara;
    timer_parameter_struct timer_initpara;

    rcu_periph_clock_enable(RCU_GPIOA);

    /*configure PB4 (TIMER2 CH0) as alternate function*/
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_2);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_2);
    gpio_af_set(GPIOA, GPIO_AF_1, GPIO_PIN_2);

    nvic_irq_enable(TIMER1_IRQn, 1, 0);

    rcu_periph_clock_enable(RCU_TIMER1);
    rcu_timer_clock_prescaler_config(RCU_TIMER_PSC_MUL4);

    timer_deinit(TIMER1);

    /* TIMER2 configuration */
    timer_initpara.prescaler         = 199;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = 65535;
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0;
    timer_init(TIMER1, &timer_initpara);

    /* TIMER2 configuration */
    /* TIMER2 CH0 PWM input capture configuration */
    timer_icinitpara.icpolarity  = TIMER_IC_POLARITY_RISING;
    timer_icinitpara.icselection = TIMER_IC_SELECTION_DIRECTTI;
    timer_icinitpara.icprescaler = TIMER_IC_PSC_DIV1;
    timer_icinitpara.icfilter    = 0x5;
    timer_input_pwm_capture_config(TIMER1, TIMER_CH_2, &timer_icinitpara);

    /* slave mode selection: TIMER2 */
    timer_input_trigger_source_select(TIMER1, TIMER_SMCFG_TRGSEL_CI0FE0);
    timer_slave_mode_select(TIMER1, TIMER_SLAVE_MODE_RESTART);

    /* select the master slave mode */
    timer_master_slave_mode_config(TIMER1, TIMER_MASTER_SLAVE_MODE_ENABLE);

    /* auto-reload preload enable */
    timer_auto_reload_shadow_enable(TIMER1);
    /* clear channel 0 interrupt bit */
    timer_interrupt_flag_clear(TIMER1, TIMER_INT_CH2);
    /* channel 0 interrupt enable */
    timer_interrupt_enable(TIMER1, TIMER_INT_CH2);

    /* TIMER2 counter enable */
    timer_enable(TIMER1);
}

/**************************************************************************************************/
/*                                                                                                */
/*                    					timer3_init                                               */
/*                                                                                                */
/**************************************************************************************************/
/**
 * @brief   timer3 initialization  1000Hz
 * @param   none
 *
 * @retval  None
 */
void timer3_init(void)
{
    timer_parameter_struct timer_initpara;

    rcu_periph_clock_enable(RCU_TIMER3);

    rcu_timer_clock_prescaler_config(RCU_TIMER_PSC_MUL4);//200M

    nvic_irq_enable(TIMER3_IRQn, 3, 0);

    timer_deinit(TIMER3);

    /* TIMER3 configuration */
    timer_initpara.prescaler         = 200 - 1;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = 1;
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0;

    timer_init(TIMER3, &timer_initpara);
    /* enable TIMERI auto-reload shadow function */
    timer_auto_reload_shadow_enable( TIMER3 );
    timer_update_source_config(TIMER3, TIMER_UPDATE_SRC_REGULAR);
    /* clear the update interrupt bit */
    timer_interrupt_flag_clear( TIMER3, TIMER_INT_UP );
    /* enable the update interrupt */
    timer_interrupt_enable( TIMER3, TIMER_INT_UP);
    /* TIMERI counter enable */
    timer_enable( TIMER3);
}


uint8_t ccnumber = 0;
uint32_t ic1value = 0, ic2value = 0;
__IO uint16_t dutycycle = 0;
__IO uint16_t frequency = 0;
uint16_t count;
void TIMER1_IRQHandler(void)
{
    if(SET == timer_interrupt_flag_get(TIMER1, TIMER_INT_CH2))
    {
        /* clear channel 0 interrupt bit */
        timer_interrupt_flag_clear(TIMER1, TIMER_INT_CH2);

        if(0 == ccnumber)
        {
            /* read channel 0 capture value */
            ic1value = timer_channel_capture_value_register_read(TIMER1, TIMER_CH_2);
            ccnumber = 1;
        }
        else if(1 == ccnumber)
        {
            /* read channel 0 capture value */
            ic2value = timer_channel_capture_value_register_read(TIMER1, TIMER_CH_2);

            if(ic2value > ic1value)
            {
                count = (ic2value - ic1value);
            }
            else
            {
                count = ((0xFFFF - ic1value) + ic2value);
            }

            frequency = (float)1000000 / count;

            ccnumber = 0;
        }
    }

}

void TIMER3_IRQHandler()
{
    if(SET == timer_interrupt_flag_get(TIMER3, TIMER_INT_UP))
    {
        timer_interrupt_flag_clear( TIMER3, TIMER_INT_UP );

        
    }
}

void gd32_timer_init(void)
{
    //timer1_pwminputcapture_init();
    timer3_init();
}


