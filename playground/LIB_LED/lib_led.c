#include "lib_led.h"
#include "tim.h"

static LED_STEPS_E_T s_CurrentStep = STEP_NO_LED_E;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	  if(htim->Instance == TIM4){
		  	if(STEP_NO_LED_E == s_CurrentStep%STEP_MAX_E){
		  	  	HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET);
		  	  	HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_SET);
		  	}
		  	else if(STEP_RED_LED_E == s_CurrentStep%STEP_MAX_E){
		  		HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET);
		  		HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_SET);
		  	}
	  		else if(STEP_GREEN_LED_E == s_CurrentStep%STEP_MAX_E){
	  			HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET);
	  			HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);
	  		}
	  		else if(STEP_BOTH_LED_E == s_CurrentStep%STEP_MAX_E){
	  			HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET);
	  			HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);
	  		}
  			s_CurrentStep = (s_CurrentStep + 1u) % STEP_MAX_E;
	  }
}

