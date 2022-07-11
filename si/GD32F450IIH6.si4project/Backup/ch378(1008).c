/*
* File Name          : ch378.C
* Change Logs:
* Date					 Author 			 	Notes
* 2022.05.26		 	 zwb					CH378芯片 硬件标准SPI串行连接的硬件抽象层
*/

#include "string.h"
#include "drv_spi.h"
#include "drv_gpio.h"
#include "pin_numbers_def.h"
#include "ch378.h"
#include "systick.h"
#include "config.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"
#include "message_buffer.h"

TaskHandle_t task_ch378_handler;
SemaphoreHandle_t xCh378Semaphore = NULL;
QueueHandle_t xCh378Queue = NULL;

uint8_t product_info[64];
uint16_t dataLen;
uint8_t  disk_status;
/*******************************************************************************
* Function Name  : ch378_query_interrupt
* Description    : 查询CH378中断(INT#低电平)
* Input          : None
* Output         : 管脚的电平状态
* Return         : 返回中断状态
*******************************************************************************/
uint8_t ch378_query_interrupt( void )
{
    return gd32_pin_read(CH378_INT_PIN_NUM);
}

/*******************************************************************************
* Function Name  : CH378_Port_Init
* Description    : CH378端口初始化
*
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ch378_port_init( void )
{
    irq_priority priority =
    {
        .nvic_irq_pre_priority = 1,
        .nvic_irq_sub_priority = 0
    };
    hw_spi_init(SPI3);
    //gd32_pin_mode(CH378_INT_PIN_NUM, PIN_MODE_INPUT_PULLUP);
    gd32_pin_mode(CH378_RST1_PIN_NUM, PIN_MODE_OUTPUT);
    gd32_pin_write(CH378_RST1_PIN_NUM, PIN_LOW);//rst1脚拉低
    //绑定中断
    pin_irq_install(  CH378_INT_PIN_NUM, PIN_MODE_INPUT_PULLUP,
                      PIN_IRQ_MODE_FALLING,
                      NULL,
                      NULL,
                      &priority);
}

void ch378_cs_select(ControlStatus cs_en)
{
    if(ENABLE == cs_en)
        gd32_pin_write(SPI3_NSS_PIN_NUM, PIN_LOW);
    else
        gd32_pin_write(SPI3_NSS_PIN_NUM, PIN_HIGH);
}
//读固件版本
uint8_t ch378_get_ic_vere( void )
{
    uint8_t cmd, dataRecv;
    cmd = CMD01_GET_IC_VER;
    spi_send(&cmd, 1, 1, 0);
    spi_recv(&dataRecv, 1, 1, 1);

    return dataRecv;
}

uint8_t ch378_get_intStatus( void )
{
    uint8_t cmd, status;
    cmd = CMD01_GET_STATUS;		/* 发送读取中断状态命令 */
    spi_send(&cmd, 1, 1, 0);
    spi_recv(&status, 1, 1, 1);

    return status;
}

//sleep_mode:HALF_SLEEP_MODE or FULL_SLEEP_MODE
void ch378_enter_sleep(uint8_t sleep_mode)
{
    uint8_t cmd;

    cmd = CMD10_ENTER_SLEEP;
    spi_send(&cmd, 1, 1, 0);
    cmd = sleep_mode;
    spi_send(&cmd, 1, 1, 1);
    CH378_CS_DISABLE;//保险一点
}

