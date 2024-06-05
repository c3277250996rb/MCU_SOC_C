#include "zat.h"
#include "main.h"


int main(void)
{

  HAL_Init();

  SystemClock_Config();

	LED_Init();
//	led_pwm();
	TIM3_Init(20 - 1, 7200 - 1);
	
	GPIO_EXTI_Init();
	
  while (1)
  {
		HAL_Delay(100);
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_8);
  }
}



