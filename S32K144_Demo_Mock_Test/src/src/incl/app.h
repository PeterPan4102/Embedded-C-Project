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

extern volatile uint32_t        usart_events;

/*===========================================================================================
 *FUNCTION PROTOTYPES
 ===========================================================================================*/
void USART1_SignalEvent(uint32_t event);

/**
 * @brief Send a string via UART
 *
 * @param str Null-terminated string to send
 */
void App_SendString(const char *str);


#endif /* APP_H_ */