uint8_t ch378_wake_up(uint8_t sleep_mode)
{
    uint8_t cmd, ret = ERR_SUCCESS;
    uint8_t dataRecv;

    //片选有效可以退出低功耗
    CH378_CS_ENABLE;

    //深度睡眠需要重新初始化
    if(FULL_SLEEP_MODE == sleep_mode)
    {
        cmd = CMD11_CHECK_EXIST;
        spi_send(&cmd, 1, 1, 0);
        cmd = 0xaa;
        spi_send(&cmd, 1, 1, 0);
        spi_recv(&dataRecv, 1, 1, 1);

        if(dataRecv != 0x55)//取反
        {
            return ERR_PARAMETER_ERROR;
        }

        cmd = CMD11_SET_USB_MODE;
        spi_send(&cmd, 1, 1, 0);
        cmd = 0x06;
        spi_send(&cmd, 1, 1, 1);

        //等待完全回复正常后会产生USB_INT_WAKE_UP 事件中断
        if(pdTRUE == xSemaphoreTake(xCh378Semaphore, pdMS_TO_TICKS(100)))
        {
            ret = ch378_get_intStatus();
        }
        else
        {
            ret = ERR_DISK_DISCON;
        }
    }

    return ret;
}

uint8_t ch378_disk_connect(void)
{
    uint8_t cmd;

    cmd = CMD0H_DISK_CONNECT;//检查磁盘是否连接
    spi_send(&cmd, 1, 1, 1);
    disk_status = e_DISK_CONNECT;
    xSemaphoreTake(xCh378Semaphore, portMAX_DELAY);//死等中断

    return ch378_get_intStatus();
}

uint8_t ch378_disk_mount(void)
{
    uint8_t cmd;

    cmd = CMD0H_DISK_MOUNT;//检查磁盘是否就绪
    spi_send(&cmd, 1, 1, 1);

    if(pdTRUE == xSemaphoreTake(xCh378Semaphore, pdMS_TO_TICKS(100)))
    {
        return ch378_get_intStatus();
    }
    else
    {
        return ERR_DISK_DISCON;
    }
}

//从主机端点接收缓冲区读数据并返回长度
uint8_t ch378_read_block( uint8_t* dataRecv )
{
    uint8_t cmd, data_l, data_h;
    uint16_t len;

    cmd = CMD00_RD_HOST_REQ_DATA;
    spi_send(&cmd, 1, 1, 0);
    spi_recv(&data_l, 1, 1, 0);
    spi_recv(&data_h, 1, 1, 0);
    len = data_h << 8 | data_l;

    return spi_recv(dataRecv, len, 1, 1);

}
//检查文件名是否合法
uint8_t ch378_filename_is_valid(const char* name)
{
    uint8_t i, len;
    uint8_t ret = ERR_SUCCESS;

    const char* c = name;
    len = strlen(name);

    for(i = 0; i < len; i++)
    {
        if(*c == DEF_SEPAR_CHAR1 || *c == DEF_SEPAR_CHAR2)
        {
            ret = ERR_PARAMETER_ERROR;
            break;
        }

        c++;
    }

    return ret;
}

//设置文件名
uint8_t ch378_set_filename(const char* name)
{
    uint8_t cmd, len = strlen(name);

    if(ERR_SUCCESS != ch378_filename_is_valid(name))return ERR_PARAMETER_ERROR;//文件名非法

    cmd = CMD10_SET_FILE_NAME;
    spi_send(&cmd, 1, 1, 0);
    spi_send(name, len, 1, 1);
    return ERR_SUCCESS;
}

uint8_t ch378_dir_create(const char* name)
{
    uint8_t cmd;

    if(ERR_SUCCESS != ch378_set_filename(name))return ERR_PARAMETER_ERROR;//文件名非法

    cmd = CMD0H_DIR_CREATE;//新建目录
    spi_send(&cmd, 1, 1, 1);

    if(pdTRUE == xSemaphoreTake(xCh378Semaphore, pdMS_TO_TICKS(100)))
    {
        return ch378_get_intStatus();
    }
    else
    {
        return ERR_MISS_DIR;
    }
}

uint8_t ch378_file_create(const char* name)
{
    uint8_t cmd;

    if(ERR_SUCCESS != ch378_set_filename(name))return ERR_PARAMETER_ERROR;//文件名非法

    cmd = CMD0H_FILE_CREATE;//新建文件
    spi_send(&cmd, 1, 1, 1);

    if(pdTRUE == xSemaphoreTake(xCh378Semaphore, pdMS_TO_TICKS(100)))
    {
        return ch378_get_intStatus();
    }
    else
    {
        return ERR_MISS_DIR;
    }
}

