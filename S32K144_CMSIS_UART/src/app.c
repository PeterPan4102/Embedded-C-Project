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
#include "app.h"
#include "Driver_USART.h"
#include "Driver_GPIO.h"
#include <string.h>
#include <stdint.h>

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
 * VARIABLES
 =============================================================================*/

static uint8_t g_redLedOn   = 0U;
static uint8_t g_greenLedOn = 0U;
static uint8_t g_blueLedOn  = 0U;

/*=============================================================================
 * FUNCTION PROTOTYPES
 =============================================================================*/

static void App_SetRedLed(uint32_t on);
static void App_SetGreenLed(uint32_t on);
static void App_SetBlueLed(uint32_t on);
static uint32_t App_TrimTail(const uint8_t *cmd, uint32_t cmd_len);
static uint8_t  App_CmdEquals(const uint8_t *cmd, uint32_t cmd_len, const char *pattern, uint32_t pattern_len);

/*=============================================================================
 * FUNCTIONS
 =============================================================================*/

 /**
  * @brief Initialize LEDs
  *
  */
void App_InitLeds(void)
{
    ARM_DRIVER_GPIO *gpio;
    int32_t          status;

    gpio   = &Driver_GPIO0;
    status = ARM_DRIVER_OK;

    /* RED LED */
    if (ARM_DRIVER_OK == status)
    {
        status = gpio->Setup(LED_RED_PIN, NULL);
    }
    if (ARM_DRIVER_OK == status)
    {
        status = gpio->SetDirection(LED_RED_PIN, ARM_GPIO_OUTPUT);
    }
    if (ARM_DRIVER_OK == status)
    {
        status = gpio->SetOutputMode(LED_RED_PIN, ARM_GPIO_PUSH_PULL);
    }
    if (ARM_DRIVER_OK == status)
    {
        status = gpio->SetPullResistor(LED_RED_PIN, ARM_GPIO_PULL_NONE);
    }

    /* GREEN LED */
    if (ARM_DRIVER_OK == status)
    {
        status = gpio->Setup(LED_GREEN_PIN, NULL);
    }
    if (ARM_DRIVER_OK == status)
    {
        status = gpio->SetDirection(LED_GREEN_PIN, ARM_GPIO_OUTPUT);
    }
    if (ARM_DRIVER_OK == status)
    {
        status = gpio->SetOutputMode(LED_GREEN_PIN, ARM_GPIO_PUSH_PULL);
    }
    if (ARM_DRIVER_OK == status)
    {
        status = gpio->SetPullResistor(LED_GREEN_PIN, ARM_GPIO_PULL_NONE);
    }

    /* BLUE LED */
    if (ARM_DRIVER_OK == status)
    {
        status = gpio->Setup(LED_BLUE_PIN, NULL);
    }
    if (ARM_DRIVER_OK == status)
    {
        status = gpio->SetDirection(LED_BLUE_PIN, ARM_GPIO_OUTPUT);
    }
    if (ARM_DRIVER_OK == status)
    {
        status = gpio->SetOutputMode(LED_BLUE_PIN, ARM_GPIO_PUSH_PULL);
    }
    if (ARM_DRIVER_OK == status)
    {
        status = gpio->SetPullResistor(LED_BLUE_PIN, ARM_GPIO_PULL_NONE);
    }

    if (ARM_DRIVER_OK == status)
    {
        App_SetRedLed(0U);
        App_SetGreenLed(0U);
        App_SetBlueLed(0U);
    }
}

/**
 * @brief Set RED LED state
 *
 * @param on 1 to turn ON, 0 to turn OFF
 */
static void App_SetRedLed(uint32_t on)
{
    ARM_DRIVER_GPIO *gpio;
    uint32_t         level;

    gpio = &Driver_GPIO0;

    if (0U != on)
    {
        level      = LED_LEVEL_ON;
        g_redLedOn = 1U;
    }
    else
    {
        level      = LED_LEVEL_OFF;
        g_redLedOn = 0U;
    }

    (void)gpio->SetOutput(LED_RED_PIN, level);
}

/**
 * @brief Set GREEN LED state
 *
 * @param on 1 to turn ON, 0 to turn OFF
 */
static void App_SetGreenLed(uint32_t on)
{
    ARM_DRIVER_GPIO *gpio;
    uint32_t         level;

    gpio = &Driver_GPIO0;

    if (0U != on)
    {
        level        = LED_LEVEL_ON;
        g_greenLedOn = 1U;
    }
    else
    {
        level        = LED_LEVEL_OFF;
        g_greenLedOn = 0U;
    }

    (void)gpio->SetOutput(LED_GREEN_PIN, level);
}

/**
 * @brief Set BLUE LED state
 *
 * @param on 1 to turn ON, 0 to turn OFF
 */
static void App_SetBlueLed(uint32_t on)
{
    ARM_DRIVER_GPIO *gpio;
    uint32_t         level;

    gpio = &Driver_GPIO0;

    if (0U != on)
    {
        level       = LED_LEVEL_ON;
        g_blueLedOn = 1U;
    }
    else
    {
        level       = LED_LEVEL_OFF;
        g_blueLedOn = 0U;
    }

    (void)gpio->SetOutput(LED_BLUE_PIN, level);
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

    (void)status;
}

