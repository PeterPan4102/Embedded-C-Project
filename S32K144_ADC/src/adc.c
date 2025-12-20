/**
 * @file adc.c
 * @author Dung Van Pham (dungpv00@gmail.com)
 * @brief ADC driver implementation for S32K144
 * @version 0.1
 * @date 2025-12-20
 *
 * @copyright Copyright (c) 2025
 *
 */

/*==================================================================================================
* INCLUDES
==================================================================================================*/
#include "adc.h"
#include "S32K144.h"
#include "device_registers.h"

/*==================================================================================================
* DEFINES
==================================================================================================*/
/* PTA6 is used as ADC0 channel SE2 (ADC0_SE2) */
#define ADC0_INPUT_PORT_INDEX            (PCC_PORTA_INDEX)
#define ADC0_INPUT_PIN                   (6U)
#define ADC0_PCC_INDEX                   (PCC_ADC0_INDEX)

/* PCC clock source selection for ADC0:
 * PCS(1) is typically SOSCDIV2_CLK in many S32K examples (depends on clock tree config).
 */
#define ADC0_PCC_PCS_VALUE               (1U)

/* ADC resolution: 12-bit => max code = 4095 */
#define ADC0_MAX_CODE_12BIT              (4095.0F)

/*==================================================================================================
* LOCAL FUNCTION PROTOTYPES
==================================================================================================*/
static void ADC0_ClockInit(void);
static void ADC0_PinInit(void);
static void ADC0_CoreConfig(void);
static void ADC0_Calibration(void);

/*==================================================================================================
* FUNCTIONS
==================================================================================================*/

/**
 * @brief Initialize ADC0 peripheral:
 * - Enable clock for ADC0 and its input port.
 * - Configure input pin as analog.
 * - Configure core ADC0 registers for software-triggered conversion.
 * - Run calibration routine.
 */
static void ADC0_ClockInit(void)
{
    /* Enable clock for PORTA (PTA6 is ADC input). */
    IP_PCC->PCCn[ADC0_INPUT_PORT_INDEX] |= PCC_PCCn_CGC_MASK;

    /* Disable ADC0 clock before changing PCC fields (recommended sequence). */
    IP_PCC->PCCn[ADC0_PCC_INDEX] &= ~PCC_PCCn_CGC_MASK;

    /* Select ADC0 clock source and enable ADC0 clock. */
    IP_PCC->PCCn[ADC0_PCC_INDEX] =
            PCC_PCCn_PCS(ADC0_PCC_PCS_VALUE) | /* Select clock source for ADC0. */
            PCC_PCCn_CGC_MASK;                 /* Enable clock for ADC0. */
}

/**
 * @brief Configure ADC0 input pin (PTA6) as analog input.
 *
 */
static void ADC0_PinInit(void)
{
    /* Clear MUX field to 0 (analog). */
    IP_PORTA->PCR[ADC0_INPUT_PIN] &= ~PORT_PCR_MUX_MASK;

    /* No pull-up/pull-down configuration is required for a pure analog input. */
}

/**
 * @brief Configure core ADC0 registers for software-triggered conversion.
 *
 */
static void ADC0_CoreConfig(void)
{
    /* CFG1:
     * - ADICLK(0): input clock source (often "bus/alternate clock", device-specific)
     * - ADIV(1): divide by 2 to keep ADC clock in safe range
     * - MODE(1): 12-bit conversion
     */
    IP_ADC0->CFG1 =
        ADC_CFG1_ADICLK(0U) |
        ADC_CFG1_ADIV(1U)   |
        ADC_CFG1_MODE(1U);

    /* CFG2:
     * Default 0: selects default sample time / mux settings (device-specific).
     */
    IP_ADC0->CFG2 = 0U;

    /* SC2:
     * - REFSEL = 0: default voltage reference selection
     * - ADTRG  = 0: software trigger
     * All other bits left default (0).
     */
    IP_ADC0->SC2 = 0U;

    /* SC3:
     * Default 0: no continuous conversion, no average, no calibration request.
     * Will be overwritten during calibration routine.
     */
    IP_ADC0->SC3 = 0U;
}

/**
 * @brief Run ADC0 calibration routine.
 *
 */
static void ADC0_Calibration(void)
{
    /* Enable hardware average and select 32 samples to improve calibration accuracy. */
    IP_ADC0->SC3 =
        ADC_SC3_AVGE_MASK |  /* Enable averaging. */
        ADC_SC3_AVGS(3U);    /* 3 -> 32 samples. */

    /* Start calibration. */
    IP_ADC0->SC3 |= ADC_SC3_CAL_MASK;

    /* Wait for calibration to complete (CAL bit clears when done). */
    while ((IP_ADC0->SC3 & ADC_SC3_CAL_MASK) != 0U)
    {
        /* wait */
    }

    /* NOTE:
     * Many reference implementations also check CALF (calibration failed flag) here.
     * If CALF is set, calibration failed and results may be invalid.
     */
}

/**
 * @brief  Initialize ADC0 peripheral (clock, pin, core config, calibration).
 *
 */
void ADC0_Init(void)
{
    ADC0_ClockInit();
    ADC0_PinInit();
    ADC0_CoreConfig();
    ADC0_Calibration();
}

/**
 * @brief Read one ADC channel using polling method.
 *
 * @param ch    ADC channel number.
 * @return uint16_t Raw ADC conversion result.
 */
uint16_t ADC0_ReadChannel_Polling(uint8_t ch)
{
    /* Write channel to SC1[0]:
     * - ADCH(ch): select input channel
     * - AIEN = 0: disable interrupt (polling mode)
     * Writing SC1 starts a conversion when ADTRG = 0 (software trigger).
     */
    IP_ADC0->SC1[0] = ADC_SC1_ADCH(ch);

    /* Wait until conversion complete: COCO = 1. */
    while ((IP_ADC0->SC1[0] & ADC_SC1_COCO_MASK) == 0U)
    {
        /* wait */
    }

    /* Read conversion result from result register R[0]. */
    return (uint16_t)IP_ADC0->R[0];
}

/**
 * @brief Read ADC channel and convert raw result to voltage in mV.
 *
 * @param ch ADC channel number.
 * @param vref Reference voltage in the same unit you want returned (e.g., 5000 for mV).
 * @return float Voltage computed from raw code and vref.
 */
float ADC0_ReadVoltage_Polling(uint8_t ch, float vref)
{
    uint16_t raw = ADC0_ReadChannel_Polling(ch);

    /* 12-bit mode => full scale code = 4095.
     * voltage = raw / 4095 * vref
     */
    return (((float)raw) / ADC0_MAX_CODE_12BIT) * vref;
}