uint8_t ch378_file_open(const char* name)
{
    uint8_t cmd;

    if(ERR_SUCCESS != ch378_set_filename(name))return ERR_PARAMETER_ERROR;//文件名非法

    cmd = CMD0H_FILE_OPEN;//新建目录
    spi_send(&cmd, 1, 1, 1);

    if(pdTRUE == xSemaphoreTake(xCh378Semaphore, pdMS_TO_TICKS(100)))
    {
        return ch378_get_intStatus();
    }
    else
    {
        return ERR_MISS_DIR;
    }
}

uint32_t ch378_read_var32( uint8_t var )
{
    uint8_t cmd;
    uint32_t size;
    cmd = CMD14_READ_VAR32;
    spi_send(&cmd, 1, 1, 0);
    spi_send(&var, 1, 1, 0);
    spi_recv((uint8_t*)&size, 4, 1, 1);
    return( size );
}

uint32_t ch378_get_filesize( void )
{
    return( ch378_read_var32( VAR32_FILE_SIZE ) );
}

void ch378_task(void* arg)
{
    uint8_t cmd;
    uint8_t dataRecv,status;

    ch378_port_init();
    cmd = CMD11_CHECK_EXIST;
    spi_send(&cmd, 1, 1, 0);
    cmd = 0xaa;
    spi_send(&cmd, 1, 1, 0);
    spi_recv(&dataRecv, 1, 1, 1);

    if(dataRecv != 0x55)//取反
    {
        //do something
    }

    cmd = CMD11_SET_USB_MODE;
    spi_send(&cmd, 1, 1, 0);
    cmd = 0x06;//USB-HOST
    spi_send(&cmd, 1, 1, 0);
    delay_1ms(50);
    spi_recv(&dataRecv, 1, 1, 1);

    if(dataRecv != CMD_RET_SUCCESS)
    {

    }

    xCh378Semaphore = xSemaphoreCreateBinary();
    configASSERT( xCh378Semaphore );
    xSemaphoreTake( xCh378Semaphore, 0 );
	xCh378Queue = xQueueCreate(10, sizeof(uint8_t));
	configASSERT( xCh378Queue );
    for( ;; )
    {
		if(pdTRUE == xQueueReceive( xCh378Queue, &dataRecv, pdMS_TO_TICKS(100)))
		{
			status = ch378_get_intStatus();
			if(ERR_SUCCESS == status)
			{
				switch( dataRecv )
				{
					case e_DISK_CONNECT:
					
					break;

					default:
					break;
				}
			}
		}
        //等待磁盘连接
        while(ERR_SUCCESS != ch378_disk_connect())
        {
            vTaskDelay(100);
        }

        //等待磁盘就绪
        while(ERR_SUCCESS != ch378_disk_mount())
        {
            vTaskDelay(100);
        }
		
        dataRecv = ch378_get_ic_vere();
        #ifdef	configUse_SEGGER_RTT
        SEGGER_RTT_printf(0, "\r\nic ver is %d", dataRecv);
        #endif
        dataLen = ch378_read_block(product_info);
        #ifdef	configUse_SEGGER_RTT
        SEGGER_RTT_printf(0, "\r\nproduct info is %s", ((P_INQUIRY_DATA)product_info)->VendorIdStr);
        #endif
        ch378_dir_create("/2022year");
    }
}

void ch378_init(void)
{
    xTaskCreate( ch378_task,
                 "ch378_task",
                 configMINIMAL_STACK_SIZE,
                 ( void * ) NULL,
                 tskIDLE_PRIORITY + 2,
                 (TaskHandle_t*)&task_ch378_handler );

}

