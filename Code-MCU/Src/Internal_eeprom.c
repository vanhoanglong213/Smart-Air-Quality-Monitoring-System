#include <Internal_eeprom.h>
//#include "stm32f1xx_hal_flash.h"

extern void    FLASH_PageErase(uint32_t PageAddress);

void Flash_Lock()
{
	HAL_FLASH_Lock();
}

void Flash_Unlock()
{
	HAL_FLASH_Unlock();
}

void Flash_Write_Int(uint32_t addr, uint8_t data[])
{
		unsigned int i;

		 if((addr - 0x8000000)%0x800==0) //Erase new page if data locate at new page
		 {
			Flash_Unlock();
			FLASH_PageErase(addr);
		 } 
		 for(i = 0;i<4;i++) 
					HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr+4*i, data[i]);
		}

uint32_t Flash_Read(uint32_t addr)
{
	uint32_t* val = (uint32_t *)addr;
	return *val;
}

void Flash_Read_Int(uint32_t addr, uint8_t data[])
{
 unsigned int i;
 for(i = 0;i<4;i++ ) 
	data[i] = (unsigned char)(Flash_Read(addr+4*i));
}
