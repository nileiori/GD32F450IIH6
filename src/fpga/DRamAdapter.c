#include "DRamAdapter.h"
#include "FPGADefine.h"

#define DRAM_BASE_ADDR		FPGA_DRAM_ADDR
#define DRAM_DATA(offset)	*(volatile uint16_t *)(DRAM_BASE_ADDR + ((offset) << 1))

#define DRAM_SIZE           32768 // 32K x 16

bool DRam_Init(void)
{
	return true;
}

bool DRam_Read(uint32_t addr, uint16_t* data, uint32_t len)
{
    uint32_t i = 0;

    if ((addr + len) > DRAM_SIZE) return false;

#ifndef WIN32
    for (i = 0; i < len; i++)
    {
        data[i] = DRAM_DATA(addr + i);
    }
#endif
    return true;
}

bool DRam_Write(uint32_t addr, uint16_t* data, uint32_t len)
{
    uint32_t i = 0;

    if ((addr + len) > DRAM_SIZE) return false;
#ifndef WIN32
    for (i = 0; i < len; i++)
    {
        DRAM_DATA(addr + i) = data[i];
    }
#endif
    return true;
}
