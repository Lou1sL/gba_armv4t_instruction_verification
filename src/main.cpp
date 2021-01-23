
#include "main.h"

TIM_HandleTypeDef      timHandle;
TIM_SlaveConfigTypeDef slaveCfg;
TIM_OC_InitTypeDef     ocCfg;
TIM_IC_InitTypeDef     icCfg;

void SystemClock_Init(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) ErrorHandler();
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) ErrorHandler();
}

static void HalfTransferComplete(DMA_HandleTypeDef *DmaHandle);
static void TransferComplete(DMA_HandleTypeDef *DmaHandle);
static void TransferError(DMA_HandleTypeDef *DmaHandle);
static void TIM_Init(void){
  timHandle.Instance = TIM4;
  timHandle.Init.Period            = 0xFFFF;
  timHandle.Init.Prescaler         = 40000 - 1; // 0
  //timHandle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  timHandle.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
  timHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
  timHandle.Init.RepetitionCounter = 0;
  if (HAL_TIM_Base_Init(&timHandle) != HAL_OK) ErrorHandler();

  //NOT SUPPORT: slaveCfg.SlaveMode         = TIM_SLAVEMODE_COMBINED_RESETTRIGGER;
  slaveCfg.SlaveMode         = TIM_SLAVEMODE_TRIGGER;
  slaveCfg.InputTrigger      = TIM_TS_TI1FP1;
  slaveCfg.TriggerPolarity   = TIM_TRIGGERPOLARITY_FALLING;
  slaveCfg.TriggerPrescaler  = TIM_TRIGGERPRESCALER_DIV1;
  slaveCfg.TriggerFilter     = 0;
  if (HAL_TIM_SlaveConfigSynchronization(&timHandle, &slaveCfg) != HAL_OK) ErrorHandler();

  ocCfg.OCMode       = TIM_OCMODE_TOGGLE;
  ocCfg.OCPolarity   = TIM_OCPOLARITY_HIGH;
  ocCfg.Pulse        = 400;
  ocCfg.OCNPolarity  = TIM_OCPOLARITY_HIGH;
  ocCfg.OCFastMode   = TIM_OCFAST_DISABLE;
  ocCfg.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  ocCfg.OCIdleState  = TIM_OCIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&timHandle, &ocCfg, TIM_CHANNEL_2) != HAL_OK) ErrorHandler();

  icCfg.ICPolarity  = TIM_ICPOLARITY_RISING;
  icCfg.ICSelection = TIM_ICSELECTION_DIRECTTI;
  icCfg.ICPrescaler = TIM_ICPSC_DIV1;
  icCfg.ICFilter    = 1; // 0
  if (HAL_TIM_IC_ConfigChannel(&timHandle, &icCfg, TIM_CHANNEL_1) != HAL_OK) ErrorHandler();

  timHandle.hdma[TIM_DMA_ID_CC1]->XferHalfCpltCallback = HalfTransferComplete;
  timHandle.hdma[TIM_DMA_ID_CC1]->XferCpltCallback = TransferComplete;
  timHandle.hdma[TIM_DMA_ID_CC1]->XferErrorCallback = TransferError;
}

