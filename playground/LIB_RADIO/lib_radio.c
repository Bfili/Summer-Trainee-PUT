#include "lib_radio.h"
#include "spi.h"
#include <stdint.h>
#include <string.h>

static uint8_t s_RadioConfigurationDataArray[] = RADIO_CONFIGURATION_DATA_ARRAY;
static uint8_t s_SendValue[16] = {0x00};
static uint16_t s_CheckSum = 0;
static uint8_t s_CommandsSent = 0;
static uint8_t* s_LenPointer;


void sendInstructions(){
	s_LenPointer = &s_RadioConfigurationDataArray[0];
	while(402 > s_CheckSum + s_CommandsSent){
		for(int j = 0;j < 16;j++){
			s_SendValue[j] = 0;
		}
		s_CommandsSent++;

		memcpy(&s_SendValue[0], &s_RadioConfigurationDataArray[s_CheckSum + s_CommandsSent], *s_LenPointer*sizeof(*s_LenPointer));

		HAL_GPIO_WritePin(CHIP_SELECT_GPIO_Port, CHIP_SELECT_Pin, GPIO_PIN_RESET);
		HAL_SPI_Transmit(&hspi3, s_SendValue, 16, 500);
		HAL_Delay(100);
		HAL_GPIO_WritePin(CHIP_SELECT_GPIO_Port, CHIP_SELECT_Pin, GPIO_PIN_SET);

		s_CheckSum = s_CheckSum + *s_LenPointer;
		s_LenPointer = &s_RadioConfigurationDataArray[s_CheckSum + s_CommandsSent];
	}
}
