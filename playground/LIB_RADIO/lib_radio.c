/*!
* \file lib_radio.c
* \brief
* lib_radio.c implements Si4463 radio functionality for STM32L5
* \par Membership:
* Hardware Abstraction
* \par Dialect:
* C99
* \par MCU type:
* STM32
*/

/* Includes **********************************************************/

#include "lib_radio.h"
#include "spi.h"
#include <stdint.h>
#include <string.h>
#include "stm32l5xx_hal_def.h"

// USER DEFINED PARAMETERS
// Define your own parameters here

// INPUT DATA
/*
// Crys_freq(Hz): 26000000    Crys_tol(ppm): 20    IF_mode: 2    High_perf_Ch_Fil: 1    OSRtune: 0    Ch_Fil_Bw_AFC: 0    ANT_DIV: 0    PM_pattern: 0
// MOD_type: 3    Rsymb(sps): 76800    Fdev(Hz): 32000    RXBW(Hz): 150000    Manchester: 0    AFC_en: 0    Rsymb_error: 0.0    Chip-Version: 2
// RF Freq.(MHz): 869.85    API_TC: 29    fhst: 190000    inputBW: 0    BERT: 0    RAW_dout: 0    D_source: 0    Hi_pfm_div: 1
// API_ARR_Det_en: 0    Fdev_error: 0    API_ETSI: 0
//
// # RX IF frequency is  -406250 Hz
// # WB filter 4 (BW = 143.24 kHz);  NB-filter 4 (BW = 143.24 kHz)
//
// Modulation index: 0,833
*/


// CONFIGURATION PARAMETERS
#define RADIO_CONFIGURATION_DATA_RADIO_XO_FREQ                     26000000L
#define RADIO_CONFIGURATION_DATA_CHANNEL_NUMBER                    0x00
#define RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH               0x12
#define RADIO_CONFIGURATION_DATA_RADIO_STATE_AFTER_POWER_UP        0x03
#define RADIO_CONFIGURATION_DATA_RADIO_DELAY_CNT_AFTER_RESET       0xF000


// CONFIGURATION COMMANDS

/*
// Command:                  RF_POWER_UP
// Description:              Command to power-up the device and select the operational mode and functionality.
*/
#define RF_POWER_UP 0x02, 0x01, 0x01, 0x01, 0x8C, 0xBA, 0x80

/*
// Command:                  RF_GPIO_PIN_CFG
// Description:              Configures the GPIO pins.
*/
#define RF_GPIO_PIN_CFG 0x13, 0x21, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00

/*
// Set properties:           RF_GLOBAL_XO_TUNE_2
// Number of properties:     2
// Group ID:                 0x00
// Start ID:                 0x00
// Default values:           0x40, 0x00,
// Descriptions:
//   GLOBAL_XO_TUNE - Configure the internal capacitor frequency tuning bank for the crystal oscillator.
//   GLOBAL_CLK_CFG - Clock configuration options.
*/
#define RF_GLOBAL_XO_TUNE_2 0x11, 0x00, 0x02, 0x00, 0x00, 0x00

/*
// Set properties:           RF_GLOBAL_CONFIG_1
// Number of properties:     1
// Group ID:                 0x00
// Start ID:                 0x03
// Default values:           0x20,
// Descriptions:
//   GLOBAL_CONFIG - Global configuration settings.
*/
#define RF_GLOBAL_CONFIG_1 0x11, 0x00, 0x01, 0x03, 0x20

/*
// Set properties:           RF_INT_CTL_ENABLE_2
// Number of properties:     2
// Group ID:                 0x01
// Start ID:                 0x00
// Default values:           0x04, 0x00,
// Descriptions:
//   INT_CTL_ENABLE - This property provides for global enabling of the three interrupt groups (Chip, Modem and Packet Handler) in order to generate HW interrupts at the NIRQ pin.
//   INT_CTL_PH_ENABLE - Enable individual interrupt sources within the Packet Handler Interrupt Group to generate a HW interrupt on the NIRQ output pin.
*/
#define RF_INT_CTL_ENABLE_2 0x11, 0x01, 0x02, 0x00, 0x01, 0x20

/*
// Set properties:           RF_FRR_CTL_A_MODE_4
// Number of properties:     4
// Group ID:                 0x02
// Start ID:                 0x00
// Default values:           0x01, 0x02, 0x09, 0x00,
// Descriptions:
//   FRR_CTL_A_MODE - Fast Response Register A Configuration.
//   FRR_CTL_B_MODE - Fast Response Register B Configuration.
//   FRR_CTL_C_MODE - Fast Response Register C Configuration.
//   FRR_CTL_D_MODE - Fast Response Register D Configuration.
*/
#define RF_FRR_CTL_A_MODE_4 0x11, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00

/*
// Set properties:           RF_PREAMBLE_TX_LENGTH_9
// Number of properties:     9
// Group ID:                 0x10
// Start ID:                 0x00
// Default values:           0x08, 0x14, 0x00, 0x0F, 0x21, 0x00, 0x00, 0x00, 0x00,
// Descriptions:
//   PREAMBLE_TX_LENGTH - Configure length of TX Preamble.
//   PREAMBLE_CONFIG_STD_1 - Configuration of reception of a packet with a Standard Preamble pattern.
//   PREAMBLE_CONFIG_NSTD - Configuration of transmission/reception of a packet with a Non-Standard Preamble pattern.
//   PREAMBLE_CONFIG_STD_2 - Configuration of timeout periods during reception of a packet with Standard Preamble pattern.
//   PREAMBLE_CONFIG - General configuration bits for the Preamble field.
//   PREAMBLE_PATTERN_31_24 - Configuration of the bit values describing a Non-Standard Preamble pattern.
//   PREAMBLE_PATTERN_23_16 - Configuration of the bit values describing a Non-Standard Preamble pattern.
//   PREAMBLE_PATTERN_15_8 - Configuration of the bit values describing a Non-Standard Preamble pattern.
//   PREAMBLE_PATTERN_7_0 - Configuration of the bit values describing a Non-Standard Preamble pattern.
*/
#define RF_PREAMBLE_TX_LENGTH_9 0x11, 0x10, 0x09, 0x00, 0x08, 0x14, 0x00, 0x0F, 0x31, 0x00, 0x00, 0x00, 0x00

/*
// Set properties:           RF_SYNC_CONFIG_6
// Number of properties:     6
// Group ID:                 0x11
// Start ID:                 0x00
// Default values:           0x01, 0x2D, 0xD4, 0x2D, 0xD4, 0x00,
// Descriptions:
//   SYNC_CONFIG - Sync Word configuration bits.
//   SYNC_BITS_31_24 - Sync word.
//   SYNC_BITS_23_16 - Sync word.
//   SYNC_BITS_15_8 - Sync word.
//   SYNC_BITS_7_0 - Sync word.
//   SYNC_CONFIG2 - Sync Word configuration bits.
*/
#define RF_SYNC_CONFIG_6 0x11, 0x11, 0x06, 0x00, 0x23, 0xCB, 0x89, 0xCB, 0x89, 0x00