static void GPIO_Init(void) {
  
  GPIO_InitTypeDef  GPIO_InitStructure;

  //GPIO RISING INPUT /RD:PB6 (TIM4_CH1 DMA1Stream0)
  //see stm32f4xx_hal_msp.c
  __HAL_RCC_GPIOB_CLK_ENABLE();
  GPIO_InitStructure.Pin = GPIO_PIN_6;
  GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStructure.Alternate = GPIO_AF2_TIM4;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

  //GPIO INPUT /CS1:PC0 /RD:PC1 /WR:PC2 /CS2:PC3
  //GPIOC 0x40020810
  __HAL_RCC_GPIOC_CLK_ENABLE();
  GPIO_InitStructure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3;
  GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
	GPIO_InitStructure.Pull = GPIO_PULLUP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

  //GPIO INPUT A{0-15}:PD{0-15}
  //GPIO INPUT/OUTPUT D{0-15}:PD{0-15}
  //GPIOD 0x40020C10
  __HAL_RCC_GPIOD_CLK_ENABLE();
  GPIO_InitStructure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2  | GPIO_PIN_3  | GPIO_PIN_4  | GPIO_PIN_5  | GPIO_PIN_6  | GPIO_PIN_7 | 
    GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
	GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
	GPIO_InitStructure.Pull = GPIO_PULLUP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);

  //GPIO INPUT A{16-23}:PE{0-7}
  __HAL_RCC_GPIOE_CLK_ENABLE();
  GPIO_InitStructure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2  | GPIO_PIN_3  | GPIO_PIN_4  | GPIO_PIN_5  | GPIO_PIN_6  | GPIO_PIN_7;
  GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
	GPIO_InitStructure.Pull = GPIO_PULLUP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStructure);

  //GPIO OUTPUT DBG_INF:PF0 DBG_WRN:PF1 DBG_ERR:PF2
  __HAL_RCC_GPIOF_CLK_ENABLE();
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_0, GPIO_PIN_RESET);
  GPIO_InitStructure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2;
  GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStructure.Pull = GPIO_PULLDOWN;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStructure);
}

void ErrorHandler(void) {
  __disable_irq();
  DBG_ERR_TRAP;
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line) { }
#endif

#define GPIO_CS1 (GPIOC->IDR & 0b0001)
#define GPIO_RD (GPIOC->IDR & 0b0010)
#define GPIO_WR (GPIOC->IDR & 0b0100)
#define GPIO_CS2 (GPIOC->IDR & 0b1000)

#define GPIO_AD_LO GPIOD->IDR
#define GPIO_AD_HI (GPIOE->IDR & 0xFF)

#define GPIO_DATA_I GPIOD->IDR
#define GPIO_DATA_O GPIOD->ODR

// GBA BUS(NOT CART BUS) CART SEC
//rom ws0 0x08000000 0x09FFFFFF 0x01FFFFFF cs1
//rom ws1 0x0A000000 0x0BFFFFFF 0x01FFFFFF cs1
//rom ws2 0x0C000000 0x0DFFFFFF 0x01FFFFFF cs1
//pak ram 0x0E000000 0x0E00FFFF 0x0000FFFF cs2
#define CART_CS1 (GPIO_CS1 == 0)
#define CART_RD (GPIO_RD == 0)
#define CART_WR (GPIO_WR == 0)
#define CART_CS2 (GPIO_CS2 == 0)

#define CART_DATA (GPIO_D00_D15_I)

#define ADDR(hi,lo) ((hi << 17) | (lo << 1) | 0)



#define INIT_PATTERN 0b1111
#define GPIO_CTRL GPIOC->IDR
#define IS_PATTERN_IN_SEQ(ct) ((ct & 0b1111) == 0b1110)
#define IS_CS1_HI(ct) ((ct & 0b0001) == 0b0001)
#define IS_CS1_LO(ct) ((ct & 0b0001) == 0b0000)
#define IS_RD_LO(ct) ((ct & 0b0010) == 0b0000)

uint16_t data_dump_flash[DATA_FLASH_LEN] __attribute__ ((section(".flash_data")));
void DumpClear(){
  HAL_FLASH_Unlock();
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGSERR );
  FLASH_Erase_Sector(FLASH_SECTOR_11, VOLTAGE_RANGE_3); // 0x080E0000
  HAL_FLASH_Lock();
}
void DumpWrite16(uint32_t offset, uint16_t data) {
  HAL_FLASH_Unlock();
  HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, 0x080E0000 + offset, data);
  HAL_FLASH_Lock();
}
void DumpWrite32(uint32_t offset, uint32_t data) {
  HAL_FLASH_Unlock();
  HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, 0x080E0000 + offset, data);
  HAL_FLASH_Lock();
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
	DBG_INF_ON;
}

