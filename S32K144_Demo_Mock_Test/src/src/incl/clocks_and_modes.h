/**
 * @file clocks_and_modes.h
 * @author Dung Van Pham (dungpv00@gmail.com)
 * @brief Header file for clock and mode initialization functions for S32K144
 * @version 0.1
 * @date 2025-12-20
 *
 * @copyright Copyright (c) 2025
 *
 */
#ifndef CLOCKS_AND_MODES_H_
#define CLOCKS_AND_MODES_H_

/*==================================================================================================
* INCLUDE FILES
==================================================================================================*/
#include "device_registers.h"

#ifdef __cplusplus
extern "C" {
#endif

/*===================================================================================================
 *FUNCTIONS
 ===================================================================================================*/

/**
 * @brief Initialize System Oscillator (SOSC) for 8 MHz external crystal.
 *
 */
void SOSC_init_8MHz(void);

/**
 * @brief Initialize System PLL (SPLL) for 160 MHz system clock.
 *
 */
void SPLL_init_160MHz(void);

/**
 * @brief Initialize Normal RUN mode for 80 MHz system clock.
 *
 */
void NormalRUNmode_80MHz(void);


#ifdef __cplusplus
}
#endif

#endif /* CLOCKS_AND_MODES_H_ */