/**
 * @brief Send command prompt via USART
 *
 */
void App_SendPrompt(void)
{
    App_SendString("\r\n> ");
}

/**
 * @brief Trim trailing whitespace characters from command
 *
 * @param cmd Pointer to command buffer
 * @param cmd_len Length of command
 * @return uint32_t New length after trimming
 */
static uint32_t App_TrimTail(const uint8_t *cmd, uint32_t cmd_len)
{
    uint32_t len;

    len = cmd_len;

    if (NULL != cmd)
    {
        while (len > 0U)
        {
            uint8_t c;

            c = cmd[len - 1U];

            if ((c == (uint8_t)'\r') || (c == (uint8_t)'\n') || (c == (uint8_t)' '))
            {
                len--;
            }
            else
            {
                break;
            }
        }
    }
    else
    {
        len = 0U;
    }

    return len;
}

/**
 * @brief Check if command equals a pattern
 *
 * @param cmd Pointer to command buffer
 * @param cmd_len Length of command
 * @param pattern Pointer to pattern string
 * @param pattern_len Length of pattern
 * @return uint8_t 1 if equal, 0 otherwise
 */
static uint8_t App_CmdEquals(const uint8_t *cmd, uint32_t cmd_len, const char *pattern, uint32_t pattern_len)
{
    uint8_t matched;

    matched = 0U;

    if ((NULL != cmd) && (NULL != pattern))
    {
        if (cmd_len == pattern_len)
        {
            if (memcmp((const void *)cmd, (const void *)pattern, pattern_len) == 0)
            {
                matched = 1U;
            }
        }
    }

    return matched;
}

/**
 * @brief Process a command
 *
 * @param cmd Pointer to command buffer
 * @param cmd_len Length of command
 */
void App_ProcessCommand(const uint8_t *cmd, uint32_t cmd_len)
{
    int32_t  handled;
    uint32_t len;

    handled = 0;
    len     = App_TrimTail(cmd, cmd_len);

    /* LED STATUS */
    if (0U != App_CmdEquals(cmd, len, "LED STATUS", (uint32_t)(sizeof("LED STATUS") - 1U)))
    {
        App_SendString("\r\nRED: ");
        if (0U != g_redLedOn)
        {
            App_SendString("ON");
        }
        else
        {
            App_SendString("OFF");
        }

        App_SendString(", GREEN: ");
        if (0U != g_greenLedOn)
        {
            App_SendString("ON");
        }
        else
        {
            App_SendString("OFF");
        }

        App_SendString(", BLUE: ");
        if (0U != g_blueLedOn)
        {
            App_SendString("ON");
        }
        else
        {
            App_SendString("OFF");
        }

        handled = 1;
    }

    if (0 == handled)
    {
        if (0U != App_CmdEquals(cmd, len, "RED ON", (uint32_t)(sizeof("RED ON") - 1U)))
        {
            App_SetRedLed(1U);
            App_SendString("\r\nOK: RED ON");
            handled = 1;
        }
        else if (0U != App_CmdEquals(cmd, len, "RED OFF", (uint32_t)(sizeof("RED OFF") - 1U)))
        {
            App_SetRedLed(0U);
            App_SendString("\r\nOK: RED OFF");
            handled = 1;
        }
        else
        {
            /* Do nothing. */
        }
    }

    if (0 == handled)
    {
        if (0U != App_CmdEquals(cmd, len, "GREEN ON", (uint32_t)(sizeof("GREEN ON") - 1U)))
        {
            App_SetGreenLed(1U);
            App_SendString("\r\nOK: GREEN ON");
            handled = 1;
        }
        else if (0U != App_CmdEquals(cmd, len, "GREEN OFF", (uint32_t)(sizeof("GREEN OFF") - 1U)))
        {
            App_SetGreenLed(0U);
            App_SendString("\r\nOK: GREEN OFF");
            handled = 1;
        }
        else
        {
            /* Do nothing. */
        }
    }

    if (0 == handled)
    {
        if (0U != App_CmdEquals(cmd, len, "BLUE ON", (uint32_t)(sizeof("BLUE ON") - 1U)))
        {
            App_SetBlueLed(1U);
            App_SendString("\r\nOK: BLUE ON");
            handled = 1;
        }
        else if (0U != App_CmdEquals(cmd, len, "BLUE OFF", (uint32_t)(sizeof("BLUE OFF") - 1U)))
        {
            App_SetBlueLed(0U);
            App_SendString("\r\nOK: BLUE OFF");
            handled = 1;
        }
        else
        {
            /* Do nothing. */
        }
    }

    if (0 == handled)
    {
        if (0U != App_CmdEquals(cmd, len, "HELP", (uint32_t)(sizeof("HELP") - 1U)))
        {
            App_SendString("\r\nAvailable commands:\r\n");
            App_SendString("  LED STATUS\r\n");
            App_SendString("  RED ON   / RED OFF\r\n");
            App_SendString("  GREEN ON / GREEN OFF\r\n");
            App_SendString("  BLUE ON  / BLUE OFF\r\n");
            App_SendString("  HELP\r\n");
            handled = 1;
        }
    }

    if (0 == handled)
    {
        App_SendString("\r\nCommand not available");
    }
}



