/*!
    \file    main.c
    \brief   led spark with systick

    \version 2016-08-15, V1.0.0, firmware for GD32F4xx
    \version 2018-12-12, V2.0.0, firmware for GD32F4xx
    \version 2020-09-30, V2.1.0, firmware for GD32F4xx
    \version 2022-03-09, V3.0.0, firmware for GD32F4xx
*/

/*
    Copyright (c) 2022, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors
       may be used to endorse or promote products derived from this software without
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
OF SUCH DAMAGE.
*/

#include "gd32f4xx.h"
#include "systick.h"
//#include "fmc_operation.h"
#include <stdio.h>
//#include "../src/SDRAM/exmc_sdram.h"
#include "config.h"
#include "ch378.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"
#include "message_buffer.h"



TaskHandle_t task_start_handler;


static void prvSetupHardware( void )
{
    SystemInit();

    SystemCoreClockUpdate();

    nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);

    systick_config();

    #ifdef	configUse_SEGGER_RTT
    SEGGER_RTT_Init();
    SEGGER_RTT_printf(0, "\r\nCK_SYS is %d", rcu_clock_freq_get(CK_SYS));
    SEGGER_RTT_printf(0, "\r\nCK_AHB is %d", rcu_clock_freq_get(CK_AHB));
    SEGGER_RTT_printf(0, "\r\nCK_APB1 is %d", rcu_clock_freq_get(CK_APB1));
    SEGGER_RTT_printf(0, "\r\nCK_APB2 is %d", rcu_clock_freq_get(CK_APB2));
    #endif
}

void start_task(void* arg)
{
    taskENTER_CRITICAL();

    ch378_init();
    vTaskDelete(task_start_handler);

    taskEXIT_CRITICAL();
}


/*!
    \brief    main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{
    __set_PRIMASK(1);

    prvSetupHardware();

    xTaskCreate( start_task,
                 "start_task",
                 configMINIMAL_STACK_SIZE,
                 ( void * ) NULL,
                 tskIDLE_PRIORITY + 1,
                 (TaskHandle_t*)&task_start_handler );

    /* Start the scheduler. */
    vTaskStartScheduler();

    for(;;);
}


/**
  * @}
  */

void vApplicationIdleHook( void )
{


}

void vApplicationTickHook( void )
{

}

void OS_PreSleepProcessing(uint32_t ulExpectedIdleTime)
{

    
}

void OS_PostSleepProcessing(uint32_t ulExpectedIdleTime)
{

    
}

void vApplicationMallocFailedHook( void )
{

    taskDISABLE_INTERRUPTS();

    for( ;; );
}

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
    ( void ) pcTaskName;
    ( void ) pxTask;

    /* Run time stack overflow checking is performed if
    configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
    function is called if a stack overflow is detected. */
    taskDISABLE_INTERRUPTS();

    for( ;; );
}

