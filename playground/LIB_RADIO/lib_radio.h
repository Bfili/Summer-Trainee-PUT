/*! @file radio_config.h
 * @brief This file contains the automatically generated
 * configurations.
 *
 * @n WDS GUI Version: 3.2.11.0
 * @n Device: Si4463 Rev.: C2                                 
 *
 * @b COPYRIGHT
 * @n Silicon Laboratories Confidential
 * @n Copyright 2017 Silicon Laboratories, Inc.
 * @n http://www.silabs.com
 */

#ifndef RADIO_CONFIG_H_
#define RADIO_CONFIG_H_

#include "stm32l5xx.h"
#include "spi.h"

typedef struct
{
    SPI_HandleTypeDef *SPI_Handle;
    GPIO_TypeDef *ChipSelectGPIOPort;
    uint16_t ChipSelectGPIOPin;
}SI4463_HANDLER_S_T;

HAL_StatusTypeDef sendConfigurationSettings(SPI_HandleTypeDef *SPIx, GPIO_TypeDef *ChipSelectPort,
                                            uint16_t ChipSelectPin, SI4463_HANDLER_S_T *TypeDefStruct);

HAL_StatusTypeDef sendMessage(uint8_t* MessageFromUser, uint8_t MessageLength, SI4463_HANDLER_S_T *TypeDefStruct);

#endif /* RADIO_CONFIG_H_ */
