/**
 * @file HAL_GPIO.h
 * @author Dung Van Pham (dungpv00@gmail.com)
 * @brief HAL GPIO header file for S32K144
 * @version 0.1
 * @date 2025-12-20
 *
 * @copyright Copyright (c) 2025
 *
 */

 #ifndef HAL_GPIO_H_
#define HAL_GPIO_H_

/*==================================================================================================
* INCLUDES
==================================================================================================*/
#include <stdint.h>

#include "S32K144.h"
#include "device_registers.h"
#include "Driver_Common.h"
#include "Driver_GPIO.h"   /* For ARM_GPIO_Pin_t and related enums/types */

#ifdef __cplusplus
extern "C" {
#endif

/*==================================================================================================
* DEFINES
==================================================================================================*/
/* Maximum number of virtual pins supported by the HAL mapping table. */
#define HAL_GPIO_MAX_PINS             (64U)

/*==================================================================================================
* TYPEDEFS
==================================================================================================*/
/**
 * @brief Pin mapping structure
 *
 */
typedef struct
{
    GPIO_Type *gpio;   /* GPIO base (e.g., PTD, PTC, ...) */
    PORT_Type *port;   /* PORT base (e.g., PORTD, PORTC, ...) */
    uint8_t    pin;    /* Bit index within the port */
} pin_map_t;

/**
 * @brief GPIO Callback type
 *
 */
typedef ARM_GPIO_SignalEvent_t HAL_GPIO_Callback_t;

/*==================================================================================================
* FUNCTION PROTOTYPES
==================================================================================================*/
/**
 * @brief Initialize and configure GPIO pin for basic usage.
 *
 * @param pin GPIO Pin
 * @param cb_event Callback function for GPIO events (can be NULL)
 * @return int32_t Execution status
 */
int32_t HAL_GPIO_Setup(ARM_GPIO_Pin_t pin, HAL_GPIO_Callback_t cb_event);

/**
 * @brief Set GPIO Direction.
 *
 * @param pin GPIO Pin
 * @param direction Direction to set (input or output)
 * @return int32_t Execution status
 */
int32_t HAL_GPIO_SetDirection(ARM_GPIO_Pin_t pin, ARM_GPIO_DIRECTION direction);

/**
 * @brief Set GPIO Output Mode.
 *
 * @param pin GPIO Pin
 * @param mode Output mode to set (push-pull or open-drain)
 * @return int32_t Execution status
 */
int32_t HAL_GPIO_SetOutputMode(ARM_GPIO_Pin_t pin, ARM_GPIO_OUTPUT_MODE mode);

/**
 * @brief Set GPIO Pull Resistor.
 *
 * @param pin GPIO Pin
 * @param resistor Pull resistor to set (pull-up, pull-down, or none)
 * @return int32_t Execution status
 */
int32_t HAL_GPIO_SetPullResistor(ARM_GPIO_Pin_t pin, ARM_GPIO_PULL_RESISTOR resistor);

/**
 * @brief Set GPIO Event Trigger.
 *
 * @param pin GPIO Pin
 * @param trigger Event trigger to set (rising edge, falling edge, etc.)
 * @return int32_t Execution status
 */
int32_t HAL_GPIO_SetEventTrigger(ARM_GPIO_Pin_t pin, ARM_GPIO_EVENT_TRIGGER trigger);

/**
 * @brief Write GPIO output value.
 *
 * @param pin GPIO Pin
 * @param val Value to write (0 or 1)
 */
void HAL_GPIO_Write(ARM_GPIO_Pin_t pin, uint32_t val);

/**
 * @brief Read GPIO input value.
 *
 * @param pin GPIO Pin
 * @return uint32_t Input value (0 or 1)
 */
uint32_t HAL_GPIO_Read(ARM_GPIO_Pin_t pin);

#ifdef __cplusplus
}
#endif

#endif /* HAL_GPIO_H_ */
