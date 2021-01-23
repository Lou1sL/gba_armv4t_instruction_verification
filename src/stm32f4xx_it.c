
#include "main.h"
#include "stm32f4xx_it.h"

void SysTick_Handler   (void) { HAL_IncTick(); }
void NMI_Handler       (void) { DBG_ERR_TRAP; }
void HardFault_Handler (void) { DBG_ERR_TRAP; }
void MemManage_Handler (void) { DBG_ERR_TRAP; }
void BusFault_Handler  (void) { DBG_ERR_TRAP; }
void UsageFault_Handler(void) { DBG_ERR_TRAP; }
void SVC_Handler       (void) { DBG_ERR_TRAP; }
void DebugMon_Handler  (void) { DBG_ERR_TRAP; }
void PendSV_Handler    (void) { DBG_ERR_TRAP; }

extern TIM_HandleTypeDef timHandle;

void TIM4_IRQHandler(void) {
  HAL_TIM_IRQHandler(&timHandle);
}

void DMA1_Stream0_IRQHandler(void) {
  //HAL_DMA_IRQHandler(timHandle.hdma[TIM_DMA_ID_CC1]);
  //if((DMA1->ISR & DMA_FLAG_TC5) != RESET) { //STM32L4
  // DMA_Handle_index TIM DMA Handle Index /*!< Index of the DMA handle used for Capture/Compare 1 DMA requests */
  if(__HAL_DMA_GET_FLAG(timHandle.hdma[TIM_DMA_ID_CC1], __HAL_DMA_GET_TC_FLAG_INDEX(timHandle.hdma[TIM_DMA_ID_CC1])) != RESET){

    //end of data transfer
    DBG_INF_ON;

    //DMA1->IFCR |= DMA_FLAG_TC5; //STM32L4
    __HAL_DMA_CLEAR_FLAG(timHandle.hdma[TIM_DMA_ID_CC1], __HAL_DMA_GET_TC_FLAG_INDEX(timHandle.hdma[TIM_DMA_ID_CC1]));
    timHandle.hdma[TIM_DMA_ID_CC1]->ErrorCode |= HAL_DMA_ERROR_NONE;
    timHandle.hdma[TIM_DMA_ID_CC1]->State = HAL_DMA_STATE_READY;
    __HAL_UNLOCK(timHandle.hdma[TIM_DMA_ID_CC1]);
    timHandle.hdma[TIM_DMA_ID_CC1]->XferCpltCallback(timHandle.hdma[TIM_DMA_ID_CC1]);
  }
}