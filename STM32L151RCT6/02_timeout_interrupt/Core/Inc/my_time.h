//#include "stm32l1xx_hal_tim.h"

#include "stm32l1xx_hal.h"

void led_init(void);
void TIM3_Init(uint16_t arr,uint16_t psc);
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim);

//��ʱ��3�жϷ�����
void TIM3_IRQHandler(void);

//�ص���������ʱ���жϷ���������
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
