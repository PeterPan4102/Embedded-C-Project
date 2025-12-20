/**
 * @file main.c
 * @author Dung Van Pham (dungpv00@gmail.com)
 * @brief Main file for UART LED control example on S32K144 using CMSIS Driver
 * @version 0.1
 * @date 2025-12-20
 *
 * @copyright Copyright (c) 2025
 *
 */
/*============================================================================
 * INCLUDE
 =============================================================================*/
#include "S32K144.h"
#include "device_registers.h"
#include "clocks_and_modes.h"
#include "Driver_USART.h"
#include "Driver_GPIO.h"
#include "app.h"

/*============================================================================
 * DEFINES
 =============================================================================*/
#define UART_CMD_BUFFER_SIZE   (32U)

/*============================================================================
 * EXTERN DRIVER INSTANCES
 =============================================================================*/

extern ARM_DRIVER_USART Driver_USART0;
extern ARM_DRIVER_GPIO  Driver_GPIO0;

/*============================================================================
 * FUNCTION PROTOTYPES
 =============================================================================*/

static void WDOG_disable(void);
static void App_InitClock(void);

/*============================================================================
 * FUNCTIONS
 =============================================================================*/
/**
 * @brief Disable the watchdog timer
 *
 */
void WDOG_disable (void){
 IP_WDOG->CNT   = 0xD928C520; /* Unlock watchdog */
 IP_WDOG->TOVAL = 0x0000FFFF; /* Maximum timeout value */
 IP_WDOG->CS    = 0x00002100; /* Disable watchdog */
}

/**
 * @brief Initialize system clock to 80 MHz
 *
 */
static void App_InitClock(void)
{
    SOSC_init_8MHz();
    SPLL_init_160MHz();
    NormalRUNmode_80MHz();
}

/*===============================================================================
 * MAIN CODE
 ================================================================================*/
int main(void)
{
    ARM_DRIVER_USART *usart;
    int32_t           status;
    uint8_t           rx_char;
    uint8_t           cmd_buffer[UART_CMD_BUFFER_SIZE];
    uint32_t          cmd_index;
    uint8_t           s_ignoreLf = 0U;


    usart = &Driver_USART0;

    WDOG_disable();
    App_InitClock();

    status = usart->Initialize(NULL);
    if (ARM_DRIVER_OK == status)
    {
        status = usart->PowerControl(ARM_POWER_FULL);
    }

    if (ARM_DRIVER_OK == status)
    {
        status = usart->Control(ARM_USART_MODE_ASYNCHRONOUS   |
                                ARM_USART_DATA_BITS_8         |
                                ARM_USART_PARITY_NONE         |
                                ARM_USART_STOP_BITS_1,
                                9600U);
    }

    if (ARM_DRIVER_OK == status)
    {
        status = usart->Control(ARM_USART_CONTROL_TX, 1U);
    }

    if (ARM_DRIVER_OK == status)
    {
        status = usart->Control(ARM_USART_CONTROL_RX, 1U);
    }

    if (ARM_DRIVER_OK == status)
    {
        App_InitLeds();
    }

    if (ARM_DRIVER_OK != status)
    {
        while (1)
        {
        }
    }

    cmd_index     = 0U;
    cmd_buffer[0] = '\0';

    App_SendString("\r\n*** UART LED CONTROL DEMO ***\r\n");
    App_SendString("Type \"HELP\" for command list.\r\n");
    App_SendPrompt();

    while (1)
    {
        status = usart->Receive(&rx_char, 1U);

        if (ARM_DRIVER_OK == status)
        {
            if (rx_char == (uint8_t)'\r')
            {
                s_ignoreLf = 1U;

                if (cmd_index > 0U)
                {
                    App_ProcessCommand(cmd_buffer, cmd_index);
                    cmd_index = 0U;
                }

                App_SendPrompt();
            }
            else if (rx_char == (uint8_t)'\n')
            {
                if (s_ignoreLf != 0U)
                {
                    s_ignoreLf = 0U;
                }
                else
                {
                    if (cmd_index > 0U)
                    {
                        App_ProcessCommand(cmd_buffer, cmd_index);
                        cmd_index = 0U;
                    }

                    App_SendPrompt();
                }
            }
            else
            {
                s_ignoreLf = 0U;

                if (cmd_index < UART_CMD_BUFFER_SIZE)
                {
                    cmd_buffer[cmd_index] = rx_char;
                    cmd_index++;
                }
                else
                {
                    cmd_index = 0U;
                    App_SendString("\r\nError: command too long.\r\n");
                    App_SendPrompt();
                }
            }
        }
    }

    return 0;
}
