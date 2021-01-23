
#include "main.h"

void HAL_MspInit(void) {
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim) {
    static DMA_HandleTypeDef  hdma_tim4_ch1;

    __HAL_RCC_TIM4_CLK_ENABLE();
    __HAL_RCC_DMA1_CLK_ENABLE();

    hdma_tim4_ch1.Instance = DMA1_Stream0;
    hdma_tim4_ch1.Init.Channel = DMA_CHANNEL_2;
    hdma_tim4_ch1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_tim4_ch1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_tim4_ch1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_tim4_ch1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_tim4_ch1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_tim4_ch1.Init.Mode = DMA_CIRCULAR;
    hdma_tim4_ch1.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    //hdma_tim4_ch1.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    //hdma_tim4_ch1.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    //hdma_tim4_ch1.Init.MemBurst = DMA_MBURST_SINGLE;
    //hdma_tim4_ch1.Init.PeriphBurst = DMA_PBURST_SINGLE;
    __HAL_LINKDMA(htim, hdma[TIM_DMA_ID_CC1], hdma_tim4_ch1);
    if(HAL_DMA_Init(htim->hdma[TIM_DMA_ID_CC1]) != HAL_OK) ErrorHandler();

    HAL_NVIC_SetPriority(TIM4_IRQn, 0, 2);
    HAL_NVIC_EnableIRQ(TIM4_IRQn);

    HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);
}