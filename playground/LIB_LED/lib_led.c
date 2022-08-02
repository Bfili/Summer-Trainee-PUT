#include "lib_led.h"
#include "tim.h"
#include <stdbool.h>

static LED_STEPS_E_T s_CurrentStep = STEP_NO_LED_E;
static _Bool s_TimerElapsed = false;
static _Bool s_Count = false;

void ledTask(void){
		switch(s_CurrentStep){
					  case STEP_NO_LED_E:
						  HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET);
						  HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_SET);
						  break;
					  case STEP_RED_LED_E:
						  HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET);
						  HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_SET);
						  break;
					  case STEP_GREEN_LED_E:
						  HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET);
						  HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);
						  break;
					  case STEP_BOTH_LED_E:
						  HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET);
						  HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);
						  break;
					  default:
						  /* do nothing */
						  break;
				  }
		if(s_Count){
			s_Count = false;
		}
}


void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin){
	if(GPIO_Pin == USER_BUTTON_Pin){
		s_CurrentStep = (s_CurrentStep + 1u) % STEP_MAX_E;
		s_Count = true;
	}
}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	  if(htim->Instance == TIM4){
		  s_CurrentStep = (s_CurrentStep + 1u) % STEP_MAX_E;
		  s_TimerElapsed = true;
	  }
}