/*
// Set properties:           RF_PKT_CRC_CONFIG_12
// Number of properties:     12
// Group ID:                 0x12
// Start ID:                 0x00
// Default values:           0x00, 0x01, 0x08, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30,
// Descriptions:
//   PKT_CRC_CONFIG - Select a CRC polynomial and seed.
//   PKT_WHT_POLY_15_8 - 16-bit polynomial value for the PN Generator (e.g., for Data Whitening)
//   PKT_WHT_POLY_7_0 - 16-bit polynomial value for the PN Generator (e.g., for Data Whitening)
//   PKT_WHT_SEED_15_8 - 16-bit seed value for the PN Generator (e.g., for Data Whitening)
//   PKT_WHT_SEED_7_0 - 16-bit seed value for the PN Generator (e.g., for Data Whitening)
//   PKT_WHT_BIT_NUM - Selects which bit of the LFSR (used to generate the PN / data whitening sequence) is used as the output bit for data scrambling.
//   PKT_CONFIG1 - General configuration bits for transmission or reception of a packet.
//   PKT_CONFIG2 - General packet configuration bits.
//   PKT_LEN - Configuration bits for reception of a variable length packet.
//   PKT_LEN_FIELD_SOURCE - Field number containing the received packet length byte(s).
//   PKT_LEN_ADJUST - Provides for adjustment/offset of the received packet length value (in order to accommodate a variety of methods of defining total packet length).
//   PKT_TX_THRESHOLD - TX FIFO almost empty threshold.
*/
#define RF_PKT_CRC_CONFIG_12 0x11, 0x12, 0x0C, 0x00, 0x04, 0x01, 0x08, 0xFF, 0xFF, 0x20, 0x02, 0x00, 0x00, 0x00, 0x00, 0x30

/*
// Set properties:           RF_PKT_RX_THRESHOLD_12
// Number of properties:     12
// Group ID:                 0x12
// Start ID:                 0x0C
// Default values:           0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
// Descriptions:
//   PKT_RX_THRESHOLD - RX FIFO Almost Full threshold.
//   PKT_FIELD_1_LENGTH_12_8 - Unsigned 13-bit Field 1 length value.
//   PKT_FIELD_1_LENGTH_7_0 - Unsigned 13-bit Field 1 length value.
//   PKT_FIELD_1_CONFIG - General data processing and packet configuration bits for Field 1.
//   PKT_FIELD_1_CRC_CONFIG - Configuration of CRC control bits across Field 1.
//   PKT_FIELD_2_LENGTH_12_8 - Unsigned 13-bit Field 2 length value.
//   PKT_FIELD_2_LENGTH_7_0 - Unsigned 13-bit Field 2 length value.
//   PKT_FIELD_2_CONFIG - General data processing and packet configuration bits for Field 2.
//   PKT_FIELD_2_CRC_CONFIG - Configuration of CRC control bits across Field 2.
//   PKT_FIELD_3_LENGTH_12_8 - Unsigned 13-bit Field 3 length value.
//   PKT_FIELD_3_LENGTH_7_0 - Unsigned 13-bit Field 3 length value.
//   PKT_FIELD_3_CONFIG - General data processing and packet configuration bits for Field 3.
*/
#define RF_PKT_RX_THRESHOLD_12 0x11, 0x12, 0x0C, 0x0C, 0x30, 0x00, 0x01, 0x00, 0x82, 0x00, 0x11, 0x00, 0x22, 0x00, 0x00, 0x00

/*
// Set properties:           RF_PKT_FIELD_3_CRC_CONFIG_12
// Number of properties:     12
// Group ID:                 0x12
// Start ID:                 0x18
// Default values:           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
// Descriptions:
//   PKT_FIELD_3_CRC_CONFIG - Configuration of CRC control bits across Field 3.
//   PKT_FIELD_4_LENGTH_12_8 - Unsigned 13-bit Field 4 length value.
//   PKT_FIELD_4_LENGTH_7_0 - Unsigned 13-bit Field 4 length value.
//   PKT_FIELD_4_CONFIG - General data processing and packet configuration bits for Field 4.
//   PKT_FIELD_4_CRC_CONFIG - Configuration of CRC control bits across Field 4.
//   PKT_FIELD_5_LENGTH_12_8 - Unsigned 13-bit Field 5 length value.
//   PKT_FIELD_5_LENGTH_7_0 - Unsigned 13-bit Field 5 length value.
//   PKT_FIELD_5_CONFIG - General data processing and packet configuration bits for Field 5.
//   PKT_FIELD_5_CRC_CONFIG - Configuration of CRC control bits across Field 5.
//   PKT_RX_FIELD_1_LENGTH_12_8 - Unsigned 13-bit RX Field 1 length value.
//   PKT_RX_FIELD_1_LENGTH_7_0 - Unsigned 13-bit RX Field 1 length value.
//   PKT_RX_FIELD_1_CONFIG - General data processing and packet configuration bits for RX Field 1.
*/
#define RF_PKT_FIELD_3_CRC_CONFIG_12 0x11, 0x12, 0x0C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

/*
// Set properties:           RF_PKT_RX_FIELD_1_CRC_CONFIG_12
// Number of properties:     12
// Group ID:                 0x12
// Start ID:                 0x24
// Default values:           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
// Descriptions:
//   PKT_RX_FIELD_1_CRC_CONFIG - Configuration of CRC control bits across RX Field 1.
//   PKT_RX_FIELD_2_LENGTH_12_8 - Unsigned 13-bit RX Field 2 length value.
//   PKT_RX_FIELD_2_LENGTH_7_0 - Unsigned 13-bit RX Field 2 length value.
//   PKT_RX_FIELD_2_CONFIG - General data processing and packet configuration bits for RX Field 2.
//   PKT_RX_FIELD_2_CRC_CONFIG - Configuration of CRC control bits across RX Field 2.
//   PKT_RX_FIELD_3_LENGTH_12_8 - Unsigned 13-bit RX Field 3 length value.
//   PKT_RX_FIELD_3_LENGTH_7_0 - Unsigned 13-bit RX Field 3 length value.
//   PKT_RX_FIELD_3_CONFIG - General data processing and packet configuration bits for RX Field 3.
//   PKT_RX_FIELD_3_CRC_CONFIG - Configuration of CRC control bits across RX Field 3.
//   PKT_RX_FIELD_4_LENGTH_12_8 - Unsigned 13-bit RX Field 4 length value.
//   PKT_RX_FIELD_4_LENGTH_7_0 - Unsigned 13-bit RX Field 4 length value.
//   PKT_RX_FIELD_4_CONFIG - General data processing and packet configuration bits for RX Field 4.
*/
#define RF_PKT_RX_FIELD_1_CRC_CONFIG_12 0x11, 0x12, 0x0C, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

