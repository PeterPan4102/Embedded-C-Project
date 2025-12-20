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
#include <stdint.h>
#include "NVIC.h"

#ifdef __cplusplus
extern "C" {
#endif

/*==================================================================================================
* GLOBAL VARIABLES (updated in ADC0_IRQHandler)
==================================================================================================*/
extern volatile uint16_t g_adcResult;
extern volatile uint32_t g_adcNew;

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
 * @brief Start one conversion by software trigger. Call this from LPIT0_Ch0_IRQHandler().
 *
 */
void ADC0_StartConversion_SwTrigger(void);

#ifdef __cplusplus
}
#endif

#endif /* ADC_H_ */
