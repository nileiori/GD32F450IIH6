#ifndef DRV_RTC_H_INCLUDED
#define DRV_RTC_H_INCLUDED


#ifdef __cplusplus
extern "C" {
#endif

#include "insdef.h"
#include "gd32f4xx.h"

void rtc_configuration(void);

char* rtc_update(void);

#ifdef __cplusplus
}
#endif

#endif // gd32F20X_40X_SPI_H_INCLUDED