/*
// Set properties:           RF_PKT_RX_FIELD_4_CRC_CONFIG_5
// Number of properties:     5
// Group ID:                 0x12
// Start ID:                 0x30
// Default values:           0x00, 0x00, 0x00, 0x00, 0x00,
// Descriptions:
//   PKT_RX_FIELD_4_CRC_CONFIG - Configuration of CRC control bits across RX Field 4.
//   PKT_RX_FIELD_5_LENGTH_12_8 - Unsigned 13-bit RX Field 5 length value.
//   PKT_RX_FIELD_5_LENGTH_7_0 - Unsigned 13-bit RX Field 5 length value.
//   PKT_RX_FIELD_5_CONFIG - General data processing and packet configuration bits for RX Field 5.
//   PKT_RX_FIELD_5_CRC_CONFIG - Configuration of CRC control bits across RX Field 5.
*/
#define RF_PKT_RX_FIELD_4_CRC_CONFIG_5 0x11, 0x12, 0x05, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00

/*
// Set properties:           RF_PKT_CRC_SEED_31_24_4
// Number of properties:     4
// Group ID:                 0x12
// Start ID:                 0x36
// Default values:           0x00, 0x00, 0x00, 0x00,
// Descriptions:
//   PKT_CRC_SEED_31_24 - 32-bit seed value for the 32-bit CRC engine
//   PKT_CRC_SEED_23_16 - 32-bit seed value for the 32-bit CRC engine
//   PKT_CRC_SEED_15_8 - 32-bit seed value for the 32-bit CRC engine
//   PKT_CRC_SEED_7_0 - 32-bit seed value for the 32-bit CRC engine
*/
#define RF_PKT_CRC_SEED_31_24_4 0x11, 0x12, 0x04, 0x36, 0xFF, 0xFF, 0xFF, 0xFF

/*
// Set properties:           RF_MODEM_MOD_TYPE_12
// Number of properties:     12
// Group ID:                 0x20
// Start ID:                 0x00
// Default values:           0x02, 0x80, 0x07, 0x0F, 0x42, 0x40, 0x01, 0xC9, 0xC3, 0x80, 0x00, 0x06,
// Descriptions:
//   MODEM_MOD_TYPE - Selects the type of modulation. In TX mode, additionally selects the source of the modulation.
//   MODEM_MAP_CONTROL - Controls polarity and mapping of transmit and receive bits.
//   MODEM_DSM_CTRL - Miscellaneous control bits for the Delta-Sigma Modulator (DSM) in the PLL Synthesizer.
//   MODEM_DATA_RATE_2 - Unsigned 24-bit value used to determine the TX data rate
//   MODEM_DATA_RATE_1 - Unsigned 24-bit value used to determine the TX data rate
//   MODEM_DATA_RATE_0 - Unsigned 24-bit value used to determine the TX data rate
//   MODEM_TX_NCO_MODE_3 - TX Gaussian filter oversampling ratio and Byte 3 of unsigned 26-bit TX Numerically Controlled Oscillator (NCO) modulus.
//   MODEM_TX_NCO_MODE_2 - TX Gaussian filter oversampling ratio and Byte 3 of unsigned 26-bit TX Numerically Controlled Oscillator (NCO) modulus.
//   MODEM_TX_NCO_MODE_1 - TX Gaussian filter oversampling ratio and Byte 3 of unsigned 26-bit TX Numerically Controlled Oscillator (NCO) modulus.
//   MODEM_TX_NCO_MODE_0 - TX Gaussian filter oversampling ratio and Byte 3 of unsigned 26-bit TX Numerically Controlled Oscillator (NCO) modulus.
//   MODEM_FREQ_DEV_2 - 17-bit unsigned TX frequency deviation word.
//   MODEM_FREQ_DEV_1 - 17-bit unsigned TX frequency deviation word.
*/
#define RF_MODEM_MOD_TYPE_12 0x11, 0x20, 0x0C, 0x00, 0x03, 0x00, 0x07, 0x17, 0x70, 0x00, 0x09, 0x8C, 0xBA, 0x80, 0x00, 0x05

/*
// Set properties:           RF_MODEM_FREQ_DEV_0_1
// Number of properties:     1
// Group ID:                 0x20
// Start ID:                 0x0C
// Default values:           0xD3,
// Descriptions:
//   MODEM_FREQ_DEV_0 - 17-bit unsigned TX frequency deviation word.
*/
#define RF_MODEM_FREQ_DEV_0_1 0x11, 0x20, 0x01, 0x0C, 0x0B

/*
// Set properties:           RF_MODEM_TX_RAMP_DELAY_12
// Number of properties:     12
// Group ID:                 0x20
// Start ID:                 0x18
// Default values:           0x01, 0x00, 0x08, 0x03, 0xC0, 0x00, 0x10, 0x20, 0x00, 0x00, 0x00, 0x4B,
// Descriptions:
//   MODEM_TX_RAMP_DELAY - TX ramp-down delay setting.
//   MODEM_MDM_CTRL - MDM control.
//   MODEM_IF_CONTROL - Selects Fixed-IF, Scaled-IF, or Zero-IF mode of RX Modem operation.
//   MODEM_IF_FREQ_2 - the IF frequency setting (an 18-bit signed number).
//   MODEM_IF_FREQ_1 - the IF frequency setting (an 18-bit signed number).
//   MODEM_IF_FREQ_0 - the IF frequency setting (an 18-bit signed number).
//   MODEM_DECIMATION_CFG1 - Specifies three decimator ratios for the Cascaded Integrator Comb (CIC) filter.
//   MODEM_DECIMATION_CFG0 - Specifies miscellaneous parameters and decimator ratios for the Cascaded Integrator Comb (CIC) filter.
//   MODEM_DECIMATION_CFG2 - Specifies miscellaneous decimator filter selections.
//   MODEM_IFPKD_THRESHOLDS -
//   MODEM_BCR_OSR_1 - RX BCR/Slicer oversampling rate (12-bit unsigned number).
//   MODEM_BCR_OSR_0 - RX BCR/Slicer oversampling rate (12-bit unsigned number).
*/
#define RF_MODEM_TX_RAMP_DELAY_12 0x11, 0x20, 0x0C, 0x18, 0x01, 0x00, 0x08, 0x03, 0xC0, 0x00, 0x10, 0x20, 0x00, 0xE8, 0x00, 0x55

