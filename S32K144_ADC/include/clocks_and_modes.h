/**
 * @file clocks_and_modes.h
 * @author Dung Van Pham (dungpv00@gmail.com)
 * @brief Clocks and modes configuration for S32K144
 * @version 0.1
 * @date 2025-12-20
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef CLOCKS_AND_MODES_H_
#define CLOCKS_AND_MODES_H_

/*==================================================================================================
* INCLUDES
==================================================================================================*/
#include "device_registers.h"

#ifdef __cplusplus
extern "C" {
#endif

/*==================================================================================================
* FUNCTION PROTOTYPES
==================================================================================================*/
/**
 * @brief Initialize System Oscillator (SOSC) to use 8 MHz external crystal.
 *
 */
void SOSC_init_8MHz(void);

/**
 * @brief Initialize System PLL (SPLL) to generate 160 MHz system clock.
 *
 */
void SPLL_init_160MHz(void);

/**
 * @brief Switch to Normal RUN mode with 80 MHz core clock using SPLL.
 *
 */
void NormalRUNmode_80MHz(void);

#ifdef __cplusplus
}
#endif

#endif /* CLOCKS_AND_MODES_H_ */

