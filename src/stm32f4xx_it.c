
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

#define GPIO_CS1 (GPIOC->IDR & 0b0001)
#define GPIO_RD (GPIOC->IDR & 0b0010)
#define GPIO_WR (GPIOC->IDR & 0b0100)
#define GPIO_CS2 (GPIOC->IDR & 0b1000)

#define GPIO_A00_A15 GPIOD->IDR
#define GPIO_A16_A23 (GPIOE->IDR & 0xFF)

#define GPIO_D00_D15_I GPIOF->IDR
#define GPIO_D00_D15_O GPIOF->ODR

// GBA BUS(NOT CART BUS) CART SEC
//rom ws0 0x08000000 0x09FFFFFF 0x01FFFFFF cs1
//rom ws1 0x0A000000 0x0BFFFFFF 0x01FFFFFF cs1
//rom ws2 0x0C000000 0x0DFFFFFF 0x01FFFFFF cs1
//pak ram 0x0E000000 0x0E00FFFF 0x0000FFFF cs2
#define CART_CS1 (!GPIO_CS1)
#define CART_RD (!GPIO_RD)
#define CART_WR (!GPIO_WR)
#define CART_CS2 (!GPIO_CS2)
#define CART_ADDR ((CART_A16_A23 << 17) | (CART_A00_A15 << 1))


#define DBG_OUT_ON GPIOG->ODR = 1
#define DBG_OUT_OFF GPIOG->ODR = 0

void EXTI0_IRQHandler(void) {
  if(GPIO_CS2 && CART_WR) DBG_OUT_ON;
  else DBG_OUT_OFF;
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}