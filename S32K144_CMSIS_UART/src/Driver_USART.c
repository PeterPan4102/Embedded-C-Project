/**
 * @file Driver_USART.c
 * @author Dung Van Pham (dungpv00@gmail.com)
 * @brief CMSIS USART Driver implementation for S32K144
 * @version 0.1
 * @date 2025-12-20
 *
 * @copyright Copyright (c) 2025
 *
 */

/*==========================================================================
 * INCLUDE
 ============================================================================*/
#include "Driver_USART.h"
#include <HAL_USART.h>

/*==========================================================================
 * DEFINES AND MACROS
 ============================================================================*/
#define ARM_USART_DRV_VERSION    ARM_DRIVER_VERSION_MAJOR_MINOR(1, 0)

/*==========================================================================
 * VARIABLES
 ============================================================================*/
/* Driver Version */
static const ARM_DRIVER_VERSION DriverVersion = {
    ARM_USART_API_VERSION,
    ARM_USART_DRV_VERSION
};

/* Driver Capabilities */
static const ARM_USART_CAPABILITIES DriverCapabilities = {
    1, /* supports UART (Asynchronous) mode */
    0, /* supports Synchronous Master mode */
    0, /* supports Synchronous Slave mode */
    0, /* supports UART Single-wire mode */
    0, /* supports UART IrDA mode */
    0, /* supports UART Smart Card mode */
    0, /* Smart Card Clock generator available */
    0, /* RTS Flow Control available */
    0, /* CTS Flow Control available */
    0, /* Transmit completed event */
    0, /* Signal receive character timeout event */
    0, /* RTS Line available */
    0, /* CTS Line available */
    0, /* DTR Line available */
    0, /* DSR Line available */
    0, /* DCD Line available */
    0, /* RI Line available */
    0, /* Signal CTS change event */
    0, /* Signal DSR change event */
    0, /* Signal DCD change event */
    0, /* Signal RI change event */
    0  /* Reserved (must be zero) */
};

/*==========================================================================
 * FUNCTIONS
 ============================================================================*/

/**
 * @brief Get driver version.
 *
 * @return ARM_DRIVER_VERSION
 */
static ARM_DRIVER_VERSION ARM_USART_GetVersion(void)
{
    return DriverVersion;
}

/**
 * @brief Get driver capabilities.
 *
 * @return ARM_USART_CAPABILITIES
 */
static ARM_USART_CAPABILITIES ARM_USART_GetCapabilities(void)
{
    return DriverCapabilities;
}

/**
 * @brief Initialize USART interface
 *
 * @param cb_event Callback function for USART events
 * @return int32_t Error code or ARM_DRIVER_OK
 */
static int32_t ARM_USART_Initialize(ARM_USART_SignalEvent_t cb_event)
{
    int32_t result;

    result = HAL_ARM_USART_Initialize(cb_event);

    return result;
}

/**
 * @brief Uninitialize USART interface
 *
 * @return int32_t Error code or ARM_DRIVER_OK
 */
static int32_t ARM_USART_Uninitialize(void)
{
    int32_t result;

    result = HAL_ARM_USART_Uninitialize();

    return result;
}

/**
 * @brief Control USART power.
 *
 * @param state Power state
 * @return int32_t Error code or ARM_DRIVER_OK
 */
static int32_t ARM_USART_PowerControl(ARM_POWER_STATE state)
{
    int32_t result;

    switch (state)
    {
        case ARM_POWER_OFF:
            result = HAL_ARM_UART_PowerControl(HAL_UART_POWER_OFF);
            break;

        case ARM_POWER_LOW:
            result = HAL_ARM_UART_PowerControl(HAL_UART_POWER_LOW);
            break;

        case ARM_POWER_FULL:
            result = HAL_ARM_UART_PowerControl(HAL_UART_POWER_FULL);
            break;

        default:
            result = ARM_DRIVER_ERROR_UNSUPPORTED;
            break;
    }

    return result;
}