volatile int TimeoutFlag = 0;
volatile int LastITHalfComplete = 0;
volatile int32_t FullDataIndex = 0;  /* index of reception table */

static uint8_t aFull_Buffer[MAX_FRAME_SIZE] = {0};
static uint8_t aDST_Buffer[BUFFER_SIZE] = {0};

static void HalfTransferComplete(DMA_HandleTypeDef *DmaHandle) {
  int position = 0;
  LastITHalfComplete = 1;
  for (int i = 0; i < BUFFER_SIZE/2; i++) {
    aFull_Buffer[FullDataIndex] = aDST_Buffer[i+position];
    FullDataIndex++;
  }
}

static void TransferComplete(DMA_HandleTypeDef *DmaHandle) {
  int position = BUFFER_SIZE/2;
  LastITHalfComplete = 0;
  for (int i = 0; i < BUFFER_SIZE/2; i++) {
    aFull_Buffer[FullDataIndex] = aDST_Buffer[i+position];
    FullDataIndex++;
  }

  for(int i=0; i<0x100; i++){
    DumpWrite32(i*4, aDST_Buffer[i]);
  }
}

static void TransferError(DMA_HandleTypeDef *DmaHandle) {
  ErrorHandler();
}

static void TimeOut_Process(void) {
  
	uint32_t data_index = 0;
	uint32_t dma_cndtr;
	uint32_t index = 0;
	uint32_t full_frame_size;
	
  /* get remaining number of free space in BUFFER */
  //https://github.com/betaflight/betaflight/issues/8550
  //NOT SUPPORT: dma_cndtr = (uint16_t) (timHandle.hdma[TIM_DMA_ID_CC1]->Instance->CNDTR);
  dma_cndtr = (uint16_t) (timHandle.hdma[TIM_DMA_ID_CC1]->Instance->NDTR);

  if (LastITHalfComplete == 1) data_index = BUFFER_SIZE/2;

  while (data_index < BUFFER_SIZE - dma_cndtr) {
    aFull_Buffer[FullDataIndex] = aDST_Buffer[data_index];
    data_index++;
    FullDataIndex++;
  }

  full_frame_size = FullDataIndex - (NB_END_DATA_TO_DISCARD + NB_START_DATA_TO_DISCARD);
  //full_frame_size = FullDataIndex - NB_END_DATA_TO_DISCARD - NB_START_DATA_TO_DISCARD;

  /* In case need to discard some data at start of frame, we recompute table */
  if (NB_START_DATA_TO_DISCARD > 0) {
    for (index = 0; index < full_frame_size + NB_START_DATA_TO_DISCARD ; index++)
      aFull_Buffer[index] = aFull_Buffer[index + NB_START_DATA_TO_DISCARD];
  }

  if (full_frame_size == 0) while (1);
  if (full_frame_size > MAX_FRAME_SIZE) while (1);
}


int main(void) {

  HAL_Init();
  SystemClock_Init();
  GPIO_Init();
  TIM_Init();

  DumpClear();
	
  HAL_StatusTypeDef status;
  status = HAL_DMA_Start_IT(timHandle.hdma[TIM_DMA_ID_CC1], (GPIOE_BASE + 0x10 + 1), (uint32_t)&aDST_Buffer, BUFFER_SIZE);
  if(status != HAL_OK){
    ErrorHandler();
  }

  __HAL_TIM_ENABLE_DMA(&timHandle, TIM_DMA_CC1);
  __HAL_TIM_ENABLE_IT(&timHandle, TIM_IT_CC2);
  TIM_CCxChannelCmd(timHandle.Instance, TIM_CHANNEL_2, TIM_CCx_ENABLE);
  TIM_CCxChannelCmd(timHandle.Instance, TIM_CHANNEL_1, TIM_CCx_ENABLE);

  while (TimeoutFlag == 0);
  TimeOut_Process();

  DBG_WRN_ON;
  while(1);
}

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim) {
  TimeoutFlag = 1;
}