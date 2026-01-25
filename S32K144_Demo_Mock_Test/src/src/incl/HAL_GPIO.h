/**
 * @file HAL_GPIO.h
 * @author Dung Van Pham (dungpv00@gmail.com)
 * @brief HAL GPIO header file
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
#include "S32K144.h"
#include "device_registers.h"

#include "Driver_Common.h"
#include "Driver_GPIO.h"

#ifdef __cplusplus
extern "C" {
#endif

/*==================================================================================================
* DEFINES
==================================================================================================*/
#define HAL_GPIO_MAX_PINS   64U

/*==================================================================================================
* TYPEDEFS
==================================================================================================*/
typedef struct {
    GPIO_Type *gpio;
    PORT_Type *port;
    uint8_t    pin;
} pin_map_t;

typedef ARM_GPIO_SignalEvent_t HAL_GPIO_Callback_t;

/*==================================================================================================
* API PROTOTYPES
==================================================================================================*/
/**
 * @brief Setup GPIO pin
 *
 * @param pin GPIO Pin
 * @param cb_event Callback function pointer
 * @return int32_t Execution status
 */
int32_t  HAL_GPIO_Setup        (ARM_GPIO_Pin_t pin, HAL_GPIO_Callback_t cb_event);

/**
 * @brief Set GPIO pin direction
 *
 * @param pin GPIO Pin
 * @param direction Direction of the pin
 * @return int32_t Execution status
 */
int32_t  HAL_GPIO_SetDirection (ARM_GPIO_Pin_t pin, ARM_GPIO_DIRECTION direction);

/**
 * @brief Set GPIO output mode
 *
 * @param pin GPIO Pin
 * @param mode Output mode
 * @return int32_t Execution status
 */
int32_t  HAL_GPIO_SetOutputMode(ARM_GPIO_Pin_t pin, ARM_GPIO_OUTPUT_MODE mode);

/**
 * @brief Set GPIO pin pull resistor
 *
 * @param pin GPIO Pin
 * @param resistor Pull resistor setting
 * @return int32_t Execution status
 */
int32_t  HAL_GPIO_SetPullResistor(ARM_GPIO_Pin_t pin, ARM_GPIO_PULL_RESISTOR resistor);

/**
 * @brief Set GPIO pin event trigger
 *
 * @param pin GPIO Pin
 * @param trigger Event trigger setting
 * @return int32_t Execution status
 */
int32_t  HAL_GPIO_SetEventTrigger(ARM_GPIO_Pin_t pin, ARM_GPIO_EVENT_TRIGGER trigger);

/**
 * @brief Write GPIO pin output value
 *
 * @param pin GPIO Pin
 * @param val Output value (0 or 1)
 */
void     HAL_GPIO_Write        (ARM_GPIO_Pin_t pin, uint32_t val);

/**
 * @brief Read GPIO pin input value
 *
 * @param pin GPIO Pin
 * @return uint32_t Input value (0 or 1)
 */
uint32_t HAL_GPIO_Read         (ARM_GPIO_Pin_t pin);

#ifdef __cplusplus
}
#endif

#endif /* HAL_GPIO_H_ */
