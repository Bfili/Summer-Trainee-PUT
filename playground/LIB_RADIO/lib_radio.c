#include "lib_radio.h"
#include "spi.h"
#include <stdint.h>
#include <string.h>

#define SEND_VALUE_ARRAY_SIZE 16
#define RADIO_CONFIGURATION_DATA_ARRAY_SIZE 402

static const uint8_t s_RadioConfigurationDataArray_C[] = RADIO_CONFIGURATION_DATA_ARRAY;
static uint8_t s_SendValue[SEND_VALUE_ARRAY_SIZE] = {0x00u};
static uint16_t s_CurrentCommandLengthIndex = 0u;
static uint8_t s_CommandsSent = 0u;
static uint8_t* s_LenPointer;
static HAL_StatusTypeDef s_Status;


void sendInstructions(void){
	s_LenPointer = &s_RadioConfigurationDataArray_C[0];
	while((uint8_t)RADIO_CONFIGURATION_DATA_ARRAY_SIZE > s_CurrentCommandLengthIndex + s_CommandsSent){

		s_CommandsSent++;

		memcpy(&s_SendValue[0], &s_RadioConfigurationDataArray_C[s_CurrentCommandLengthIndex + s_CommandsSent], *s_LenPointer*sizeof(*s_LenPointer));

		HAL_GPIO_WritePin(CHIP_SELECT_GPIO_Port, CHIP_SELECT_Pin, GPIO_PIN_RESET);
		s_Status = HAL_SPI_Transmit(&hspi3, s_SendValue, *s_LenPointer, 500u);
		HAL_GPIO_WritePin(CHIP_SELECT_GPIO_Port, CHIP_SELECT_Pin, GPIO_PIN_SET);
		HAL_Delay(100u);

		s_CurrentCommandLengthIndex = s_CurrentCommandLengthIndex + *s_LenPointer;
		s_LenPointer = &s_RadioConfigurationDataArray_C[s_CurrentCommandLengthIndex + s_CommandsSent];
		if(HAL_ERROR == s_Status){
			break;
		}
	}
}
