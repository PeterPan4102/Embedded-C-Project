/**
 * @file HAL_GPIO.c
 * @author Dung Van Pham (dungpv00@gmail.com)
 * @brief HAL GPIO driver implementation for S32K144
 * @version 0.1
 * @date 2025-12-20
 *
 * @copyright Copyright (c) 2025
 *
 */

/*==================================================================================================
* INCLUDES
==================================================================================================*/
#include "HAL_GPIO.h"
#include "S32K144.h"

/*==================================================================================================
* VARIABLES
==================================================================================================*/
/* Virtual pin mapping table */
static const pin_map_t s_pinMap[HAL_GPIO_MAX_PINS] = {
    [0] = { IP_PTD, IP_PORTD,  0 },  /* LED xanh  - PTD0  */
    [1] = { IP_PTD, IP_PORTD, 15 },  /* LED đỏ    - PTD15 */
    [2] = { IP_PTD, IP_PORTD, 16 },  /* LED xanhL - PTD16 */
    [3] = { IP_PTC, IP_PORTC, 12 },  /* Nút BTN0   - PTC12 */
    [4] = { IP_PTC, IP_PORTC, 13 },  /* Nút BTN1   - PTC13 */
};

/* Callback functions for GPIO events */
static HAL_GPIO_Callback_t s_gpioCallbacks[HAL_GPIO_MAX_PINS];

/**
 * @brief Get the pin mapping for a virtual pin
 *
 * @param vpin Virtual pin number
 * @return const pin_map_t* Pointer to the pin mapping structure or NULL if invalid
 */
static const pin_map_t *HAL_GPIO_GetMap(ARM_GPIO_Pin_t vpin)
{
    if (vpin >= HAL_GPIO_MAX_PINS) {
        return NULL;
    }

    if ((s_pinMap[vpin].gpio == NULL) || (s_pinMap[vpin].port == NULL)) {
        return NULL;
    }
    return &s_pinMap[vpin];
}

/**
 * @brief Enable clock for the GPIO port
 *
 * @param map Pointer to the pin mapping structure
 */
static void HAL_GPIO_EnablePortClock(const pin_map_t *map)
{
    if (map->port == IP_PORTC) {
        IP_PCC->PCCn[PCC_PORTC_INDEX] |= PCC_PCCn_CGC_MASK;
    } else if (map->port == IP_PORTD) {
        IP_PCC->PCCn[PCC_PORTD_INDEX] |= PCC_PCCn_CGC_MASK;
    }
}

/*==================================================================================================
* API HALs
===================================================================================================*/
/**
 * @brief Setup GPIO pin
 *
 * @param pin Virtual pin number
 * @param cb_event Callback function for GPIO events
 * @return int32_t Error code or ARM_DRIVER_OK
 */
int32_t HAL_GPIO_Setup(ARM_GPIO_Pin_t pin, HAL_GPIO_Callback_t cb_event)
{
    const pin_map_t *map = HAL_GPIO_GetMap(pin);
    if (map == NULL) {
        return ARM_GPIO_ERROR_PIN;
    }

    HAL_GPIO_EnablePortClock(map);

    uint32_t pcr = map->port -> PCR[map->pin];
    pcr &= ~PORT_PCR_MUX_MASK;
    pcr |= PORT_PCR_MUX(1);
    map->port -> PCR[map->pin] = pcr;

    s_gpioCallbacks[pin] = cb_event;

    return ARM_DRIVER_OK;
}

/**
 * @brief Set GPIO pin direction
 *
 * @param pin Virtual pin number
 * @param direction Direction (input/output)
 * @return int32_t Error code or ARM_DRIVER_OK
 */
