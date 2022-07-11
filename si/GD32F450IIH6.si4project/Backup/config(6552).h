#ifndef __CONFIG_H__
#define __CONFIG_H__


#include "gd32f4xx.h"
#include "stdbool.h"

//#define BOOT_LOADER
#ifdef BOOT_LOADER
#define PRODUCT_CONFIG_ADDRESS              0x08003800
#define SYSTEM_CONFIG_ADDRESS               0x08004000  // first 4 byte 
#define APP_LOADED_ADDR                     ((uint32_t)0x00004800)                       /*!< APP IAP address */
#else
#define PRODUCT_CONFIG_ADDRESS              0x0801F000
#define SYSTEM_CONFIG_ADDRESS               0x0801F800  // first 4 byte 
#define APP_LOADED_ADDR                     ((uint32_t)0x0)                       /*!< APP IAP address */
#endif

#define hw_interrupt_disable() __set_PRIMASK(1)//;__set_FAULTMASK(1)
#define hw_interrupt_enable()  __set_PRIMASK(0)//;__set_FAULTMASK(0)

#define _STR(s)             #s					 
#define STR(s)              _STR(s)

#define INS_NAME_MAX 12
#define INS_ALIGN_SIZE 4

#define INS_FW_VERSION                      1     
#define INS_FW_SUBVERSION                   0   
#define INS_FW_REVISION                     0     

#define INS_PROTOCOL_VERSION                1
#define INS_PROTOCOL_SUBVERSION             0
#define INS_PROTOCOL_REVISION               0

#define INS_FW_DATE_YEAR                    22              
#define INS_FW_DATE_MONTH                   04            
#define INS_FW_DATE_DAY                     27          
#define INS_FW_DATE_HOUR                    15              
#define INS_FW_DATE_MINUTE                  30            
#define INS_FW_DATE_SECOND                  29

/* INS-Device version */
#define UG_DEVICE_VERSION               ((INS_FW_VERSION * 10000) + \
                                        (INS_FW_SUBVERSION * 100) + INS_FW_REVISION)

#define UG_HAND_PAD_PROTOCOL_VER 		STR(INS_PROTOCOL_VERSION)"."STR(INS_PROTOCOL_SUBVERSION)"."STR(INS_PROTOCOL_REVISION)

#define UG_HAND_PAD_FIRMWARE_DATE 		STR(INS_FW_DATE_YEAR)"-"STR(INS_FW_DATE_MONTH)"-"STR(INS_FW_DATE_DAY)"-"STR(INS_FW_DATE_HOUR)STR(INS_FW_DATE_MINUTE)STR(INS_FW_DATE_SECOND)

//#define  configUse_wdog

//#define  configUse_RTC

#define INS_USING_UART4
#define INS_USING_UART4_DMA0



#define	configUse_SEGGER_RTT
#ifdef	configUse_SEGGER_RTT
#include "..\SEGGER_RTT_V766a\RTT\SEGGER_RTT.h"
#endif



#endif
