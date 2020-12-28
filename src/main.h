
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

void Error_Handler(void);

#define DBG_INF_ON (GPIOF->ODR = (GPIOF->ODR | 0b001))
#define DBG_INF_OFF (GPIOF->ODR = (GPIOF->ODR & 0b110))
#define DBG_WRN_ON (GPIOF->ODR = (GPIOF->ODR | 0b010))
#define DBG_WRN_OFF (GPIOF->ODR = (GPIOF->ODR & 0b101))
#define DBG_ERR_ON (GPIOF->ODR = (GPIOF->ODR | 0b100))
#define DBG_ERR_OFF (GPIOF->ODR = (GPIOF->ODR & 0b011))

#define DATA_FLASH_LEN (0x1000)

void DumpClear();
void DumpWrite16(uint32_t offset, uint16_t data);
void DumpWrite32(uint32_t offset, uint32_t data);

#ifdef __cplusplus
}
#endif

#endif