/*
// Set properties:           RF_MODEM_BCR_NCO_OFFSET_2_12
// Number of properties:     12
// Group ID:                 0x20
// Start ID:                 0x24
// Default values:           0x06, 0xD3, 0xA0, 0x06, 0xD3, 0x02, 0xC0, 0x00, 0x00, 0x23, 0x83, 0x69,
// Descriptions:
//   MODEM_BCR_NCO_OFFSET_2 - RX BCR NCO offset value (an unsigned 22-bit number).
//   MODEM_BCR_NCO_OFFSET_1 - RX BCR NCO offset value (an unsigned 22-bit number).
//   MODEM_BCR_NCO_OFFSET_0 - RX BCR NCO offset value (an unsigned 22-bit number).
//   MODEM_BCR_GAIN_1 - The unsigned 11-bit RX BCR loop gain value.
//   MODEM_BCR_GAIN_0 - The unsigned 11-bit RX BCR loop gain value.
//   MODEM_BCR_GEAR - RX BCR loop gear control.
//   MODEM_BCR_MISC1 - Miscellaneous control bits for the RX BCR loop.
//   MODEM_BCR_MISC0 - Miscellaneous RX BCR loop controls.
//   MODEM_AFC_GEAR - RX AFC loop gear control.
//   MODEM_AFC_WAIT - RX AFC loop wait time control.
//   MODEM_AFC_GAIN_1 - Sets the gain of the PLL-based AFC acquisition loop, and provides miscellaneous control bits for AFC functionality.
//   MODEM_AFC_GAIN_0 - Sets the gain of the PLL-based AFC acquisition loop, and provides miscellaneous control bits for AFC functionality.
*/
#define RF_MODEM_BCR_NCO_OFFSET_2_12 0x11, 0x20, 0x0C, 0x24, 0x06, 0x0C, 0xAB, 0x07, 0x3A, 0x02, 0x00, 0x00, 0x00, 0x12, 0x81, 0x83

/*
// Set properties:           RF_MODEM_AFC_LIMITER_1_3
// Number of properties:     3
// Group ID:                 0x20
// Start ID:                 0x30
// Default values:           0x00, 0x40, 0xA0,
// Descriptions:
//   MODEM_AFC_LIMITER_1 - Set the AFC limiter value.
//   MODEM_AFC_LIMITER_0 - Set the AFC limiter value.
//   MODEM_AFC_MISC - Specifies miscellaneous AFC control bits.
*/
#define RF_MODEM_AFC_LIMITER_1_3 0x11, 0x20, 0x03, 0x30, 0x01, 0x9E, 0xA0

/*
// Set properties:           RF_MODEM_AGC_CONTROL_1
// Number of properties:     1
// Group ID:                 0x20
// Start ID:                 0x35
// Default values:           0xE0,
// Descriptions:
//   MODEM_AGC_CONTROL - Miscellaneous control bits for the Automatic Gain Control (AGC) function in the RX Chain.
*/
#define RF_MODEM_AGC_CONTROL_1 0x11, 0x20, 0x01, 0x35, 0xE0

/*
// Set properties:           RF_MODEM_AGC_WINDOW_SIZE_12
// Number of properties:     12
// Group ID:                 0x20
// Start ID:                 0x38
// Default values:           0x11, 0x10, 0x10, 0x0B, 0x1C, 0x40, 0x00, 0x00, 0x2B, 0x0C, 0xA4, 0x03,
// Descriptions:
//   MODEM_AGC_WINDOW_SIZE - Specifies the size of the measurement and settling windows for the AGC algorithm.
//   MODEM_AGC_RFPD_DECAY - Sets the decay time of the RF peak detectors.
//   MODEM_AGC_IFPD_DECAY - Sets the decay time of the IF peak detectors.
//   MODEM_FSK4_GAIN1 - Specifies the gain factor of the secondary branch in 4(G)FSK ISI-suppression.
//   MODEM_FSK4_GAIN0 - Specifies the gain factor of the primary branch in 4(G)FSK ISI-suppression.
//   MODEM_FSK4_TH1 - 16 bit 4(G)FSK slicer threshold.
//   MODEM_FSK4_TH0 - 16 bit 4(G)FSK slicer threshold.
//   MODEM_FSK4_MAP - 4(G)FSK symbol mapping code.
//   MODEM_OOK_PDTC - Configures the attack and decay times of the OOK Peak Detector.
//   MODEM_OOK_BLOPK - Configures the slicing reference level of the OOK Peak Detector.
//   MODEM_OOK_CNT1 - OOK control.
//   MODEM_OOK_MISC - Selects the detector(s) used for demodulation of an OOK signal, or for demodulation of a (G)FSK signal when using the asynchronous demodulator.
*/
#define RF_MODEM_AGC_WINDOW_SIZE_12 0x11, 0x20, 0x0C, 0x38, 0x11, 0x13, 0x13, 0x80, 0x1A, 0x35, 0x55, 0x00, 0x28, 0x0C, 0xA4, 0x23

/*
// Set properties:           RF_MODEM_RAW_CONTROL_5
// Number of properties:     5
// Group ID:                 0x20
// Start ID:                 0x45
// Default values:           0x02, 0x00, 0xA3, 0x02, 0x80,
// Descriptions:
//   MODEM_RAW_CONTROL - Defines gain and enable controls for raw / nonstandard mode.
//   MODEM_RAW_EYE_1 - 11 bit eye-open detector threshold.
//   MODEM_RAW_EYE_0 - 11 bit eye-open detector threshold.
//   MODEM_ANT_DIV_MODE - Antenna diversity mode settings.
//   MODEM_ANT_DIV_CONTROL - Specifies controls for the Antenna Diversity algorithm.
*/
#define RF_MODEM_RAW_CONTROL_5 0x11, 0x20, 0x05, 0x45, 0x03, 0x00, 0xCC, 0x01, 0x00

/*
// Set properties:           RF_MODEM_RSSI_JUMP_THRESH_1
// Number of properties:     1
// Group ID:                 0x20
// Start ID:                 0x4B
// Default values:           0x0C,
// Descriptions:
//   MODEM_RSSI_JUMP_THRESH - Configures the RSSI Jump Detection threshold.
*/
#define RF_MODEM_RSSI_JUMP_THRESH_1 0x11, 0x20, 0x01, 0x4B, 0x06

