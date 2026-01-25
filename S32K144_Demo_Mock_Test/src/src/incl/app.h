/**
 * @file app.h
 * @author Dung Van Pham (dungpv00@gmail.com)
 * @brief Application header file for UART LED control example on S32K144 using CMSIS Driver
 * @version 0.1
 * @date 2025-12-20
 *
 * @copyright Copyright (c) 2025
 *
 */
#ifndef APP_H_
#define APP_H_

/*===========================================================================================
 *INCLUDE
 ===========================================================================================*/
#include <stdint.h>

/*===========================================================================================
 *FUNCTION PROTOTYPES
 ===========================================================================================*/
/**
 * @brief Initialize LEDs
 *
 */
void App_InitLeds(void);

/**
 * @brief Send a string via UART
 *
 * @param str Null-terminated string to send
 */
void App_SendString(const char *str);

/**
 * @brief Send command prompt via UART
 *
 */
void App_SendPrompt(void);

/**
 * @brief Process a command received via UART
 *
 * @param cmd Pointer to command buffer
 * @param cmd_len Length of the command
 */
void App_ProcessCommand(const uint8_t *cmd, uint32_t cmd_len);

#endif /* APP_H_ */