int32_t HAL_GPIO_SetDirection(ARM_GPIO_Pin_t pin, ARM_GPIO_DIRECTION direction)
{
    const pin_map_t *map = HAL_GPIO_GetMap(pin);
    if (map == NULL) {
        return ARM_GPIO_ERROR_PIN;
    }

    switch (direction) {
    case ARM_GPIO_INPUT:
        map->gpio -> PDDR &= ~(1UL << map->pin); /* 0 = input */
        break;
    case ARM_GPIO_OUTPUT:
        map->gpio -> PDDR |=  (1UL << map->pin); /* 1 = output */
        break;
    default:
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    return ARM_DRIVER_OK;
}

/**
 * @brief Set GPIO pin output mode
 *
 * @param pin Virtual pin number
 * @param mode Output mode (push-pull, open-drain)
 * @return int32_t Error code or ARM_DRIVER_OK
 */
int32_t HAL_GPIO_SetOutputMode(ARM_GPIO_Pin_t pin, ARM_GPIO_OUTPUT_MODE mode)
{
    const pin_map_t *map = HAL_GPIO_GetMap(pin);
    (void)map;

    if (NULL == map) {
        return ARM_GPIO_ERROR_PIN;
    }

    if (ARM_GPIO_PUSH_PULL == mode) {
        return ARM_DRIVER_OK;
    } else {
        return ARM_DRIVER_ERROR_PARAMETER;
    }
}

/**
 * @brief Set GPIO pin pull resistor
 *
 * @param pin Virtual pin number
 * @param resistor Pull resistor setting (none, up, down)
 * @return int32_t Error code or ARM_DRIVER_OK
 */
int32_t HAL_GPIO_SetPullResistor(ARM_GPIO_Pin_t pin, ARM_GPIO_PULL_RESISTOR resistor)
{
    const pin_map_t *map = HAL_GPIO_GetMap(pin);
    if (NULL == map)
    {
        return ARM_GPIO_ERROR_PIN;
    }

    uint32_t pcr = map->port->PCR[map->pin];

    pcr &= ~(PORT_PCR_PE_MASK | PORT_PCR_PS_MASK);

    switch (resistor)
    {
    case ARM_GPIO_PULL_NONE:
        break;
    case ARM_GPIO_PULL_UP:
        pcr |= (PORT_PCR_PE_MASK | PORT_PCR_PS_MASK);
        break;
    case ARM_GPIO_PULL_DOWN:
        pcr |= PORT_PCR_PE_MASK;
        break;
    default:
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    map->port->PCR[map->pin] = pcr;

    return ARM_DRIVER_OK;
}

/**
 * @brief Set GPIO pin event trigger
 *
 * @param pin Virtual pin number
 * @param trigger Event trigger setting
 * @return int32_t Error code or ARM_DRIVER_OK
 */
int32_t HAL_GPIO_SetEventTrigger(ARM_GPIO_Pin_t pin, ARM_GPIO_EVENT_TRIGGER trigger)
{
    const pin_map_t *map = HAL_GPIO_GetMap(pin);
    if (NULL == map)
    {
        return ARM_GPIO_ERROR_PIN;
    }

    uint32_t pcr = map->port->PCR[map->pin];

    pcr &= ~PORT_PCR_IRQC_MASK;

    switch (trigger)
    {
    case ARM_GPIO_TRIGGER_NONE:
        break;
    default:
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    map->port->PCR[map->pin] = pcr;

    return ARM_DRIVER_OK;
}

/**
 * @brief Write GPIO pin value
 *
 * @param pin Virtual pin number
 * @param val Value to write (0 or 1)
 */
void HAL_GPIO_Write(ARM_GPIO_Pin_t pin, uint32_t val)
{
    const pin_map_t *map = HAL_GPIO_GetMap(pin);
    if (NULL == map) {
        return;
    }

    if (val) {
        map->gpio -> PTOR = (1UL << map->pin);
    } else {
        map->gpio -> PTOR = (1UL << map->pin);
    }
}

/**
 * @brief Read GPIO pin value
 *
 * @param pin Virtual pin number
 * @return uint32_t Pin value (0 or 1)
 */
uint32_t HAL_GPIO_Read(ARM_GPIO_Pin_t pin)
{
    const pin_map_t *map = HAL_GPIO_GetMap(pin);
    if (NULL == map) {
        return 0U;
    }

    uint32_t pdir = map->gpio->PDIR;
    return (pdir >> map->pin) & 1UL;
}
