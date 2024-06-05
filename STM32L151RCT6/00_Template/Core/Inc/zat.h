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

//��ʱ�����ʱ����㷽��:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=��ʱ������Ƶ��,��λ:Mhz
void TIM3_Init(uint16_t period, uint16_t prescaler);

//��ʱ���ײ�����������ʱ�ӣ������ж����ȼ�
//�˺����ᱻHAL_TIM_Base_Init()��������
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *time);

//��ʱ��3�жϷ�����
void TIM3_IRQHandler(void);

//�ص���������ʱ���жϷ���������
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *time);
//

void SystemClock_Config(void);

#endif 
