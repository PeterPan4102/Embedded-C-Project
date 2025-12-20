/**
 * @file main.c
 * @author Dung Van Pham (dungpv00@gmail.com)
 * @brief Main file for LED blinking example on S32K144 using direct register access
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
#include "device_registers.h" /* include peripheral declarations S32K144 */

/*==================================================================================================
* DEFINES
==================================================================================================*/
#define LED_BLUE_PIN                 (0U)
#define LED_RED_PIN                  (15U)
#define LED_GREEN_PIN                (16U)

#define GPIO_PIN_MASK(pin)           (1UL << (pin))

/* NOTE:
 * This constant is only for a rough busy-wait delay.
 * The real delay depends on core clock and compiler optimization.
 */
#define DELAY_LOOP_PER_MS            (3000U)

/*==================================================================================================
* LOCAL FUNCTIONS
==================================================================================================*/
/**
 * @brief Busy-wait delay in milliseconds
 *
 * @param ms Delay time in milliseconds
 */
static void delay_ms(uint32_t ms)
{
    uint32_t i = 0U;

    /* Busy-wait delay:
     * Approx loops = ms * DELAY_LOOP_PER_MS.
     * NOP() helps keep the loop from being optimized away too aggressively.
     */
    for (i = 0U; i < (ms * DELAY_LOOP_PER_MS); i++)
    {
        NOP();
    }
}
/*==================================================================================================
* MAIN FUNCTION
==================================================================================================*/
int main(void)
{
    /* Enable clock gating for PORTD in PCC (Peripheral Clock Controller) */
    IP_PCC->PCCn[PCC_PORTD_INDEX] |= PCC_PCCn_CGC_MASK;

    /* Configure PORTD pin mux to GPIO mode (MUX = 1) */
    IP_PORTD->PCR[LED_BLUE_PIN]  = PORT_PCR_MUX(1U);  /* PTD0  as GPIO */
    IP_PORTD->PCR[LED_RED_PIN]   = PORT_PCR_MUX(1U);  /* PTD15 as GPIO */
    IP_PORTD->PCR[LED_GREEN_PIN] = PORT_PCR_MUX(1U);  /* PTD16 as GPIO */

    /* Configure GPIO direction as output (PDDR bit = 1 -> output) */
    IP_PTD->PDDR |= GPIO_PIN_MASK(LED_BLUE_PIN);
    IP_PTD->PDDR |= GPIO_PIN_MASK(LED_RED_PIN);
    IP_PTD->PDDR |= GPIO_PIN_MASK(LED_GREEN_PIN);

    while (1)
    {
        /* Turn ON RED LED (board LED is typically active-low):
         * - PCOR clears output bit when writing 1 to that bit position
         * - output becomes 0 -> LED ON
         */
        IP_PTD->PCOR = GPIO_PIN_MASK(LED_RED_PIN);
        delay_ms(3000U);

        /* Turn OFF RED LED:
         * - PSOR sets output bit when writing 1 to that bit position
         * - output becomes 1 -> LED OFF
         */
        IP_PTD->PSOR = GPIO_PIN_MASK(LED_RED_PIN);
        delay_ms(3000U);

        /* Turn ON GREEN LED (active-low) */
        IP_PTD->PCOR = GPIO_PIN_MASK(LED_GREEN_PIN);
        delay_ms(3000U);

        /* Turn OFF GREEN LED */
        IP_PTD->PSOR = GPIO_PIN_MASK(LED_GREEN_PIN);
        delay_ms(3000U);

        /* Turn ON BLUE LED (active-low) */
        IP_PTD->PCOR = GPIO_PIN_MASK(LED_BLUE_PIN);
        delay_ms(3000U);

        /* Turn OFF BLUE LED */
        IP_PTD->PSOR = GPIO_PIN_MASK(LED_BLUE_PIN);
        delay_ms(3000U);
    }
}