/*
// Set properties:           RF_MODEM_RSSI_CONTROL2_2
// Number of properties:     2
// Group ID:                 0x20
// Start ID:                 0x4D
// Default values:           0x00, 0x40,
// Descriptions:
//   MODEM_RSSI_CONTROL2 - RSSI Jump Detection control.
//   MODEM_RSSI_COMP - RSSI compensation value.
*/
#define RF_MODEM_RSSI_CONTROL2_2 0x11, 0x20, 0x02, 0x4D, 0x18, 0x40

/*
// Set properties:           RF_MODEM_RAW_SEARCH2_2
// Number of properties:     2
// Group ID:                 0x20
// Start ID:                 0x50
// Default values:           0x00, 0x08,
// Descriptions:
//   MODEM_RAW_SEARCH2 - Defines and controls the search period length for the Moving Average and Min-Max detectors.
//   MODEM_CLKGEN_BAND - Select PLL Synthesizer output divider ratio as a function of frequency band.
*/
#define RF_MODEM_RAW_SEARCH2_2 0x11, 0x20, 0x02, 0x50, 0x84, 0x08

/*
// Set properties:           RF_MODEM_SPIKE_DET_2
// Number of properties:     2
// Group ID:                 0x20
// Start ID:                 0x54
// Default values:           0x00, 0x00,
// Descriptions:
//   MODEM_SPIKE_DET - Configures the threshold for (G)FSK Spike Detection.
//   MODEM_ONE_SHOT_AFC - Configures parameters for th e One Shot AFC function and for BCR timing/acquisition.
*/
#define RF_MODEM_SPIKE_DET_2 0x11, 0x20, 0x02, 0x54, 0x03, 0x07

/*
// Set properties:           RF_MODEM_RSSI_MUTE_1
// Number of properties:     1
// Group ID:                 0x20
// Start ID:                 0x57
// Default values:           0x00,
// Descriptions:
//   MODEM_RSSI_MUTE - Configures muting of the RSSI to avoid false RSSI interrupts.
*/
#define RF_MODEM_RSSI_MUTE_1 0x11, 0x20, 0x01, 0x57, 0x00

/*
// Set properties:           RF_MODEM_DSA_CTRL1_5
// Number of properties:     5
// Group ID:                 0x20
// Start ID:                 0x5B
// Default values:           0x00, 0x00, 0x00, 0x00, 0x00,
// Descriptions:
//   MODEM_DSA_CTRL1 - Configures parameters for the Signal Arrival Detection circuit block and algorithm.
//   MODEM_DSA_CTRL2 - Configures parameters for the Signal Arrival Detection circuit block and algorithm.
//   MODEM_DSA_QUAL - Configures parameters for the Eye Opening qualification m ethod of the Signal Arrival Detection algorithm.
//   MODEM_DSA_RSSI - Signal Arrival Detect RSSI Qualifier Config
//   MODEM_DSA_MISC - Miscellaneous detection of signal arrival bits.
*/
#define RF_MODEM_DSA_CTRL1_5 0x11, 0x20, 0x05, 0x5B, 0x40, 0x04, 0x06, 0x78, 0x20

/*
// Set properties:           RF_MODEM_CHFLT_RX1_CHFLT_COE13_7_0_12
// Number of properties:     12
// Group ID:                 0x21
// Start ID:                 0x00
// Default values:           0xFF, 0xBA, 0x0F, 0x51, 0xCF, 0xA9, 0xC9, 0xFC, 0x1B, 0x1E, 0x0F, 0x01,
// Descriptions:
//   MODEM_CHFLT_RX1_CHFLT_COE13_7_0 - Filter coefficients for the first set of RX filter coefficients.
//   MODEM_CHFLT_RX1_CHFLT_COE12_7_0 - Filter coefficients for the first set of RX filter coefficients.
//   MODEM_CHFLT_RX1_CHFLT_COE11_7_0 - Filter coefficients for the first set of RX filter coefficients.
//   MODEM_CHFLT_RX1_CHFLT_COE10_7_0 - Filter coefficients for the first set of RX filter coefficients.
//   MODEM_CHFLT_RX1_CHFLT_COE9_7_0 - Filter coefficients for the first set of RX filter coefficients.
//   MODEM_CHFLT_RX1_CHFLT_COE8_7_0 - Filter coefficients for the first set of RX filter coefficients.
//   MODEM_CHFLT_RX1_CHFLT_COE7_7_0 - Filter coefficients for the first set of RX filter coefficients.
//   MODEM_CHFLT_RX1_CHFLT_COE6_7_0 - Filter coefficients for the first set of RX filter coefficients.
//   MODEM_CHFLT_RX1_CHFLT_COE5_7_0 - Filter coefficients for the first set of RX filter coefficients.
//   MODEM_CHFLT_RX1_CHFLT_COE4_7_0 - Filter coefficients for the first set of RX filter coefficients.
//   MODEM_CHFLT_RX1_CHFLT_COE3_7_0 - Filter coefficients for the first set of RX filter coefficients.
//   MODEM_CHFLT_RX1_CHFLT_COE2_7_0 - Filter coefficients for the first set of RX filter coefficients.
*/
#define RF_MODEM_CHFLT_RX1_CHFLT_COE13_7_0_12 0x11, 0x21, 0x0C, 0x00, 0xA2, 0x81, 0x26, 0xAF, 0x3F, 0xEE, 0xC8, 0xC7, 0xDB, 0xF2, 0x02, 0x08

/*
// Set properties:           RF_MODEM_CHFLT_RX1_CHFLT_COE1_7_0_12
// Number of properties:     12
// Group ID:                 0x21
// Start ID:                 0x0C
// Default values:           0xFC, 0xFD, 0x15, 0xFF, 0x00, 0x0F, 0xFF, 0xC4, 0x30, 0x7F, 0xF5, 0xB5,
// Descriptions:
//   MODEM_CHFLT_RX1_CHFLT_COE1_7_0 - Filter coefficients for the first set of RX filter coefficients.
//   MODEM_CHFLT_RX1_CHFLT_COE0_7_0 - Filter coefficients for the first set of RX filter coefficients.
//   MODEM_CHFLT_RX1_CHFLT_COEM0 - Filter coefficients for the first set of RX filter coefficients.
//   MODEM_CHFLT_RX1_CHFLT_COEM1 - Filter coefficients for the first set of RX filter coefficients.
//   MODEM_CHFLT_RX1_CHFLT_COEM2 - Filter coefficients for the first set of RX filter coefficients.
//   MODEM_CHFLT_RX1_CHFLT_COEM3 - Filter coefficients for the first set of RX filter coefficients.
//   MODEM_CHFLT_RX2_CHFLT_COE13_7_0 - Filter coefficients for the second set of RX filter coefficients.
//   MODEM_CHFLT_RX2_CHFLT_COE12_7_0 - Filter coefficients for the second set of RX filter coefficients.
//   MODEM_CHFLT_RX2_CHFLT_COE11_7_0 - Filter coefficients for the second set of RX filter coefficients.
//   MODEM_CHFLT_RX2_CHFLT_COE10_7_0 - Filter coefficients for the second set of RX filter coefficients.
//   MODEM_CHFLT_RX2_CHFLT_COE9_7_0 - Filter coefficients for the second set of RX filter coefficients.
//   MODEM_CHFLT_RX2_CHFLT_COE8_7_0 - Filter coefficients for the second set of RX filter coefficients.
*/
#define RF_MODEM_CHFLT_RX1_CHFLT_COE1_7_0_12 0x11, 0x21, 0x0C, 0x0C, 0x07, 0x03, 0x15, 0xFC, 0x0F, 0x00, 0xA2, 0x81, 0x26, 0xAF, 0x3F, 0xEE

