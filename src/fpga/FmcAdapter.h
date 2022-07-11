
#ifndef __FMC_ADAPTER_H
#define __FMC_ADAPTER_H

#include "gd32f4xx.h"

typedef enum
{
    SPACE_COM,
    SPACE_RAM,
}EFpgaSpace;


extern uint16_t  FMC_ReadWord(EFpgaSpace space, uint32_t addr);
extern void FMC_WriteWord(EFpgaSpace space, uint32_t addr, uint16_t data);
extern void FMC_ReadBuffer(EFpgaSpace space, uint32_t addr, uint16_t *pBuffer, uint32_t size);
extern void FMC_WriteBuffer(EFpgaSpace space, uint32_t addr, uint16_t *pBuffer, uint32_t size);


#endif

