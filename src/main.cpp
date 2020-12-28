
#include "main.h"

TIM_HandleTypeDef htim4;
DMA_HandleTypeDef hdma_tim4_ch1;

void SystemClock_Config(void) {
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
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) Error_Handler();
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) Error_Handler();
}

static void TIM_Init(void){
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_IC_InitTypeDef sConfigIC = {0};
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 0;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 65535;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_IC_Init(&htim4) != HAL_OK) Error_Handler();
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK) Error_Handler();
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  if (HAL_TIM_IC_ConfigChannel(&htim4, &sConfigIC, TIM_CHANNEL_1) != HAL_OK) Error_Handler();
}

//DMA1_Stream0_BASE: 0x40026010
static void DMA_Init(void){
  __HAL_RCC_DMA1_CLK_ENABLE();
  HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);
}

static void GPIO_Init(void) {
  
  GPIO_InitTypeDef  GPIO_InitStructure;

  //GPIO RISING INPUT /RD:PB6 (TIM4_CH1 DMA1Str0)
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

void Error_Handler(void) {
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

uint16_t dmabuf[512];

int main(void) {

  for(int i=0; i<512; i++) dmabuf[i] = 0x69;

  HAL_Init();
  SystemClock_Config();
  GPIO_Init();
  DMA_Init();
  TIM_Init();

  DumpClear();

	
  HAL_StatusTypeDef status;
  status = HAL_DMA_Start_IT(&hdma_tim4_ch1, (uint32_t)&GPIO_DATA_I, (uint32_t)dmabuf, 1);
  if(status != HAL_OK){
    DBG_ERR_TRAP;
  }
  __HAL_TIM_ENABLE_DMA(&htim4, TIM_DMA_UPDATE);
  __HAL_TIM_ENABLE(&htim4);
  DBG_WRN_ON;
  while(1);





  uint32_t ad_lo[4] = {0};
  uint32_t ad_hi[4] = {0};
  uint32_t  data[4] = {0};
  int p = 0;

  uint32_t buffer[0x100] ={0};
  while(p<0x100){
    uint16_t gpio_ad_lo = GPIOD->IDR;
    uint16_t gpio_ct = GPIOC->IDR;
    if(IS_RD_LO(gpio_ct)){
      buffer[p] = GPIOD->IDR;
      p++;
    }
  }

  for(int i=0; i<0x100; i++){
    DumpWrite32(i*4, buffer[i]);
  }
  //uint16_t old_ct = INIT_PATTERN;
  //while(p < 4){
    //uint16_t gpio_ct = GPIOC->IDR;
    //asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");asm("NOP");
    //uint16_t gpio_ad_lo = GPIOD->IDR;
    //uint16_t gpio_ad_hi = GPIOE->IDR;
    //if(IS_PATTERN_IN_SEQ(gpio_ct)){
    //  if(IS_OLD_CS1_HI(old_ct)){
    //    ad_lo[p] = gpio_ad_lo;
    //    ad_hi[p] = gpio_ad_hi;
    //    p++;
    //  }
    //}
    //old_ct = gpio_ct;
  //}

  //DumpWrite32(0x00, ad_hi[0]); DumpWrite32(0x10, ad_lo[0]); DumpWrite32(0x20, ADDR(ad_hi[0], ad_lo[0]));
  //DumpWrite32(0x04, ad_hi[1]); DumpWrite32(0x14, ad_lo[1]); DumpWrite32(0x24, ADDR(ad_hi[1], ad_lo[1]));
  //DumpWrite32(0x08, ad_hi[2]); DumpWrite32(0x18, ad_lo[2]); DumpWrite32(0x28, ADDR(ad_hi[2], ad_lo[2]));
  //DumpWrite32(0x0C, ad_hi[3]); DumpWrite32(0x1C, ad_lo[3]); DumpWrite32(0x2C, ADDR(ad_hi[3], ad_lo[3]));
  
  while(1);
}