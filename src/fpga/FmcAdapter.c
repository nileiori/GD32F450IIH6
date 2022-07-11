
#include "FmcAdapter.h"
#include "FPGADefine.h"

void FMC_ReadBuffer(EFpgaSpace space, uint32_t addr, uint16_t* pBuffer, uint32_t size)
{
	__IO uint32_t pointer = (uint32_t)addr;
    __IO uint32_t base = (space == SPACE_COM)? FPGA_BASE_ADDR : FPGA_DRAM_ADDR;

	for (; size != 0; size--)
	{
		*pBuffer++ = *(__IO uint16_t*) (base + ADDR_SHIFT(pointer));
		pointer++;
	}
}


void FMC_WriteBuffer(EFpgaSpace space, uint32_t addr, uint16_t* pBuffer, uint32_t size)
{
	__IO uint32_t pointer = (uint32_t)addr;
    __IO uint32_t base = (space == SPACE_COM)? FPGA_BASE_ADDR : FPGA_DRAM_ADDR;

	for (; size != 0; size--)
	{
		*(uint16_t*)(base + ADDR_SHIFT(pointer)) = *pBuffer++;
		pointer++;
	}
}

uint16_t FMC_ReadWord(EFpgaSpace space, uint32_t addr)
{
    __IO uint32_t base = (space == SPACE_COM)? FPGA_BASE_ADDR : FPGA_DRAM_ADDR;
	return (*(__IO uint16_t*)(base + ADDR_SHIFT(addr)));
}

void FMC_WriteWord(EFpgaSpace space, uint32_t addr, uint16_t data)
{
    __IO uint32_t base = (space == SPACE_COM)? FPGA_BASE_ADDR : FPGA_DRAM_ADDR;
	*(uint16_t*)(base + ADDR_SHIFT(addr)) = data;
}



