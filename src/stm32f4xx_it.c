
#include "main.h"
#include "stm32f4xx_it.h"

void SysTick_Handler (void) {
  HAL_IncTick();
  HAL_SYSTICK_IRQHandler();
}

extern DMA_HandleTypeDef hdma_tim4_ch1;
extern uint16_t dmabuf[512];
void DMA1_Stream0_IRQHandler(void) {
  HAL_DMA_IRQHandler(&hdma_tim4_ch1);
  //end of data transfer
  DBG_INF_ON;
  for(int i=0; i<512; i++) DumpWrite32(4*i, dmabuf[i]);
  while(1);
}