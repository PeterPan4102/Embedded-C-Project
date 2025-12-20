/**
 * @file adc.h
 * @author Dung Van Pham (dungpv00@gmail.com)
 * @brief ADC driver header for S32K144
 * @version 0.1
 * @date 2025-12-20
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef ADC_H_
#define ADC_H_

/*==================================================================================================
* INCLUDES
==================================================================================================*/
#include "device_registers.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*==================================================================================================
* PUBLIC APIs
==================================================================================================*/
/**
 * @brief Initialize ADC0 peripheral:
 * - Enable clock for ADC0 and its input port.
 * - Configure input pin as analog.
 * - Configure core ADC0 registers for software-triggered conversion.
 * - Run calibration routine.
 *
 */
void ADC0_Init(void);

/**
 * @brief Read one ADC channel using polling method.
 *
 * @param ch  ADC channel number.
 * @return uint16_t Raw ADC conversion result.
 */
uint16_t ADC0_ReadChannel_Polling(uint8_t ch);

/**
 * @brief Read ADC channel and convert raw result to voltage in mV.
 *
 * @param ch ADC channel number.
 * @param vref Reference voltage in the same unit you want returned (e.g., 5000 for mV).
 * @return float Voltage computed from raw code and vref.
 *
 */
float ADC0_ReadVoltage_Polling(uint8_t ch, float vref);

#ifdef __cplusplus
}
#endif

#endif /* ADC_H_ */