/*
// Set properties:           RF_MODEM_CHFLT_RX2_CHFLT_COE7_7_0_12
// Number of properties:     12
// Group ID:                 0x21
// Start ID:                 0x18
// Default values:           0xB8, 0xDE, 0x05, 0x17, 0x16, 0x0C, 0x03, 0x00, 0x15, 0xFF, 0x00, 0x00,
// Descriptions:
//   MODEM_CHFLT_RX2_CHFLT_COE7_7_0 - Filter coefficients for the second set of RX filter coefficients.
//   MODEM_CHFLT_RX2_CHFLT_COE6_7_0 - Filter coefficients for the second set of RX filter coefficients.
//   MODEM_CHFLT_RX2_CHFLT_COE5_7_0 - Filter coefficients for the second set of RX filter coefficients.
//   MODEM_CHFLT_RX2_CHFLT_COE4_7_0 - Filter coefficients for the second set of RX filter coefficients.
//   MODEM_CHFLT_RX2_CHFLT_COE3_7_0 - Filter coefficients for the second set of RX filter coefficients.
//   MODEM_CHFLT_RX2_CHFLT_COE2_7_0 - Filter coefficients for the second set of RX filter coefficients.
//   MODEM_CHFLT_RX2_CHFLT_COE1_7_0 - Filter coefficients for the second set of RX filter coefficients.
//   MODEM_CHFLT_RX2_CHFLT_COE0_7_0 - Filter coefficients for the second set of RX filter coefficients.
//   MODEM_CHFLT_RX2_CHFLT_COEM0 - Filter coefficients for the second set of RX filter coefficients.
//   MODEM_CHFLT_RX2_CHFLT_COEM1 - Filter coefficients for the second set of RX filter coefficients.
//   MODEM_CHFLT_RX2_CHFLT_COEM2 - Filter coefficients for the second set of RX filter coefficients.
//   MODEM_CHFLT_RX2_CHFLT_COEM3 - Filter coefficients for the second set of RX filter coefficients.
*/
#define RF_MODEM_CHFLT_RX2_CHFLT_COE7_7_0_12 0x11, 0x21, 0x0C, 0x18, 0xC8, 0xC7, 0xDB, 0xF2, 0x02, 0x08, 0x07, 0x03, 0x15, 0xFC, 0x0F, 0x00

/*
// Set properties:           RF_PA_MODE_4
// Number of properties:     4
// Group ID:                 0x22
// Start ID:                 0x00
// Default values:           0x08, 0x7F, 0x00, 0x5D,
// Descriptions:
//   PA_MODE - Selects the PA operating mode, and selects resolution of PA power adjustment (i.e., step size).
//   PA_PWR_LVL - Configuration of PA output power level.
//   PA_BIAS_CLKDUTY - Configuration of the PA Bias and duty cycle of the TX clock source.
//   PA_TC - Configuration of PA ramping parameters.
*/
#define RF_PA_MODE_4 0x11, 0x22, 0x04, 0x00, 0x08, 0x7F, 0x00, 0x1D

/*
// Set properties:           RF_SYNTH_PFDCP_CPFF_7
// Number of properties:     7
// Group ID:                 0x23
// Start ID:                 0x00
// Default values:           0x2C, 0x0E, 0x0B, 0x04, 0x0C, 0x73, 0x03,
// Descriptions:
//   SYNTH_PFDCP_CPFF - Feed forward charge pump current selection.
//   SYNTH_PFDCP_CPINT - Integration charge pump current selection.
//   SYNTH_VCO_KV - Gain scaling factors (Kv) for the VCO tuning varactors on both the integrated-path and feed forward path.
//   SYNTH_LPFILT3 - Value of resistor R2 in feed-forward path of loop filter.
//   SYNTH_LPFILT2 - Value of capacitor C2 in feed-forward path of loop filter.
//   SYNTH_LPFILT1 - Value of capacitors C1 and C3 in feed-forward path of loop filter.
//   SYNTH_LPFILT0 - Bias current of the active amplifier in the feed-forward loop filter.
*/
#define RF_SYNTH_PFDCP_CPFF_7 0x11, 0x23, 0x07, 0x00, 0x34, 0x04, 0x0B, 0x04, 0x07, 0x70, 0x03

/*
// Set properties:           RF_MATCH_VALUE_1_12
// Number of properties:     12
// Group ID:                 0x30
// Start ID:                 0x00
// Default values:           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
// Descriptions:
//   MATCH_VALUE_1 - Match value to be compared with the result of logically AND-ing (bit-wise) the Mask 1 value with the received Match 1 byte.
//   MATCH_MASK_1 - Mask value to be logically AND-ed (bit-wise) with the Match 1 byte.
//   MATCH_CTRL_1 - Enable for Packet Match functionality, and configuration of Match Byte 1.
//   MATCH_VALUE_2 - Match value to be compared with the result of logically AND-ing (bit-wise) the Mask 2 value with the received Match 2 byte.
//   MATCH_MASK_2 - Mask value to be logically AND-ed (bit-wise) with the Match 2 byte.
//   MATCH_CTRL_2 - Configuration of Match Byte 2.
//   MATCH_VALUE_3 - Match value to be compared with the result of logically AND-ing (bit-wise) the Mask 3 value with the received Match 3 byte.
//   MATCH_MASK_3 - Mask value to be logically AND-ed (bit-wise) with the Match 3 byte.
//   MATCH_CTRL_3 - Configuration of Match Byte 3.
//   MATCH_VALUE_4 - Match value to be compared with the result of logically AND-ing (bit-wise) the Mask 4 value with the received Match 4 byte.
//   MATCH_MASK_4 - Mask value to be logically AND-ed (bit-wise) with the Match 4 byte.
//   MATCH_CTRL_4 - Configuration of Match Byte 4.
*/
#define RF_MATCH_VALUE_1_12 0x11, 0x30, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

