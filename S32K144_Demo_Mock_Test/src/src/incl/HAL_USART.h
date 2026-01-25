/**
 * @file HAL_USART.h
 * @author Dung Van Pham (dungpv00@gmail.com)
 * @brief HAL USART header file
 * @version 0.1
 * @date 2025-12-20
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef _HAL_UART_H_
#define _HAL_UART_H_

/*==================================================================================================
* INCLUDES
==================================================================================================*/
#include "Driver_Common.h"
#include "Driver_USART.h"

#ifdef __cplusplus
extern "C" {
#endif

/*==================================================================================================
* TYPEDEFS
==================================================================================================*/

typedef enum
{
    HAL_UART_POWER_OFF = 0U,
    HAL_UART_POWER_LOW,
    HAL_UART_POWER_FULL
} HAL_UART_PowerState_t;

typedef struct
{
    uint8_t                 initialized;
    ARM_USART_SignalEvent_t cb_event;
} HAL_UART_Context_t;

/*==================================================================================================
* API PROTOTYPES
==================================================================================================*/

/**
 * @brief Initialize USART driver
 *
 * @param cb_event Callback function pointer
 * @return int32_t Execution status
 */
int32_t HAL_ARM_USART_Initialize(ARM_USART_SignalEvent_t cb_event);

/**
 * @brief Uninitialize USART driver
 *
 * @return int32_t Execution status
 */
int32_t HAL_ARM_USART_Uninitialize(void);

/**
 * @brief Control USART settings
 *
 * @param control Control parameter
 * @param arg Argument for control parameter
 * @return int32_t Execution status
 */
int32_t HAL_ARM_USART_Control(uint32_t control, uint32_t arg);

/**
 * @brief Control USART power state
 *
 * @param power_state Desired power state
 * @return int32_t Execution status
 */
int32_t HAL_ARM_UART_PowerControl(HAL_UART_PowerState_t power_state);

/**
 * @brief Send data over USART
 *
 * @param data Pointer to data buffer
 * @param num Number of bytes to send
 * @return int32_t Execution status
 */
int32_t HAL_ARM_USART_Send(const uint8_t *data, uint32_t num);

/**
 * @brief Receive data over USART
 *
 * @param data Pointer to data buffer
 * @param num Number of bytes to receive
 * @return int32_t Execution status
 */
int32_t HAL_ARM_USART_Receive(uint8_t *data, uint32_t num);

/**
 * @brief Transfer data to/from USART
 *
 * @param data_out Pointer to data to send
 * @param data_in Pointer to data to receive
 * @param num Number of data bytes to transfer
 * @return int32_t Execution status
 */
int32_t HAL_ARM_USART_Transfer(const uint8_t *data_out, uint8_t *data_in, uint32_t num);

/**
 * @brief Get transmitted data count
 *
 * @return uint32_t Number of transmitted data
 */
uint32_t HAL_ARM_USART_GetTxCount(void);

/**
 * @brief Get received data count
 *
 * @return uint32_t Number of received data
 */
uint32_t HAL_ARM_USART_GetRxCount(void);

/**
 * @brief Get USART status
 *
 * @return ARM_USART_STATUS Current USART status
 */
ARM_USART_STATUS HAL_ARM_USART_GetStatus(void);

/**
 * @brief Set modem control signals
 *
 * @param control Modem control command
 * @return int32_t Execution status
 */
int32_t HAL_ARM_USART_SetModemControl(ARM_USART_MODEM_CONTROL control);

/**
 * @brief Get modem status signals
 *
 * @return ARM_USART_MODEM_STATUS Current modem status
 */
ARM_USART_MODEM_STATUS HAL_ARM_USART_GetModemStatus(void);

#ifdef __cplusplus
}
#endif

#endif /* _HAL_UART_H_ */
