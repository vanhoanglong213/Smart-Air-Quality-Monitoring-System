#include "stm32f1xx_hal.h"
#include "stdint.h"
#include "string.h"


void 	Flash_Lock(void);
void 	Flash_Unlock(void);

void Flash_Write_Int(uint32_t addr, uint8_t data[]);
uint32_t Flash_Read(uint32_t addr);
void Flash_Read_Int(uint32_t addr, uint8_t data[]);