/*
// Set properties:           RF_FREQ_CONTROL_INTE_8
// Number of properties:     8
// Group ID:                 0x40
// Start ID:                 0x00
// Default values:           0x3C, 0x08, 0x00, 0x00, 0x00, 0x00, 0x20, 0xFF,
// Descriptions:
//   FREQ_CONTROL_INTE - Frac-N PLL Synthesizer integer divide number.
//   FREQ_CONTROL_FRAC_2 - Frac-N PLL fraction number.
//   FREQ_CONTROL_FRAC_1 - Frac-N PLL fraction number.
//   FREQ_CONTROL_FRAC_0 - Frac-N PLL fraction number.
//   FREQ_CONTROL_CHANNEL_STEP_SIZE_1 - EZ Frequency Programming channel step size.
//   FREQ_CONTROL_CHANNEL_STEP_SIZE_0 - EZ Frequency Programming channel step size.
//   FREQ_CONTROL_W_SIZE - Set window gating period (in number of crystal reference clock cycles) for counting VCO frequency during calibration.
//   FREQ_CONTROL_VCOCNT_RX_ADJ - Adjust target count for VCO calibration in RX mode.
*/
#define RF_FREQ_CONTROL_INTE_8 0x11, 0x40, 0x08, 0x00, 0x41, 0x0F, 0x4A, 0xD4, 0x1D, 0xEF, 0x20, 0xFF


// AUTOMATICALLY GENERATED CODE!
// DO NOT EDIT/MODIFY BELOW THIS LINE!
// --------------------------------------------

#ifndef FIRMWARE_LOAD_COMPILE
#define RADIO_CONFIGURATION_DATA_ARRAY { \
        0x07, RF_POWER_UP, \
        0x08, RF_GPIO_PIN_CFG, \
        0x06, RF_GLOBAL_XO_TUNE_2, \
        0x05, RF_GLOBAL_CONFIG_1, \
        0x06, RF_INT_CTL_ENABLE_2, \
        0x08, RF_FRR_CTL_A_MODE_4, \
        0x0D, RF_PREAMBLE_TX_LENGTH_9, \
        0x0A, RF_SYNC_CONFIG_6, \
        0x10, RF_PKT_CRC_CONFIG_12, \
        0x10, RF_PKT_RX_THRESHOLD_12, \
        0x10, RF_PKT_FIELD_3_CRC_CONFIG_12, \
        0x10, RF_PKT_RX_FIELD_1_CRC_CONFIG_12, \
        0x09, RF_PKT_RX_FIELD_4_CRC_CONFIG_5, \
        0x08, RF_PKT_CRC_SEED_31_24_4, \
        0x10, RF_MODEM_MOD_TYPE_12, \
        0x05, RF_MODEM_FREQ_DEV_0_1, \
        0x10, RF_MODEM_TX_RAMP_DELAY_12, \
        0x10, RF_MODEM_BCR_NCO_OFFSET_2_12, \
        0x07, RF_MODEM_AFC_LIMITER_1_3, \
        0x05, RF_MODEM_AGC_CONTROL_1, \
        0x10, RF_MODEM_AGC_WINDOW_SIZE_12, \
        0x09, RF_MODEM_RAW_CONTROL_5, \
        0x05, RF_MODEM_RSSI_JUMP_THRESH_1, \
        0x06, RF_MODEM_RSSI_CONTROL2_2, \
        0x06, RF_MODEM_RAW_SEARCH2_2, \
        0x06, RF_MODEM_SPIKE_DET_2, \
        0x05, RF_MODEM_RSSI_MUTE_1, \
        0x09, RF_MODEM_DSA_CTRL1_5, \
        0x10, RF_MODEM_CHFLT_RX1_CHFLT_COE13_7_0_12, \
        0x10, RF_MODEM_CHFLT_RX1_CHFLT_COE1_7_0_12, \
        0x10, RF_MODEM_CHFLT_RX2_CHFLT_COE7_7_0_12, \
        0x08, RF_PA_MODE_4, \
        0x0B, RF_SYNTH_PFDCP_CPFF_7, \
        0x10, RF_MATCH_VALUE_1_12, \
        0x0C, RF_FREQ_CONTROL_INTE_8, \
        0x00 \
 }
#else
#define RADIO_CONFIGURATION_DATA_ARRAY { 0 }
#endif

// DEFAULT VALUES FOR CONFIGURATION PARAMETERS
#define RADIO_CONFIGURATION_DATA_RADIO_XO_FREQ_DEFAULT                     30000000L
#define RADIO_CONFIGURATION_DATA_CHANNEL_NUMBER_DEFAULT                    0x00
#define RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH_DEFAULT               0x10
#define RADIO_CONFIGURATION_DATA_RADIO_STATE_AFTER_POWER_UP_DEFAULT        0x01
#define RADIO_CONFIGURATION_DATA_RADIO_DELAY_CNT_AFTER_RESET_DEFAULT       0x1000

#define RADIO_CONFIGURATION_DATA_RADIO_PATCH_INCLUDED                      0x00
#define RADIO_CONFIGURATION_DATA_RADIO_PATCH_SIZE                          0x00
#define RADIO_CONFIGURATION_DATA_RADIO_PATCH                               {  }

#ifndef RADIO_CONFIGURATION_DATA_ARRAY
#error "This property must be defined!"
#endif

#ifndef RADIO_CONFIGURATION_DATA_RADIO_XO_FREQ
#define RADIO_CONFIGURATION_DATA_RADIO_XO_FREQ          RADIO_CONFIGURATION_DATA_RADIO_XO_FREQ_DEFAULT
#endif

/** @addtogroup HAL_Si4463
* @{ \brief This is the Si4463 module
* \details \copydetails lib_radio.c
*/

/**
 * \brief
 * Struct containing handles and/or values of hardware setup.
 *
 * Struct containing handles and/or values of hardware setup necessary for establishing communication
 * with radio Si4463 through SPI.
 */
typedef struct{
    SPI_HandleTypeDef *SPI_Handle;
    GPIO_TypeDef *ChipSelectGPIOPort;
    uint16_t ChipSelectGPIOPin;
}SI4463_HANDLER_S_T;

#define SEND_VALUE_ARRAY_SIZE                   19u             /**< Size of array used for sending configuration/messages to radio. */
#define RADIO_CONFIGURATION_DATA_ARRAY_SIZE     402u            /**< Size of array with configuration. */
#define START_TX                                0x31u           /**< Byte command for starting TX (from Si4463 documentation). */
#define WRITE_TX_FIFO                           0x66u           /**< Byte command for writing data to TX FIFO (from Si4463 documentation). */

