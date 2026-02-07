/**
 * @file app.c
 * @author Dung Van Pham (dungpv00@gmail.com)
 * @brief Application code
 * @version 0.1
 * @date 2025-12-20
 *
 * @copyright Copyright (c) 2025
 *
 */

/*=============================================================================
 * INCLUDES
 =============================================================================*/
#include "incl/app.h"

#include <stdint.h>
#include <string.h>
#include "incl/Driver_GPIO.h"
#include "incl/Driver_USART.h"

/*=============================================================================
 * DEFINES
 =============================================================================*/

#define LED_BLUE_PIN          (0U)
#define LED_RED_PIN           (1U)
#define LED_GREEN_PIN         (2U)

/* EVB, LED RGB active-low: 0 = LED ON, 1 = LED OFF */
#define LED_LEVEL_ON          (0U)
#define LED_LEVEL_OFF         (1U)

/*=============================================================================
 * EXTERN DRIVER INSTANCES
 =============================================================================*/
extern ARM_DRIVER_GPIO  Driver_GPIO0;
extern ARM_DRIVER_USART Driver_USART0;

/*=============================================================================
 * FUNCTION PROTOTYPES
 =============================================================================*/
volatile uint32_t usart_events = 0U;


/*=============================================================================
 * FUNCTIONS
 =============================================================================*/

void USART1_SignalEvent(uint32_t event)
{
	usart_events |= event;
}

/**
 * @brief Send a string via USART
 *
 * @param str Null-terminated string to send
 */
void App_SendString(const char *str)
{
    ARM_DRIVER_USART *usart;
    uint32_t          len;
    int32_t           status;

    usart  = &Driver_USART0;
    len    = (uint32_t)strlen(str);
    status = ARM_DRIVER_OK;

        if (len > 0U)
        {
            status = usart->Send(str, len);
        }

    while(0U == (usart_events & ARM_USART_EVENT_SEND_COMPLETE))
    {
        /* Wait until send is complete */
    }

    usart_events &= ~ARM_USART_EVENT_SEND_COMPLETE;
    (void)status;
}



