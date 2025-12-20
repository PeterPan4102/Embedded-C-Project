/*==================================================================================================
* INCLUDES
==================================================================================================*/
#include "Timer_LPIT.h"
#include "device_registers.h"

/*==================================================================================================
* DEFINES
==================================================================================================*/
#define LPIT0_CLK_HZ          (40000000U)
#define LPIT0_CH0_PERIOD_HZ   (1000U)

#define LPIT0_PCC_PCS_VALUE   (6U)

/* Cortex-M priority bits implemented */
#define NVIC_PRIO_BITS        (4U)
#define LPIT0_IRQ_PRIORITY    (10U)

/*==================================================================================================
* STATIC FUNCTION PROTOTYPES
==================================================================================================*/
static void LPIT0_ClockInit(void);
static void LPIT0_ModuleInit(void);
static void LPIT0_Ch0_Init_1ms(void);
static void LPIT0_Ch0_IrqEnable(void);

/*==================================================================================================
* INTERRUPT HANDLER
==================================================================================================*/

/**
 * @brief LPIT0 Channel 0 Interrupt Handler
 *
 */
void LPIT0_Ch0_IRQHandler(void)
{
    /* LPIT0 Channel 0 interrupt: used as a periodic trigger to start ADC conversion. */
    if ((IP_LPIT0->MSR & LPIT_MSR_TIF0_MASK) != 0U)
    {
        /* Clear timeout flag (write-1-to-clear). */
        IP_LPIT0->MSR = LPIT_MSR_TIF0_MASK;

        /* Trigger one ADC conversion by software. */
        ADC0_StartConversion_SwTrigger();
    }
}

/*==================================================================================================
* STATIC FUNCTIONS
==================================================================================================*/

/**
 * @brief Initialize LPIT0 clock source and enable its clock.
 *
 */
static void LPIT0_ClockInit(void)
{
    uint32_t reg;

    /* Disable interface clock before changing PCS */
    reg  = IP_PCC->PCCn[PCC_LPIT_INDEX];
    reg &= ~PCC_PCCn_CGC_MASK;
    IP_PCC->PCCn[PCC_LPIT_INDEX] = reg;

    reg &= ~PCC_PCCn_PCS_MASK;
    reg |= PCC_PCCn_PCS(LPIT0_PCC_PCS_VALUE);
    reg |= PCC_PCCn_CGC_MASK;
    IP_PCC->PCCn[PCC_LPIT_INDEX] = reg;
}

/**
 * @brief Initialize LPIT0 module.
 *
 */
static void LPIT0_ModuleInit(void)
{
    uint32_t i;

    /* Enable LPIT module clock inside LPIT */
    IP_LPIT0->MCR |= LPIT_MCR_M_CEN_MASK;

    /* Wait >= 4 peripheral clock cycles for synchronization */
    for (i = 0U; i < 4U; i++)
    {
        NOP();
    }
}

/**
 * @brief Initialize LPIT0 Channel 0 for 1 ms periodic interrupts.
 *
 */
static void LPIT0_Ch0_Init_1ms(void)
{
    uint32_t timerTicks;

    timerTicks = (LPIT0_CLK_HZ / LPIT0_CH0_PERIOD_HZ) - 1U;

    /* Disable channel before configuring */
    IP_LPIT0->TMR[0].TCTRL = 0U;

    /* Clear any pending timeout flag */
    IP_LPIT0->MSR = LPIT_MSR_TIF0_MASK;

    /* Load timeout value */
    IP_LPIT0->TMR[0].TVAL = timerTicks;

    /* Enable interrupt generation for channel 0 inside LPIT */
    IP_LPIT0->MIER |= LPIT_MIER_TIE0_MASK;

    /* Enable channel 0 */
    IP_LPIT0->TMR[0].TCTRL |= LPIT_TMR_TCTRL_T_EN_MASK;
}

/**
 * @brief Enable LPIT0 Channel 0 interrupt in NVIC.
 *
 */
static void LPIT0_Ch0_IrqEnable(void)
{
    uint32_t irqn;

    irqn = (uint32_t)LPIT0_Ch0_IRQn;

    /* Clear pending */
    MY_NVIC->ICPR[irqn / 32U] = (1UL << (irqn % 32U));

    /* Enable */
    MY_NVIC->ISER[irqn / 32U] = (1UL << (irqn % 32U));

    /* Priority */
    MY_NVIC->IP[irqn] = (uint8_t)((uint32_t)LPIT0_IRQ_PRIORITY << (8U - NVIC_PRIO_BITS));
}

/*==================================================================================================
* PUBLIC FUNCTIONS
==================================================================================================*/

/**
 * @brief Initialize LPIT0 module and Channel 0 for 1 ms periodic interrupts.
 *
 */
void LPIT0_Init(void)
{
    LPIT0_ClockInit();
    LPIT0_ModuleInit();
    LPIT0_Ch0_IrqEnable();
    LPIT0_Ch0_Init_1ms();
}

/**
 * @brief Start LPIT0 Channel 0 timer.
 *
 */
void LPIT0_Ch0_Start(void)
{
    IP_LPIT0->MSR = LPIT_MSR_TIF0_MASK;
    IP_LPIT0->TMR[0].TCTRL |= LPIT_TMR_TCTRL_T_EN_MASK;
}

/**
 * @brief Stop LPIT0 Channel 0 timer.
 *
 */
void LPIT0_Ch0_Stop(void)
{
    IP_LPIT0->TMR[0].TCTRL &= ~LPIT_TMR_TCTRL_T_EN_MASK;
}
