
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

extern DMA_HandleTypeDef hdma_tim4_ch1;
void DMA1_Stream0_IRQHandler(void) {
  HAL_DMA_IRQHandler(&hdma_tim4_ch1);
}