
#include "main.h"
#include "stm32f4xx_it.h"

void NMI_Handler        (void) { while (1) {  } }
void HardFault_Handler  (void) { while (1) {  } }
void MemManage_Handler  (void) { while (1) {  } }
void BusFault_Handler   (void) { while (1) {  } }
void UsageFault_Handler (void) { while (1) {  } }
void SVC_Handler        (void) { }
void DebugMon_Handler   (void) { }
void PendSV_Handler     (void) { }
void SysTick_Handler    (void) { HAL_IncTick(); }

#define DBG_OUT_ON GPIOG->ODR = 1
#define DBG_OUT_OFF GPIOG->ODR = 0
void EXTI0_IRQHandler(void) {
  DBG_OUT_ON;
  asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");
  DBG_OUT_OFF;
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}