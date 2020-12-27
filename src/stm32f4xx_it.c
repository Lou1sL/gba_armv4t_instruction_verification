
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

// /CS1 falling edge (rom access)
void EXTI0_IRQHandler(void) {
  //current_addr_lo = GPIOD->IDR;
  //current_addr_hi = GPIOE->IDR;
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}

// /RD falling edge
void EXTI1_IRQHandler(void) {
  //asm("NOP");asm("NOP");asm("NOP");asm("NOP");
  //uint32_t d = GPIOD->IDR;
  //if(CART_CS1 && (current_addr_lo != 0)){
  //  uint32_t addr = (((current_addr_hi & 0xFF) << 17) | (current_addr_lo << 1) | 0);
  //  Write_Dump_32(0x00, addr);
  //  Write_Dump_16(0x10, d);
  //  DBG_OUT_ON;
  //  while(1);
  //}
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
}

// /WR falling edge
void EXTI2_IRQHandler(void) {
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
}

// /CS2 falling edge (ram access)
void EXTI3_IRQHandler(void) {
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);
}

// /RD rising edge
void EXTI4_IRQHandler(void) {
  //if(CART_CS1) current_addr_lo++;
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_4);
}