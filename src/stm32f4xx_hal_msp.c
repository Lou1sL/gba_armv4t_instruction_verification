
#include "main.h"

extern DMA_HandleTypeDef hdma_tim4_ch1;

void HAL_MspInit(void) {
  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_RCC_PWR_CLK_ENABLE();
}

void HAL_TIM_IC_MspInit(TIM_HandleTypeDef* htim_ic) {
  if(htim_ic->Instance==TIM4) {
    __HAL_RCC_TIM4_CLK_ENABLE();
    hdma_tim4_ch1.Instance = DMA1_Stream0;
    hdma_tim4_ch1.Init.Channel = DMA_CHANNEL_2;
    hdma_tim4_ch1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_tim4_ch1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_tim4_ch1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_tim4_ch1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_tim4_ch1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_tim4_ch1.Init.Mode = DMA_NORMAL;
    hdma_tim4_ch1.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    hdma_tim4_ch1.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    hdma_tim4_ch1.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    hdma_tim4_ch1.Init.MemBurst = DMA_MBURST_SINGLE;
    hdma_tim4_ch1.Init.PeriphBurst = DMA_PBURST_SINGLE;
    if (HAL_DMA_Init(&hdma_tim4_ch1) != HAL_OK) Error_Handler();
    __HAL_LINKDMA(htim_ic,hdma[TIM_DMA_ID_CC1],hdma_tim4_ch1);
  }
}

void HAL_TIM_IC_MspDeInit(TIM_HandleTypeDef* htim_ic) {
  if(htim_ic->Instance==TIM4) {
    __HAL_RCC_TIM4_CLK_DISABLE();
    HAL_DMA_DeInit(htim_ic->hdma[TIM_DMA_ID_CC1]);
  }
}