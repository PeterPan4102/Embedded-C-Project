/**
 * @file Timer_LPIT.h
 * @author Dung Van Pham (dungpv00@gmail.com)
 * @brief LPIT timer driver for S32K144
 * @version 0.1
 * @date 2025-12-20
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef TIMER_LPIT_H_
#define TIMER_LPIT_H_

/*==================================================================================================
* INCLUDES
==================================================================================================*/
#include <stdint.h>
#include "NVIC.h"
#include "adc.h"

#ifdef __cplusplus
extern "C" {
#endif

/*==================================================================================================
* FUNCTION PROTOTYPES
==================================================================================================*/

/**
 * @brief Initialize LPIT0 module and Channel 0 for 1 ms periodic interrupts.
 *
 */
void LPIT0_Init(void);

/**
 * @brief Start LPIT0 Channel 0 timer.
 *
 */
void LPIT0_Ch0_Start(void);

/**
 * @brief Stop LPIT0 Channel 0 timer.
 *
 */
void LPIT0_Ch0_Stop(void);

#ifdef __cplusplus
}
#endif

#endif /* TIMER_LPIT_H_ */
