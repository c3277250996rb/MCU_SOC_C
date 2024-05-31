//#include "stm32l1xx_hal_tim.h"

#include "stm32l1xx_hal.h"

void led_init(void);
void TIM3_Init(uint16_t arr,uint16_t psc);
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim);

//定时器3中断服务函数
void TIM3_IRQHandler(void);

//回调函数，定时器中断服务函数调用
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
