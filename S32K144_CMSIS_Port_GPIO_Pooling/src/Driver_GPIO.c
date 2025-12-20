/**
 * @file Driver_GPIO.c
 * @author Dung Van Pham (dungpv00@gmail.com)
 * @brief GPIO Driver implementation file (CMSIS-Driver style)
 * @version 0.1
 * @date 2025-12-20
 *
 * @copyright Copyright (c) 2025
 *
 */

/*==================================================================================================
* INCLUDES
==================================================================================================*/
#include "Driver_GPIO.h"
#include "HAL_GPIO.h"     /* HAL implementation for platform-specific access */

/*==================================================================================================
* DEFINES
==================================================================================================*/
/* Maximum number of virtual pins supported by this driver instance. */
#define GPIO_MAX_PINS                 (64U)

/* Check whether a virtual pin ID is within supported range. */
#define PIN_IS_AVAILABLE(n)           ((n) < GPIO_MAX_PINS)

/*==================================================================================================
* LOCAL FUNCTIONS
==================================================================================================*/

/**
 * @brief Setup GPIO pin.
 *
 * @param pin GPIO Pin
 * @param cb_event Optional callback for GPIO events/interrupt (can be NULL).
 * @return int32_t Execution status
 */
static int32_t GPIO_Setup(ARM_GPIO_Pin_t pin, ARM_GPIO_SignalEvent_t cb_event)
{
    int32_t result = ARM_DRIVER_OK;

    if (PIN_IS_AVAILABLE(pin) != 0U)
    {
        /* Forward to HAL */
        result = HAL_GPIO_Setup(pin, cb_event);
    }
    else
    {
        result = ARM_GPIO_ERROR_PIN;
    }

    return result;
}

/**
 * @brief Set GPIO direction.
 *
 * @param pin GPIO Pin
 * @param direction Direction (input or output)
 * @return int32_t Execution status
 */
static int32_t GPIO_SetDirection(ARM_GPIO_Pin_t pin, ARM_GPIO_DIRECTION direction)
{
    int32_t result = ARM_DRIVER_OK;

    if (PIN_IS_AVAILABLE(pin) != 0U)
    {
        result = HAL_GPIO_SetDirection(pin, direction);
    }
    else
    {
        result = ARM_GPIO_ERROR_PIN;
    }

    return result;
}

/**
 * @brief Set GPIO output mode.
 *
 * @param pin GPIO Pin
 * @param mode Output mode (push-pull or open-drain)
 * @return int32_t Execution status
 */
static int32_t GPIO_SetOutputMode(ARM_GPIO_Pin_t pin, ARM_GPIO_OUTPUT_MODE mode)
{
    int32_t result = ARM_DRIVER_OK;

    if (PIN_IS_AVAILABLE(pin) != 0U)
    {
        result = HAL_GPIO_SetOutputMode(pin, mode);
    }
    else
    {
        result = ARM_GPIO_ERROR_PIN;
    }

    return result;
}

/**
 * @brief Set GPIO pull resistor.
 *
 * @param pin GPIO Pin
 * @param resistor Pull resistor type
 * @return int32_t Execution status
 */
static int32_t GPIO_SetPullResistor(ARM_GPIO_Pin_t pin, ARM_GPIO_PULL_RESISTOR resistor)
{
    int32_t result = ARM_DRIVER_OK;

    if (PIN_IS_AVAILABLE(pin) != 0U)
    {
        result = HAL_GPIO_SetPullResistor(pin, resistor);
    }
    else
    {
        result = ARM_GPIO_ERROR_PIN;
    }

    return result;
}

/**
 * @brief Set GPIO event trigger.
 *
 * @param pin GPIO Pin
 * @param trigger Event trigger type
 * @return int32_t Execution status
 */
static int32_t GPIO_SetEventTrigger(ARM_GPIO_Pin_t pin, ARM_GPIO_EVENT_TRIGGER trigger)
{
    int32_t result = ARM_DRIVER_OK;

    if (PIN_IS_AVAILABLE(pin) != 0U)
    {
        result = HAL_GPIO_SetEventTrigger(pin, trigger);
    }
    else
    {
        result = ARM_GPIO_ERROR_PIN;
    }

    return result;
}

/**
 * @brief Set GPIO output value.
 *
 * @param pin GPIO Pin
 * @param val Output value (0 or 1)
 */
static void GPIO_SetOutput(ARM_GPIO_Pin_t pin, uint32_t val)
{
    if (PIN_IS_AVAILABLE(pin) != 0U)
    {
        HAL_GPIO_Write(pin, val);
    }
}

/**
 * @brief Get GPIO input value.
 *
 * @param pin GPIO Pin
 * @return uint32_t Input value (0 or 1)
 */
static uint32_t GPIO_GetInput(ARM_GPIO_Pin_t pin)
{
    uint32_t val = 0U;

    if (PIN_IS_AVAILABLE(pin) != 0U)
    {
        val = HAL_GPIO_Read(pin);
    }

    return val;
}

/*==================================================================================================
* DRIVER INSTANCE
==================================================================================================*/
/**
 * @brief GPIO Driver instance
 *
 */
ARM_DRIVER_GPIO Driver_GPIO0 =
{
    GPIO_Setup,
    GPIO_SetDirection,
    GPIO_SetOutputMode,
    GPIO_SetPullResistor,
    GPIO_SetEventTrigger,
    GPIO_SetOutput,
    GPIO_GetInput
};
