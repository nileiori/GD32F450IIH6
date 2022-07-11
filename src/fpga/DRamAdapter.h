#ifndef _DRAM_ADAPTER_H_
#define _DRAM_ADAPTER_H_

#include "stdbool.h"
#include "gd32f4xx.h"

#ifdef __cplusplus
extern "C" {
#endif

extern bool DRam_Init(void);
extern bool DRam_Read(uint32_t addr, uint16_t* data, uint32_t len);
extern bool DRam_Write(uint32_t addr, uint16_t* data, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif
