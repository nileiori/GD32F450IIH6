
#ifndef __SPI_ADAPTER_H
#define __SPI_ADAPTER_H

#include "stm32f4xx.h"


typedef enum ESpiType
{
    SPI_1 = 0, 
}SpiType;

#define SPI_W5500 SPI_1

extern void Spi_Init(SpiType spiType);
extern u16 Spi_ReadWriteData(SpiType spiType, u16 data);
extern void Spi_WriteData(SpiType spiType, u16 data);
extern void Spi_CS(SpiType spiType,u8 val);

#endif

