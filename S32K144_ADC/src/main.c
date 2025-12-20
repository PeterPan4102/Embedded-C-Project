/**
 * @file main.c
 * @author Dung Van Pham (dungpv00@gmail.com)
 * @brief Main file for ADC voltage level LED indicator example on S32K144
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
#include "device_registers.h"            /* include peripheral declarations S32K144 */
#include "clocks_and_modes.h"
#include "adc.h"

/*==================================================================================================
* DEFINES
==================================================================================================*/
#define LED_RED                       (15U)
#define LED_BLUE                      (0U)
#define LED_GREEN                     (16U)

#define GPIO_PIN_MASK(pin)            (1UL << (pin))

/* LED on this board is active-low: output = 0 -> LED ON, output = 1 -> LED OFF. */
#define LED_ON(pin)                   do { IP_PTD->PCOR = GPIO_PIN_MASK(pin); } while (0)
#define LED_OFF(pin)                  do { IP_PTD->PSOR = GPIO_PIN_MASK(pin); } while (0)

/* Rough delay loop factor (must be calibrated if you need accurate ms). */
#define DELAY_LOOP_PER_MS             (3000U)

/* Voltage thresholds (unit: mV if ADC0_ReadVoltage_Polling returns mV). */
#define VMAX_MV                       (5000.0F)
#define THRESH_3_MV                   (3750.0F)
#define THRESH_2_MV                   (2500.0F)
#define THRESH_1_MV                   (1250.0F)



/*==================================================================================================
* FUNCTIONS
==================================================================================================*/
static void WDOG_disable(void)
{
    /* Unlock watchdog, set max timeout, then disable.
     * NOTE: Values are device-specific.
     */
    IP_WDOG->CNT   = 0xD928C520U;  /* Unlock watchdog. */
    IP_WDOG->TOVAL = 0x0000FFFFU;  /* Maximum timeout value. */
    IP_WDOG->CS    = 0x00002100U;  /* Disable watchdog. */
}

static void delay_ms(uint32_t ms)
{
    uint32_t i = 0U;

    /* Busy-wait delay: approximate only. */
    for (i = 0U; i < (ms * DELAY_LOOP_PER_MS); i++)
    {
        NOP(); /* No operation. */
    }
}

/*==================================================================================================
* MAIN FUNCTION
==================================================================================================*/
int main(void)
{
    float voltageMv = 0.0F;

    WDOG_disable();

    SOSC_init_8MHz();        /* Enable external 8 MHz crystal oscillator and SOSCDIV1/2. */
    SPLL_init_160MHz();      /* Use 8 MHz source, configure PLL to 160 MHz. */
    NormalRUNmode_80MHz();   /* Switch to RUN mode using PLL (core 80 MHz, bus 40 MHz). */

    /* Enable clock gating for PORTD in PCC (Peripheral Clock Controller). */
    IP_PCC->PCCn[PCC_PORTD_INDEX] |= PCC_PCCn_CGC_MASK;

    /* Configure PORTD pins as GPIO (MUX = 1). */
    IP_PORTD->PCR[LED_RED]   = PORT_PCR_MUX(1U); /* PTD15 as GPIO. */
    IP_PORTD->PCR[LED_BLUE]  = PORT_PCR_MUX(1U); /* PTD0  as GPIO. */
    IP_PORTD->PCR[LED_GREEN] = PORT_PCR_MUX(1U); /* PTD16 as GPIO. */

    /* Configure pins as outputs (PDDR bit = 1 -> output). */
    IP_PTD->PDDR |= GPIO_PIN_MASK(LED_RED);
    IP_PTD->PDDR |= GPIO_PIN_MASK(LED_BLUE);
    IP_PTD->PDDR |= GPIO_PIN_MASK(LED_GREEN);

    /* Default state: turn OFF all LEDs (active-low). */
    LED_OFF(LED_GREEN);
    LED_OFF(LED_BLUE);
    LED_OFF(LED_RED);

    /* Initialize ADC0 (implementation is inside adc.c). */
    ADC0_Init();

    for (;;)
    {
        /* Read ADC channel 2 and convert to voltage (expected unit: mV).
         * The second argument (5000) is commonly used as Vref (mV).
         */
        voltageMv = ADC0_ReadVoltage_Polling(2U, 5000U);

        /* Select LED based on voltage range.
         * Use else-if to guarantee only one branch is executed.
         */
        if ((voltageMv > THRESH_3_MV) && (voltageMv <= VMAX_MV))
        {
            LED_OFF(LED_GREEN);  /* Turn OFF GREEN. */
            LED_OFF(LED_BLUE);   /* Turn OFF BLUE.  */
            LED_ON(LED_RED);     /* Turn ON RED.    */
        }
        else if ((voltageMv > THRESH_2_MV) && (voltageMv <= THRESH_3_MV))
        {
            LED_ON(LED_GREEN);   /* Turn ON GREEN.  */
            LED_OFF(LED_BLUE);   /* Turn OFF BLUE.  */
            LED_OFF(LED_RED);    /* Turn OFF RED.   */
        }
        else if ((voltageMv > THRESH_1_MV) && (voltageMv <= THRESH_2_MV))
        {
            LED_OFF(LED_GREEN);  /* Turn OFF GREEN. */
            LED_ON(LED_BLUE);    /* Turn ON BLUE.   */
            LED_OFF(LED_RED);    /* Turn OFF RED.   */
        }
        else if ((voltageMv >= 0.0F) && (voltageMv <= THRESH_1_MV))
        {
            LED_OFF(LED_GREEN);  /* Turn OFF GREEN. */
            LED_ON(LED_BLUE);    /* Turn ON BLUE.   */
            LED_OFF(LED_RED);    /* Turn OFF RED.   */
        }
        else
        {
            /* Out-of-range safety: turn OFF all LEDs. */
            LED_OFF(LED_GREEN);
            LED_OFF(LED_BLUE);
            LED_OFF(LED_RED);
        }

        delay_ms(100U); /* Rough delay to reduce LED flicker / ADC polling rate. */
    }

    return 0;
}

