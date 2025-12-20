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
#include "device_registers.h"
#include "S32K144.h"

/*==================================================================================================
* DEFINES
==================================================================================================*/
#define ADC0_PORTA_PIN_INDEX     (6U)   /* PTA6 */
#define ADC0_CHANNEL_SE          (2U)   /* Your design: PTA6 -> ADC0 channel 2 */
#define ADC0_IRQ_PRIORITY        (10U)  /* Priority level: 0..15 (implemented in MSBs) */
#define NVIC_PRIO_BITS           (4U)   /* S32K144: 16 priority levels */

/*==================================================================================================
* GLOBAL VARIABLES
==================================================================================================*/
volatile uint16_t g_adcResult = 0U;
volatile uint32_t g_adcNew    = 0U;

/*==================================================================================================
* STATIC VARIABLES
==================================================================================================*/
static uint8_t s_adcChannel = (uint8_t)ADC0_CHANNEL_SE;

/*==================================================================================================
* STATIC FUNCTION PROTOTYPES
==================================================================================================*/
static void ADC0_ClockInit(void);
static void ADC0_PinInit(void);
static void ADC0_CoreConfig_SwTrigger(void);
static void ADC0_Calibration(void);
static void ADC0_IrqEnable(void);

/*==================================================================================================
* INTERRUPT HANDLER
==================================================================================================*/
void ADC0_IRQHandler(void)
{
    if ((IP_ADC0->SC1[0] & ADC_SC1_COCO_MASK) != 0U)
    {
        g_adcResult = (uint16_t)IP_ADC0->R[0];
        g_adcNew    = 1U;
    }
}

/*==================================================================================================
* STATIC FUNCTIONS
==================================================================================================*/

/**
 * @brief Initialize ADC0 clock in PCC and enable it.
 *
 */
static void ADC0_ClockInit(void)
{
    uint32_t reg;

    /* Enable PORTA clock for PCR access */
    IP_PCC->PCCn[PCC_PORTA_INDEX] |= PCC_PCCn_CGC_MASK;

    /* Disable ADC0 interface clock before configuring PCS */
    reg  = IP_PCC->PCCn[PCC_ADC0_INDEX];
    reg &= ~PCC_PCCn_CGC_MASK;
    IP_PCC->PCCn[PCC_ADC0_INDEX] = reg;

    /* Configure PCS then enable CGC (adjust PCS value to your clock tree if needed) */
    reg &= ~PCC_PCCn_PCS_MASK;
    reg |= PCC_PCCn_PCS(1U);
    reg |= PCC_PCCn_CGC_MASK;
    IP_PCC->PCCn[PCC_ADC0_INDEX] = reg;
}

/**
 * @brief Initialize ADC0 input pin (PTA6) as analog.
 *
 */
static void ADC0_PinInit(void)
{
    /* Analog pin: default PCR (MUX=0, no pulls, no interrupts) */
    IP_PORTA->PCR[ADC0_PORTA_PIN_INDEX] = 0U;
}

/**
 * @brief Configure core ADC0 registers for software-triggered conversion.
 *
 */
static void ADC0_CoreConfig_SwTrigger(void)
{
    /* CFG1: clock select / divider / resolution */
    IP_ADC0->CFG1 =
        ADC_CFG1_ADICLK(0U) |
        ADC_CFG1_ADIV(1U)   |
        ADC_CFG1_MODE(1U);  /* 12-bit */

    IP_ADC0->CFG2 = ADC_CFG2_SMPLTS(12U); /* sample time 13 ADCK */

    /* Software trigger: ADTRG = 0 */
    IP_ADC0->SC2 &= ~ADC_SC2_ADTRG_MASK;

    /* No average by default; calibration will enable temporarily */
    IP_ADC0->SC3 = 0U;

    /* Disable conversions initially by selecting ADCH=31 (module-specific: "disabled") */
    IP_ADC0->SC1[0] = ADC_SC1_ADCH(31U);
}

/**
 * @brief Run ADC0 calibration routine.
 *
 */
static void ADC0_Calibration(void)
{
    /* Enable HW average 32 samples during calibration */
    IP_ADC0->SC3 = ADC_SC3_AVGE_MASK | ADC_SC3_AVGS(3U);

    /* Start calibration */
    IP_ADC0->SC3 |= ADC_SC3_CAL_MASK;

    while ((IP_ADC0->SC3 & ADC_SC3_CAL_MASK) != 0U)
    {
        /* wait */
    }

    /* Disable average after calibration for faster runtime conversions */
    IP_ADC0->SC3 = 0U;
}

/**
 * @brief Enable ADC0 interrupt in NVIC with desired priority.
 *
 */
static void ADC0_IrqEnable(void)
{
    uint32_t irqn;

    irqn = (uint32_t)ADC0_IRQn;

    /* Clear pending */
    MY_NVIC->ICPR[irqn / 32U] = (1UL << (irqn % 32U));

    /* Enable */
    MY_NVIC->ISER[irqn / 32U] = (1UL << (irqn % 32U));

    /* Priority: shift into MSBs */
    MY_NVIC->IP[irqn] = (uint8_t)((uint32_t)ADC0_IRQ_PRIORITY << (8U - NVIC_PRIO_BITS));
}

/*==================================================================================================
* PUBLIC FUNCTIONS
==================================================================================================*/

/**
 * @brief Start one conversion by software trigger. Call this from LPIT0_Ch0_IRQHandler().
 *
 */
void ADC0_StartConversion_SwTrigger(void)
{
    /* Start conversion by writing SC1[0]. AIEN=1 => IRQ on COCO. */
    IP_ADC0->SC1[0] = ADC_SC1_ADCH((uint32_t)s_adcChannel) | ADC_SC1_AIEN_MASK;
}

/**
 * @brief Initialize ADC0 peripheral:
 * - Enable clock for ADC0 and its input port.
 * - Configure input pin as analog.
 * - Configure core ADC0 registers for software-triggered conversion.
 * - Run calibration routine.
 *
 */
void ADC0_Init(void)
{
    ADC0_ClockInit();
    ADC0_PinInit();
    ADC0_CoreConfig_SwTrigger();
    ADC0_Calibration();
    ADC0_IrqEnable();

    /* Ensure channel selection is set for runtime starts */
    s_adcChannel = (uint8_t)ADC0_CHANNEL_SE;

    /* Do not start conversion here; LPIT IRQ will call ADC0_StartConversion_SwTrigger(). */
}
