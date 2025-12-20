/**
 * @file main.c
 * @author Dung Van Pham (dungpv00@gmail.com)
 * @brief Main file for ADC with LPIT trigger example on S32K144 using CMSIS Driver
 * @version 0.1
 * @date 2025-12-20
 *
 * @copyright Copyright (c) 2025
 *
 */

/*==================================================================================================
* INCLUDES
==================================================================================================*/
#include "device_registers.h"
#include "clocks_and_modes.h"
#include "adc.h"
#include "Timer_LPIT.h"

/*==================================================================================================
* DEFINES
==================================================================================================*/
#define LED_RED_PIN                 (15U)
#define LED_BLUE_PIN                (0U)
#define LED_GREEN_PIN               (16U)

#define LED_RED_MASK                (1UL << LED_RED_PIN)
#define LED_BLUE_MASK               (1UL << LED_BLUE_PIN)
#define LED_GREEN_MASK              (1UL << LED_GREEN_PIN)
#define LED_ALL_MASK                (LED_RED_MASK | LED_BLUE_MASK | LED_GREEN_MASK)

/* ADC conversion scaling (adjust ADC_REF_MV_F to your board VDDA/VREF)
 * ADC 12-bit => code range 0..4095 (2^12 - 1). Used to scale ADC counts to voltage. */
#define ADC_MAX_COUNT_F             (4095.0f)
#define ADC_REF_MV_F                (5000.0f)

/* Voltage thresholds in mV */
#define THRESH_RED_MV_F             (3750.0f)
#define THRESH_GREEN_MV_F           (2500.0f)
#define THRESH_BLUE_MV_F            (1250.0f)

/*==================================================================================================
* PROTOTYPES
==================================================================================================*/
static void WDOG_Disable(void);
static void App_InitSystemClock(void);
static void App_InitLeds(void);
static void App_AllLedsOff(void);
static void App_SetLedRedOnly(void);
static void App_SetLedGreenOnly(void);
static void App_SetLedBlueOnly(void);
static void App_UpdateLeds(float vol_mv);

/*==================================================================================================
* FUNCTIONS
==================================================================================================*/

/**
 * @brief Disable the watchdog timer.
 *
 */
void WDOG_Disable(void)
{
    /* Disable watchdog (development use):
	 *  - Unlock watchdog
	 *  - Maximum timeout value
	 *  - Disable watchdog */
    IP_WDOG->CNT   = 0xD928C520U;
    IP_WDOG->TOVAL = 0x0000FFFFU;
    IP_WDOG->CS    = 0x00002100U;
}

/**
 * @brief Initialize system clock to 80 MHz and necessary modes.
 *
 */
void App_InitSystemClock(void)
{
    SOSC_init_8MHz();
    SPLL_init_160MHz();
    NormalRUNmode_80MHz();
}

/**
 * @brief Initialize LED GPIOs.
 *
 */
void App_InitLeds(void)
{
    /* Enable clock for PORTD (PCR access). */
    IP_PCC->PCCn[PCC_PORTD_INDEX] |= PCC_PCCn_CGC_MASK;

    /* Configure PTD pins as GPIO (MUX = 1). */
    IP_PORTD->PCR[LED_RED_PIN]   = PORT_PCR_MUX(1U);
    IP_PORTD->PCR[LED_BLUE_PIN]  = PORT_PCR_MUX(1U);
    IP_PORTD->PCR[LED_GREEN_PIN] = PORT_PCR_MUX(1U);

    /* Configure PTD pins as outputs. */
    IP_PTD->PDDR |= LED_ALL_MASK;

    /* LEDs are active-low on typical S32K144 EVB: set output high to turn OFF. */
    App_AllLedsOff();
}

/**
 * @brief Turn OFF all LEDs.
 *
 */
void App_AllLedsOff(void)
{
    /* Active-low: set output high => LED OFF. */
    IP_PTD->PSOR = LED_ALL_MASK;
}

/**
 * @brief Turn ON only the RED LED.
 *
 */
void App_SetLedRedOnly(void)
{
	/* Active-low: clear output => LED ON. */
    App_AllLedsOff();
    IP_PTD->PCOR = LED_RED_MASK;
}

/**
 * @brief Turn ON only the GREEN LED.
 *
 */
void App_SetLedGreenOnly(void)
{
    App_AllLedsOff();
    IP_PTD->PCOR = LED_GREEN_MASK;
}

/**
 * @brief Turn ON only the BLUE LED.
 *
 */
void App_SetLedBlueOnly(void)
{
    App_AllLedsOff();
    IP_PTD->PCOR = LED_BLUE_MASK;
}

/**
 * @brief Update LEDs based on measured voltage.
 *
 */
void App_UpdateLeds(float vol_mv)
{
    /* Update LED state based on measured voltage thresholds. */
    if ((vol_mv > THRESH_RED_MV_F) && (vol_mv <= ADC_REF_MV_F))
    {
        App_SetLedRedOnly();
    }
    else if ((vol_mv > THRESH_GREEN_MV_F) && (vol_mv <= THRESH_RED_MV_F))
    {
        App_SetLedGreenOnly();
    }
    else if ((vol_mv > THRESH_BLUE_MV_F) && (vol_mv <= THRESH_GREEN_MV_F))
    {
        App_SetLedBlueOnly();
    }
    else
    {
        /* vol_mv <= THRESH_BLUE_MV_F (or out of expected range): all LEDs OFF. */
        App_AllLedsOff();
    }
}

/*==================================================================================================
* MAIN FUNCTION
==================================================================================================*/
int main(void)
{
    float vol_mv = 0.0f;

    WDOG_Disable();
    App_InitSystemClock();
    App_InitLeds();

    /* ADC0 is configured for software trigger. Result/flag are updated in ADC0_IRQHandler().
     * Note: g_adcResult and g_adcNew are global (extern) to share data between ISR and main loop.
     */
    ADC0_Init();

    /* LPIT0 must be configured to generate periodic IRQ (CH0) for sampling. */
    LPIT0_Init();

    while (1)
    {
        if (g_adcNew != 0U)
        {
            g_adcNew = 0U;

            /* Convert ADC counts to millivolts. */
            vol_mv = ((float)g_adcResult / ADC_MAX_COUNT_F) * ADC_REF_MV_F;

            /* Update LEDs based on voltage. */
            App_UpdateLeds(vol_mv);
        }
    }

    /* Unreachable (kept to satisfy single-exit rule). */
    return 0;
}
