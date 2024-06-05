#include "main.h"

//
#ifndef __ZAT_
#define __ZAT_

void LED_Init(void);

void led_on(void);

void led_off(void);

void led_flash(void);

void led_pwm(void);

void EXTI0_IRQHandler(void);

void EXTI9_5_IRQHandler(void);

void EXTI15_10_IRQHandler(void);

void GPIO_EXTI_Init(void);

//定时器溢出时间计算方法:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=定时器工作频率,单位:Mhz
void TIM3_Init(uint16_t period, uint16_t prescaler);

//定时器底册驱动，开启时钟，设置中断优先级
//此函数会被HAL_TIM_Base_Init()函数调用
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *time);

//定时器3中断服务函数
void TIM3_IRQHandler(void);

//回调函数，定时器中断服务函数调用
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *time);
//

void SystemClock_Config(void);

#endif 