/**
 * @brief Send data via USART
 *
 * @param data Pointer to data to send
 * @param num Number of data bytes to send
 * @return int32_t Error code or ARM_DRIVER_OK
 */
static int32_t ARM_USART_Send(const void *data, uint32_t num)
{
    int32_t result;

    result = HAL_ARM_USART_Send((const uint8_t *)data, num);

    return result;
}

/**
 * @brief Receive data via USART
 *
 * @param data Pointer to data to receive
 * @param num Number of data bytes to receive
 * @return int32_t Error code or ARM_DRIVER_OK
 */
static int32_t ARM_USART_Receive(void *data, uint32_t num)
{
    int32_t result;

    result = HAL_ARM_USART_Receive((uint8_t *)data, num);

    return result;
}

/**
 * @brief Transfer data to/from USART
 *
 * @param data_out Pointer to data to send
 * @param data_in Pointer to data to receive
 * @param num Number of data bytes to transfer
 * @return int32_t Error code or ARM_DRIVER_OK
 */
static int32_t ARM_USART_Transfer(const void *data_out, void *data_in, uint32_t num)
{
    int32_t result;

    result = HAL_ARM_USART_Transfer((const uint8_t *)data_out, (uint8_t *)data_in, num);

    return result;
}

/**
 * @brief Get transmitted data count
 *
 * @return uint32_t Number of transmitted data
 */
static uint32_t ARM_USART_GetTxCount(void)
{
    uint32_t tx_count;

    tx_count = HAL_ARM_USART_GetTxCount();

    return tx_count;
}

/**
 * @brief Get received data count
 *
 * @return uint32_t Number of received data
 */
static uint32_t ARM_USART_GetRxCount(void)
{
    uint32_t rx_count;

    rx_count = HAL_ARM_USART_GetRxCount();

    return rx_count;
}

/**
 * @brief Control USART settings
 *
 * @param control Control parameter
 * @param arg Argument for control parameter
 * @return int32_t Error code or ARM_DRIVER_OK
 */
static int32_t ARM_USART_Control(uint32_t control, uint32_t arg)
{
    int32_t result;

    result = HAL_ARM_USART_Control(control, arg);

    return result;
}

/**
 * @brief Get USART status
 *
 * @return ARM_USART_STATUS Current USART status
 */
static ARM_USART_STATUS ARM_USART_GetStatus(void)
{
    ARM_USART_STATUS status;

    status = HAL_ARM_USART_GetStatus();

    return status;
}

/**
 * @brief Set modem control signals
 *
 * @param control Modem control command
 * @return int32_t Error code or ARM_DRIVER_OK
 */
static int32_t ARM_USART_SetModemControl(ARM_USART_MODEM_CONTROL control)
{
    int32_t result;

    result = HAL_ARM_USART_SetModemControl(control);

    return result;
}

/**
 * @brief Get modem status signals
 *
 * @return ARM_USART_MODEM_STATUS Current modem status
 */
static ARM_USART_MODEM_STATUS ARM_USART_GetModemStatus(void)
{
    ARM_USART_MODEM_STATUS status;

    status = HAL_ARM_USART_GetModemStatus();

    return status;
}

/**
 * @brief USART Driver instance
 *
 */
ARM_DRIVER_USART Driver_USART0 = {
    ARM_USART_GetVersion,
    ARM_USART_GetCapabilities,
    ARM_USART_Initialize,
    ARM_USART_Uninitialize,
    ARM_USART_PowerControl,
    ARM_USART_Send,
    ARM_USART_Receive,
    ARM_USART_Transfer,
    ARM_USART_GetTxCount,
    ARM_USART_GetRxCount,
    ARM_USART_Control,
    ARM_USART_GetStatus,
    ARM_USART_SetModemControl,
    ARM_USART_GetModemStatus
};
