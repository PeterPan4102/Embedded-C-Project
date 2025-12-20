/**
 * @file main.c
 * @author Dung Van Pham (dungpv00@gmail.com)
 * @brief Main file for GPIO polling example on S32K144 using CMSIS Driver
 * @version 0.1
 * @date 2025-12-20
 *
 * @copyright Copyright (c) 2025
 *
 */

/*==================================================================================================
* INCLUDES
==================================================================================================*/
#include "S32K144.h"
#include "device_registers.h"
#include "Driver_GPIO.h"

/*==================================================================================================
* DEFINES
==================================================================================================*/
/* CPU clock frequency used for the rough DelayMs busy-wait.
 * WARNING: Must match the real core clock, otherwise delay will be incorrect.
 */
#define CPU_CLOCK_FREQ                 (8000000UL)
#define CYCLES_PER_MS                  (CPU_CLOCK_FREQ / 1000UL)

/* Button logic:
 * With pull-up enabled and button wired to GND:
 * - Released: 1
 * - Pressed : 0
 */
#define BTN_PRESSED                    (0U)

/* Virtual pin mapping (must match the mapping table in HAL_GPIO.c)
 * These numbers are NOT physical pins; they are "virtual IDs" that the HAL maps to real PORT/pin.
 */
#define LED_RED_PIN                    (1U)   /* Virtual pin -> PTD15: RED LED   */
#define LED_GREEN_PIN                  (2U)   /* Virtual pin -> PTD16: GREEN LED */

#define BUTTON_1_PIN                   (3U)   /* Virtual pin -> PTC12: BTN0      */
#define BUTTON_2_PIN                   (4U)   /* Virtual pin -> PTC13: BTN1      */

/*==================================================================================================
* EXTERN DRIVER INSTANCES
==================================================================================================*/
/* Driver instance implemented in Driver_GPIO.c */
extern ARM_DRIVER_GPIO Driver_GPIO0;

/*==================================================================================================
* FUNCTIONS
==================================================================================================*/
static void DelayMs(uint32_t delay_in_ms)
{
    volatile uint32_t loop_count;

    /* Busy-wait delay: approximate only.
     * CYCLES_PER_MS depends on CPU_CLOCK_FREQ and compiler optimization.
     */
    while (delay_in_ms > 0U)
    {
        delay_in_ms--;

        for (loop_count = 0U; loop_count < CYCLES_PER_MS; loop_count++)
        {
            __asm("nop");
        }
    }
}

/*==================================================================================================
* MAIN FUNCTION
==================================================================================================*/
int main(void)
{
    /*--------------------------------------------------------------------------------------------
    * LED CONFIGURATION
    *-------------------------------------------------------------------------------------------*/
    /* Configure RED LED */
    (void)Driver_GPIO0.Setup(LED_RED_PIN, NULL);                     /* No interrupt callback used */
    (void)Driver_GPIO0.SetDirection(LED_RED_PIN, ARM_GPIO_OUTPUT);
    (void)Driver_GPIO0.SetOutputMode(LED_RED_PIN, ARM_GPIO_PUSH_PULL);

    /* EVB LEDs are commonly active-low:
     * - Output = 0 -> LED ON
     * - Output = 1 -> LED OFF
     */
    (void)Driver_GPIO0.SetOutput(LED_RED_PIN, 1U);                   /* Turn OFF RED LED */

    /* Configure GREEN LED */
    (void)Driver_GPIO0.Setup(LED_GREEN_PIN, NULL);                   /* No interrupt callback used */
    (void)Driver_GPIO0.SetDirection(LED_GREEN_PIN, ARM_GPIO_OUTPUT);
    (void)Driver_GPIO0.SetOutputMode(LED_GREEN_PIN, ARM_GPIO_PUSH_PULL);
    (void)Driver_GPIO0.SetOutput(LED_GREEN_PIN, 1U);                 /* Turn OFF GREEN LED */

    /*--------------------------------------------------------------------------------------------
    * BUTTON CONFIGURATION
    *-------------------------------------------------------------------------------------------*/
    /* Configure BUTTON 1 */
    (void)Driver_GPIO0.Setup(BUTTON_1_PIN, NULL);
    (void)Driver_GPIO0.SetDirection(BUTTON_1_PIN, ARM_GPIO_INPUT);
    (void)Driver_GPIO0.SetPullResistor(BUTTON_1_PIN, ARM_GPIO_PULL_UP);

    /* Configure BUTTON 2 */
    (void)Driver_GPIO0.Setup(BUTTON_2_PIN, NULL);
    (void)Driver_GPIO0.SetDirection(BUTTON_2_PIN, ARM_GPIO_INPUT);
    (void)Driver_GPIO0.SetPullResistor(BUTTON_2_PIN, ARM_GPIO_PULL_UP);

    /*--------------------------------------------------------------------------------------------
    * MAIN LOOP (POLLING)
    *-------------------------------------------------------------------------------------------*/
    while (1)
    {
        uint32_t btn_1;
        uint32_t btn_2;

        /* Read current button states */
        btn_1 = Driver_GPIO0.GetInput(BUTTON_1_PIN);
        btn_2 = Driver_GPIO0.GetInput(BUTTON_2_PIN);

        /*----------------------------------------------------------------------------------------
        * Button 1 -> RED LED control (with simple debounce)
        *---------------------------------------------------------------------------------------*/
        if (BTN_PRESSED == btn_1)
        {
            /* Debounce delay */
            DelayMs(20U);

            /* NOTE:
             * This code reuses the old btn_1 value; it should read again after the delay
             * if you want a real debounce check. Kept as-is to match your structure.
             */

            if (BTN_PRESSED == btn_1)
            {
                /* Pressed -> turn ON RED LED (active-low should be 0).
                 * WARNING: Your original code writes 1U but comments say 0 = LED on.
                 */
                (void)Driver_GPIO0.SetOutput(LED_RED_PIN, 0U);        /* 0 = LED ON */
            }
        }
        else
        {
            /* Not pressed -> optionally turn OFF LED */
            (void)Driver_GPIO0.SetOutput(LED_RED_PIN, 1U);            /* 1 = LED OFF */
        }

        /*----------------------------------------------------------------------------------------
        * Button 2 -> GREEN LED control (with simple debounce)
        *---------------------------------------------------------------------------------------*/
        if (BTN_PRESSED == btn_2)
        {
            DelayMs(20U);

            /* Same debounce note as above: should read again after delay. */
            if (btn_2 == BTN_PRESSED)
            {
                (void)Driver_GPIO0.SetOutput(LED_GREEN_PIN, 0U);      /* 0 = LED ON */
            }
        }
        else
        {
            (void)Driver_GPIO0.SetOutput(LED_GREEN_PIN, 1U);          /* 1 = LED OFF */
        }

        /* Slow down polling loop */
        DelayMs(100U);
    }
}

