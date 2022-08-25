/*!
* \file lib_radio.h
* \brief
* lib_radio.h provides functions definition for Si4463 radio module
* \par Membership:
* Hardware Abstraction
* \par Dialect:
* C99
* \par MCU type:
* STM32
*/

#ifndef RADIO_CONFIG_H_
#define RADIO_CONFIG_H_

#include "stm32l5xx.h"
#include "spi.h"

/** @addtogroup HAL_Si4463
* @{ \brief This is the Si4463 module
* \details \copydetails lib_radio.h
*/

/** @defgroup HAL_Si4463_IF External Interface
* @{
* \brief External interface of Si4463
*/


/** @defgroup Si4463_Exported_Functions Public Functions
* @{
*/

/**
 * \brief
 * Sends radio condiguration
 * \param SPIx: pointer to SPI handle
 * \param ChipSelectPort: pointer to GPIO Port of Chip Select
 * \param ChipSelectPin: GPIO pin of Chip Select
 *
 * \return Status of transmission
 */
HAL_StatusTypeDef sendConfigurationSettings(SPI_HandleTypeDef *SPIx, GPIO_TypeDef *ChipSelectPort,
                                            uint16_t ChipSelectPin);

/**
 * \brief
 * Sends provided message through radio
 * \param MessageFromUser: pointer to array containing message
 * \param MessageLength: length of array containing message
 *
 * \return Status of transmission
 */
HAL_StatusTypeDef sendMessage(uint8_t* MessageFromUser, uint8_t MessageLength);

/**
 * \brief
 * Gets all possible interrupt statuses
 *
 * Needs to be interpreted using Si4463 documentation.
 *
 * \param[out] IntResponse: Pointer to array where interrupt status will be saved
 * \return Status of transmission
 */
HAL_StatusTypeDef getRadioIntStatus(uint8_t* IntResponse);

/**
* @}
*/

/**
* @}
*/

/**
* @}
*/


#endif /* RADIO_CONFIG_H_ */