static uint8_t s_SendValue[SEND_VALUE_ARRAY_SIZE] = {0x00u};    /* Array used as buffer for sending data */
static uint8_t s_ReadValue[SEND_VALUE_ARRAY_SIZE] = {0x00u};    /* Array used as buffer for receiving data */
static SI4463_HANDLER_S_T s_Si4463_S;                           /* Instance of SI4463_HANDLER_S_T */

HAL_StatusTypeDef sendConfigurationSettings(SPI_HandleTypeDef *SPIx, GPIO_TypeDef *ChipSelectPort,
        uint16_t ChipSelectPin){

    HAL_StatusTypeDef Status = HAL_OK;

    const uint8_t RadioConfigurationDataArray_C[RADIO_CONFIGURATION_DATA_ARRAY_SIZE] = RADIO_CONFIGURATION_DATA_ARRAY;
    uint16_t CurrentCommandLengthIndex = 0u;
    uint8_t* LenPointer;
    uint8_t Commands_Sent = 0u;

    /* writing arguments to struct variables for further usage. */
    s_Si4463_S.SPI_Handle = SPIx;
    s_Si4463_S.ChipSelectGPIOPort = ChipSelectPort;
    s_Si4463_S.ChipSelectGPIOPin = ChipSelectPin;

    /* set pointer to the beginning of configuration data array*/
    LenPointer = &RadioConfigurationDataArray_C[0];

    /*
     * send data configuration to Si4463, using SPI, as long as:
     * - iterator doesn't exceed configuration data array size
     * - LenPointer is not 0x00 (last index in defined RADIO_CONFIGURATION_DATA_ARRAY, signalizing end of array)
     * - the previous transmission was successful
     */
    while((RADIO_CONFIGURATION_DATA_ARRAY_SIZE > (CurrentCommandLengthIndex + Commands_Sent))
            && (*LenPointer != 0x00)
            && (HAL_OK == Status)){

        Commands_Sent++;

        /*
         * copy data from configuration data array to sending buffer (size of each configuration command is
         * specified in RADIO_CONFIGURATION_DATA_ARRAY as first byte in every row)
         */
        memcpy(&s_SendValue[0], &RadioConfigurationDataArray_C[CurrentCommandLengthIndex + Commands_Sent], *LenPointer*sizeof(uint8_t));

        HAL_GPIO_WritePin(s_Si4463_S.ChipSelectGPIOPort, s_Si4463_S.ChipSelectGPIOPin, GPIO_PIN_RESET);
        Status = HAL_SPI_Transmit(s_Si4463_S.SPI_Handle, s_SendValue, *LenPointer, 500u);
        HAL_GPIO_WritePin(s_Si4463_S.ChipSelectGPIOPort, s_Si4463_S.ChipSelectGPIOPin, GPIO_PIN_SET);
        HAL_Delay(100u);

        CurrentCommandLengthIndex = CurrentCommandLengthIndex + *LenPointer;
        LenPointer = &RadioConfigurationDataArray_C[CurrentCommandLengthIndex + Commands_Sent];
    }
    return Status;
}

HAL_StatusTypeDef sendMessage(uint8_t* MessageFromUser, uint8_t MessageLength){

    HAL_StatusTypeDef Status = HAL_OK;

    /* load message from user to TX FIFO */
    s_SendValue[0u] = WRITE_TX_FIFO;
    memcpy(&s_SendValue[1u], MessageFromUser, MessageLength);
    HAL_GPIO_WritePin(s_Si4463_S.ChipSelectGPIOPort, s_Si4463_S.ChipSelectGPIOPin, GPIO_PIN_RESET);
    Status = HAL_SPI_Transmit(s_Si4463_S.SPI_Handle, s_SendValue, MessageLength+1u, 500u);
    HAL_GPIO_WritePin(s_Si4463_S.ChipSelectGPIOPort, s_Si4463_S.ChipSelectGPIOPin, GPIO_PIN_SET);
    HAL_Delay(100u);

    /* load transmission settings and transmit user message through radio */
    s_SendValue[0] = START_TX;
    s_SendValue[1u] = RADIO_CONFIGURATION_DATA_CHANNEL_NUMBER_DEFAULT;
    s_SendValue[2u] = 0x30u; //CONDITION - TXCOMPLETE_STATE - READY [00110000]
    s_SendValue[3u] = 0x00u;
    s_SendValue[4u] = 0x00u;
    s_SendValue[5u] = 0x00u;
    s_SendValue[6u] = 0x00u;
    if(HAL_OK == Status){
        HAL_GPIO_WritePin(s_Si4463_S.ChipSelectGPIOPort, s_Si4463_S.ChipSelectGPIOPin, GPIO_PIN_RESET);
        Status = HAL_SPI_Transmit(s_Si4463_S.SPI_Handle, s_SendValue, 7u, 500u);
        HAL_GPIO_WritePin(s_Si4463_S.ChipSelectGPIOPort, s_Si4463_S.ChipSelectGPIOPin, GPIO_PIN_SET);
        HAL_Delay(100u);
    }
    return Status;
}

HAL_StatusTypeDef getRadioIntStatus(uint8_t* IntResponse){

    HAL_StatusTypeDef Status = HAL_OK;

    /* send command to load radio command buffer with informations about interrupt statuses */
    s_SendValue[0] = 0x20u; //GET_INT_STATUS
    HAL_GPIO_WritePin(s_Si4463_S.ChipSelectGPIOPort, s_Si4463_S.ChipSelectGPIOPin, GPIO_PIN_RESET);
    Status = HAL_SPI_Transmit(s_Si4463_S.SPI_Handle, s_SendValue, 1u, 500u);
    HAL_GPIO_WritePin(s_Si4463_S.ChipSelectGPIOPort, s_Si4463_S.ChipSelectGPIOPin, GPIO_PIN_SET);

    /* read radio command buffer */
    if(HAL_OK == Status){
        /* clear sending array */
        memset(s_SendValue, 0x00u, SEND_VALUE_ARRAY_SIZE);
        s_SendValue[0] = 0x44; //READ_CMD_BUFFER
        HAL_GPIO_WritePin(s_Si4463_S.ChipSelectGPIOPort, s_Si4463_S.ChipSelectGPIOPin, GPIO_PIN_RESET);
        Status = HAL_SPI_TransmitReceive(s_Si4463_S.SPI_Handle, s_SendValue, s_ReadValue, 16u, 500u);
        HAL_GPIO_WritePin(s_Si4463_S.ChipSelectGPIOPort, s_Si4463_S.ChipSelectGPIOPin, GPIO_PIN_SET);
    }

    /* copy interrupt statuses to array provided by user */
    memcpy(IntResponse, s_ReadValue, 16u);

    return Status;
}
/**
* @}
*/
