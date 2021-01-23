
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim);
void ErrorHandler(void);

#define DBG_INF_ON (GPIOF->ODR = (GPIOF->ODR | 0b001))
#define DBG_INF_OFF (GPIOF->ODR = (GPIOF->ODR & 0b110))
#define DBG_WRN_ON (GPIOF->ODR = (GPIOF->ODR | 0b010))
#define DBG_WRN_OFF (GPIOF->ODR = (GPIOF->ODR & 0b101))
#define DBG_ERR_ON (GPIOF->ODR = (GPIOF->ODR | 0b100))
#define DBG_ERR_OFF (GPIOF->ODR = (GPIOF->ODR & 0b011))
#define DBG_ERR_TRAP DBG_ERR_OFF;while(true);

#define DATA_FLASH_LEN (0x1000)

void DumpClear();
void DumpWrite16(uint32_t offset, uint16_t data);
void DumpWrite32(uint32_t offset, uint32_t data);

#ifdef __cplusplus
}
#endif

#endif


#define CLK_RANGE1

/**************************************************************************************/
#define DUMMY_START_DATA 4        /* 4 dummy data are sent at start to avoid timing issue on the first data sent */

#ifdef CLK_RANGE1
/* clock ratio [80000:28] [xx-2.86MHz]     (tried down to 1KHz)                       */
#define NB_START_DATA_TO_DISCARD  4
#define NB_END_DATA_TO_DISCARD    0
#define CLK_LATCHING_DATA_EDGE    TIM_ICPOLARITY_FALLING
#endif

#ifdef CLK_RANGE2
/* clock ratio [27:24] [2.96MHz-3.33MHz]                                           */
#define NB_START_DATA_TO_DISCARD  5
#define NB_END_DATA_TO_DISCARD    0
#define CLK_LATCHING_DATA_EDGE    TIM_ICPOLARITY_RISING
#endif

#ifdef CLK_RANGE3
/* clock ratio [23:12] [3.48MHz-6.67MHz]                                          */
#define NB_START_DATA_TO_DISCARD  5
#define NB_END_DATA_TO_DISCARD    1
#define CLK_LATCHING_DATA_EDGE    TIM_ICPOLARITY_RISING
#endif

#ifdef CLK_RANGE4
/* clock ratio [11:10] [7.27MHz-8MHz]                                            */
#define NB_START_DATA_TO_DISCARD  5
#define NB_END_DATA_TO_DISCARD    2
#define CLK_LATCHING_DATA_EDGE    TIM_ICPOLARITY_RISING
#endif

#ifdef CLK_RANGE5
/* clock ratio [9:8] [8.89MHz-10MHz]                                                              */
#define NB_START_DATA_TO_DISCARD  4
#define NB_END_DATA_TO_DISCARD    3
#define CLK_LATCHING_DATA_EDGE    TIM_ICPOLARITY_FALLING
#endif

/*******************************************************************************************/
/* the dma buffer size is smaller than full frame size                                     */
/* to see impact of CPU bus loading when reaching                                          */
/* dma buffer half complete and transfer complete state                                    */
#define BUFFER_SIZE           40000      /* buffer size for DMA (should be multiple of 2)  */
/*******************************************************************************************/
/* for large buffer size, the max frame size must be reduced to enter in the SRAM size     */
/* so the MAX_FRAME SIZE parameter must be reduced :                                       */
/*                  from 65535 to 55010 (with 40000 for buffer size) for  clock ratio 8    */
/*                  from 65535 to 60510 (with 32000 for buffer size) for  clock ratio 9    */
#define MAX_FRAME_SIZE    55010     /* MAX FRAME SIZE RECEIVED .. to reserve memory space  */
/*******************************************************************************************/