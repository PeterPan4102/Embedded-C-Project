/**
 * @file HAL_USART.c
 * @author Dung Van Pham (dungpv00@gmail.com)
 * @brief HAL USART driver implementation for S32K144
 * @version 0.1
 * @date 2025-12-20
 *
 * @copyright Copyright (c) 2025
 *
 */

/*==================================================================================================
* INCLUDES
==================================================================================================*/
#include "incl/HAL_USART.h"
#include "incl/NVIC.h"
#include "S32K144.h"

/*==================================================================================================
* DEFINES
==================================================================================================*/
#define CLEAR_LPUART_STAT    (0xFFFFFFFFUL)
#define HAL_UART_RX_TIMEOUT_LOOP   (1000000U)
#define LPUART1_IRQ_PRIORITY        (5U)  /* Priority level: 0..15 (implemented in MSBs) */

/*==================================================================================================
* VARIABLES
==================================================================================================*/
static HAL_UART_Context_t      s_uart1_ctx;
static HAL_UART_PowerState_t   s_uartPowerState = HAL_UART_POWER_OFF;
static volatile uint32_t       s_usartTxCount   = 0U;
static volatile uint32_t       s_usartRxCount   = 0U;
static ARM_USART_STATUS        s_usartStatus;

static const uint8_t *s_txBuffer;
static uint32_t s_txLen;
static uint32_t s_txPos;

static uint8_t *s_rxBuffer;
static uint32_t s_rxLen;
static uint32_t s_rxPos;

/*==================================================================================================
* API FUNCTIONS
==================================================================================================*/
void LPUART1_RxTx_IRQHandler(void)
{
    uint32_t status = IP_LPUART1->STAT;

    /* Handle Tx interrupt */
    if((status & LPUART_STAT_TDRE_MASK) && (IP_LPUART1->CTRL & LPUART_CTRL_TIE_MASK))
    {
        if(0U != s_usartStatus.tx_busy)
        {
            if(s_txPos < s_txLen)
            {
                IP_LPUART1->DATA = s_txBuffer[s_txPos];
                s_txPos++;
                s_usartTxCount ++;
            }
            else
            {
                IP_LPUART1->CTRL &= (uint32_t)~LPUART_CTRL_TIE_MASK;
                s_usartStatus.tx_busy = 0U;
                 if (0U != s_uart1_ctx.cb_event)
                 {
                     s_uart1_ctx.cb_event(ARM_USART_EVENT_SEND_COMPLETE);
                 }
            }
        }
        else
        {
            /* Should not be here */
            IP_LPUART1->CTRL &= (uint32_t)~LPUART_CTRL_TIE_MASK;
        }
    }


    /* Handle Rx interrupt */
    if(0U != (status & LPUART_STAT_RDRF_MASK))
    {
        uint8_t c = (uint8_t)(IP_LPUART1->DATA & 0xFFU);

        if (0U != s_usartStatus.rx_busy)
        {
            s_rxBuffer[s_rxPos] = c;
            s_rxPos++;
            s_usartRxCount++;

            if(s_rxPos >= s_rxLen)
            {
                s_usartStatus.rx_busy = 0U;
                if (0U != s_uart1_ctx.cb_event)
                {
                    s_uart1_ctx.cb_event(ARM_USART_EVENT_RECEIVE_COMPLETE);
                }
            }
            else
            {
                /* Continue receiving */
            }
        }
        else
        {
            /* Discard data if not in receiving state */
        }
    }
}

void LPUART1_IrqEnable(void)
{
    uint32_t irqn;

    irqn = (uint32_t) LPUART1_RxTx_IRQn;

    /* Clear pending */
    MY_NVIC->ICPR[irqn / 32U] = (1UL << (irqn % 32U));

    /* Enable */
    MY_NVIC->ISER[irqn / 32U] = (1UL << (irqn % 32U));

    /* Priority: shift into MSBs */
    MY_NVIC->IP[irqn] = (uint8_t)((uint32_t)LPUART1_IRQ_PRIORITY << (8U-__NVIC_PRIO_BITS));
}

/**
 * @brief Initialize USART driver
 *
 * @param cb_event Callback function for USART events
 * @return int32_t ARM_DRIVER_OK on success, error code otherwise
 */
int32_t HAL_ARM_USART_Initialize(ARM_USART_SignalEvent_t cb_event)
{
    int32_t result = ARM_DRIVER_OK;

    if (0U != s_uart1_ctx.initialized)
    {
        s_uart1_ctx.cb_event = cb_event;
    }
    else
    {
        s_uart1_ctx.cb_event    = cb_event;
        s_uart1_ctx.initialized = 1U;

        s_uartPowerState = HAL_UART_POWER_OFF;
        s_usartTxCount   = 0U;
        s_usartRxCount   = 0U;

        /* Clear status struct */
        s_usartStatus.tx_busy           = 0U;
        s_usartStatus.rx_busy           = 0U;
        s_usartStatus.tx_underflow      = 0U;
        s_usartStatus.rx_overflow       = 0U;
        s_usartStatus.rx_break          = 0U;
        s_usartStatus.rx_framing_error  = 0U;
        s_usartStatus.rx_parity_error   = 0U;

        /* Enable clock for PORTC */
        /* Port C6: MUX = ALT2, LPUART1 TX */
        /* Port C7: MUX = ALT2, LPUART1 RX */
        IP_PCC->PCCn[PCC_PORTC_INDEX] |= PCC_PCCn_CGC_MASK;
        IP_PORTC->PCR[6U]             |= PORT_PCR_MUX(2U);
        IP_PORTC->PCR[7U]             |= PORT_PCR_MUX(2U);

        /* Clock Source = 1 (SOSCDIV2_CLK) */
        /* Enable clock for LPUART1 registers */
        IP_PCC->PCCn[PCC_LPUART1_INDEX] &= (uint32_t)~PCC_PCCn_CGC_MASK;
        IP_PCC->PCCn[PCC_LPUART1_INDEX] |=  PCC_PCCn_PCS(0x1U)
                                        |   PCC_PCCn_CGC_MASK;

        /* Disable transmitter and receiver before configuration */
        IP_LPUART1->CTRL &= (uint32_t)~(LPUART_CTRL_TE_MASK | LPUART_CTRL_RE_MASK);

        /* Clear baud configuration */
        IP_LPUART1->BAUD  = 0U;
    }

    return result;
}

/**
 * @brief Uninitialize USART driver
 *
 * @return int32_t ARM_DRIVER_OK on success, error code otherwise
 */
int32_t HAL_ARM_USART_Uninitialize(void)
{
    int32_t result = ARM_DRIVER_OK;

    (void)HAL_ARM_UART_PowerControl(HAL_UART_POWER_OFF);

    s_uart1_ctx.initialized = 0U;
    s_uart1_ctx.cb_event    = NULL;

    s_usartTxCount = 0U;
    s_usartRxCount = 0U;

    s_usartStatus.tx_busy           = 0U;
    s_usartStatus.rx_busy           = 0U;
    s_usartStatus.tx_underflow      = 0U;
    s_usartStatus.rx_overflow       = 0U;
    s_usartStatus.rx_break          = 0U;
    s_usartStatus.rx_framing_error  = 0U;
    s_usartStatus.rx_parity_error   = 0U;

    return result;
}

/**
 * @brief Control USART settings
 *
 * @param control Control parameter
 * @param arg Argument for control parameter
 * @return int32_t ARM_DRIVER_OK on success, error code otherwise
 */
int32_t HAL_ARM_USART_Control(uint32_t control, uint32_t arg)
{
    int32_t  result    = ARM_DRIVER_OK;
    uint32_t mode      = 0U;
    uint32_t data_bits = 0U;
    uint32_t parity    = 0U;
    uint32_t stop_bits = 0U;
    uint32_t baudrate  = 0U;

    baudrate = arg;

    if (0U == s_uart1_ctx.initialized)
    {
        result = ARM_DRIVER_ERROR;
    }
    else if (HAL_UART_POWER_FULL != s_uartPowerState)
    {
        result = ARM_DRIVER_ERROR;
    }
    else
    {
        mode = control & ARM_USART_CONTROL_Msk;

        switch (mode)
        {
            case ARM_USART_MODE_ASYNCHRONOUS:
            {
                switch(baudrate)
                {
                    case 1200U:
                    case 2400U:
                    case 4800U:
                    case 9600U:
                        /* Supported baud rates */
                        /* OSR = 15 (16x oversampling), SBR = 52 => 8MHz / (16 * 52) ~ 9615 bps */
                        IP_LPUART1->BAUD = 0x0F000034UL; //0x0F000034
                        break;
                    case 19200U:
                        /* 8MHz / (16 * 26) ≈ 19230 */
                        IP_LPUART1->BAUD = 0x0F00001AUL; /* OSR=15, SBR=26 */
                        break;
                    case 38400U:
                        /* 8MHz / (16 * 13) ≈ 38461 */
                        IP_LPUART1->BAUD = 0x0F00000DUL; /* OSR=15, SBR=13 */
                        break;
                    case 57600U:
                        /* 8MHz / (16 * 9) ≈ 55555 (−3.5%) */
                        IP_LPUART1->BAUD = 0x0F000009UL; /* OSR=15, SBR=9 */
                        break;
                    case 115200U:
                        /* Supported baud rates */
                        /* 8MHz, OSR=22 (23x), SBR=3  => ~115942 bps (~+0.644%) */
                        IP_LPUART1->BAUD = 0x16000003UL; //0x0F00000A
                        break;

                    default:
                        result = ARM_USART_ERROR_BAUDRATE;
                        break;
                }

                data_bits = control & ARM_USART_DATA_BITS_Msk;
                parity    = control & ARM_USART_PARITY_Msk;
                stop_bits = control & ARM_USART_STOP_BITS_Msk;

                /* Data bits */
                if (ARM_USART_DATA_BITS_8 == data_bits)
                {
                	IP_LPUART1->CTRL &= (uint32_t)~LPUART_CTRL_M_MASK;
                }
                else if (ARM_USART_DATA_BITS_9 == data_bits)
                {
                	IP_LPUART1->CTRL |= LPUART_CTRL_M_MASK;
                }
                else
                {
                	result = ARM_DRIVER_ERROR_UNSUPPORTED;
                }

                /* Parity */
                if (ARM_DRIVER_OK == result)
                {
                    if (ARM_USART_PARITY_NONE == parity)
                    {
                        IP_LPUART1->CTRL &= (uint32_t)~LPUART_CTRL_PE_MASK;
                    }
                    else if (ARM_USART_PARITY_EVEN == parity)
                    {
                        IP_LPUART1->CTRL |=  LPUART_CTRL_PE_MASK;
                        IP_LPUART1->CTRL &= (uint32_t)~LPUART_CTRL_PT_MASK;
                    }
                    else if (ARM_USART_PARITY_ODD == parity)
                    {
                        IP_LPUART1->CTRL |= (LPUART_CTRL_PE_MASK | LPUART_CTRL_PT_MASK);
                    }
                    else
                    {
                        result = ARM_DRIVER_ERROR_UNSUPPORTED;
                    }
                }

                /* Stop bits */
                if (ARM_DRIVER_OK == result)
                {
                    if (ARM_USART_STOP_BITS_1 == stop_bits)
                    {
                        IP_LPUART1->BAUD &= (uint32_t)~LPUART_BAUD_SBNS_MASK;
                    }
                    else if (ARM_USART_STOP_BITS_2 == stop_bits)
                    {
                        IP_LPUART1->BAUD |= LPUART_BAUD_SBNS_MASK;
                    }
                    else
                    {
                        result = ARM_DRIVER_ERROR_UNSUPPORTED;
                    }
                }
            }
            break;

            case ARM_USART_CONTROL_TX:
            {
                if (0U != arg)
                {
                    IP_LPUART1->CTRL |= LPUART_CTRL_TE_MASK;
                }
                else
                {
                    IP_LPUART1->CTRL &= (uint32_t)~LPUART_CTRL_TE_MASK;
                }
            }
            break;

            case ARM_USART_CONTROL_RX:
            {
                if (0U != arg)
                {
                    IP_LPUART1->CTRL |= LPUART_CTRL_RE_MASK;
                    IP_LPUART1->CTRL |= LPUART_CTRL_RIE_MASK;
                }
                else
                {
                    IP_LPUART1->CTRL &= (uint32_t)~LPUART_CTRL_RE_MASK;
                }
            }
            break;

            default:
            {
                result = ARM_DRIVER_ERROR_UNSUPPORTED;
            }
            break;
        }
    }

    return result;
}

/**
 * @brief Control USART power state
 *
 * @param power_state Desired power state
 * @return int32_t ARM_DRIVER_OK on success, error code otherwise
 */
int32_t HAL_ARM_UART_PowerControl(HAL_UART_PowerState_t power_state)
{
    int32_t result = ARM_DRIVER_OK;

    switch (power_state)
    {
        case HAL_UART_POWER_OFF:
        {
            IP_LPUART1->CTRL &=
                    (uint32_t)~(LPUART_CTRL_TE_MASK   |
                                LPUART_CTRL_RE_MASK   |
                                LPUART_CTRL_TIE_MASK  |
                                LPUART_CTRL_TCIE_MASK |
                                LPUART_CTRL_RIE_MASK  |
                                LPUART_CTRL_ILIE_MASK);

            IP_LPUART1->STAT = CLEAR_LPUART_STAT;


            IP_PCC->PCCn[PCC_LPUART1_INDEX] &= (uint32_t)~PCC_PCCn_CGC_MASK;

            s_uartPowerState = HAL_UART_POWER_OFF;
        }
        break;

        case HAL_UART_POWER_LOW:
        {
            IP_PCC->PCCn[PCC_LPUART1_INDEX] |= PCC_PCCn_CGC_MASK;

            IP_LPUART1->CTRL &=
                    (uint32_t)~(LPUART_CTRL_TE_MASK   |
                                LPUART_CTRL_RE_MASK   |
                                LPUART_CTRL_TIE_MASK  |
                                LPUART_CTRL_TCIE_MASK |
                                LPUART_CTRL_RIE_MASK  |
                                LPUART_CTRL_ILIE_MASK);


            s_uartPowerState = HAL_UART_POWER_LOW;
        }
        break;

        case HAL_UART_POWER_FULL:
        {
            IP_PCC->PCCn[PCC_LPUART1_INDEX] |= PCC_PCCn_CGC_MASK;

            LPUART1_IrqEnable();

            s_uartPowerState = HAL_UART_POWER_FULL;
        }
        break;

        default:
        {
            result = ARM_DRIVER_ERROR_PARAMETER;
        }
        break;
    }

    return result;
}


/**
 * @brief Send data via USART
 *
 * @param data Pointer to data to send
 * @param num Number of data bytes to send
 * @return int32_t ARM_DRIVER_OK on success, error code otherwise
 */
int32_t HAL_ARM_USART_Send(const uint8_t *data, uint32_t num)
{
    int32_t  result = ARM_DRIVER_OK;

    if ((NULL == data) || (0U == num))
    {
        result = ARM_DRIVER_ERROR_PARAMETER;
    }
    else if (0U == s_uart1_ctx.initialized)
    {
        result = ARM_DRIVER_ERROR;
    }
    else if (HAL_UART_POWER_FULL != s_uartPowerState)
    {
        result = ARM_DRIVER_ERROR;
    }
    else if (0U != s_usartStatus.tx_busy)
    {
        result = ARM_DRIVER_ERROR_BUSY;
    }
    else
    {
        s_txBuffer            = data;
        s_txLen               = num;
        s_txPos               = 0U;
        s_usartTxCount        = 0U;
        s_usartStatus.tx_busy = 1U;

        if (0U != (IP_LPUART1->STAT & LPUART_STAT_TDRE_MASK))
        {
            IP_LPUART1->DATA = s_txBuffer[s_txPos];
            s_txPos++;
        }

        IP_LPUART1->CTRL |= LPUART_CTRL_TIE_MASK;
    }

    return result;
}

/**
 * @brief Receive data via USART
 *
 * @param data Pointer to data to receive
 * @param num Number of data bytes to receive
 * @return int32_t ARM_DRIVER_OK on success, error code otherwise
 */
int32_t HAL_ARM_USART_Receive(uint8_t *data, uint32_t num)
{
    int32_t  result      = ARM_DRIVER_OK;

    if ((NULL == data) || (0U == num))
    {
        result = ARM_DRIVER_ERROR_PARAMETER;
    }
    else if (0U == s_uart1_ctx.initialized)
    {
        result = ARM_DRIVER_ERROR;
    }
    else if (HAL_UART_POWER_FULL != s_uartPowerState)
    {
        result = ARM_DRIVER_ERROR;
    }
    else if (0U != s_usartStatus.rx_busy)
    {
        result = ARM_DRIVER_ERROR_BUSY;
    }
    else
    {
        s_rxBuffer            = data;
        s_rxLen               = num;
        s_rxPos               = 0U;
        s_usartRxCount        = 0U;
        s_usartStatus.rx_busy = 1U;

        IP_LPUART1->CTRL |= LPUART_CTRL_RIE_MASK;
    }

    return result;
}

/**
 * @brief Transfer data to/from USART
 *
 * @param data_out Pointer to data to send
 * @param data_in Pointer to data to receive
 * @param num Number of data bytes to transfer
 * @return int32_t ARM_DRIVER_OK on success, error code otherwise
 */
int32_t HAL_ARM_USART_Transfer(const uint8_t *data_out, uint8_t *data_in, uint32_t num)
{
    int32_t result = ARM_DRIVER_ERROR_UNSUPPORTED;

    (void)data_out;
    (void)data_in;
    (void)num;

    return result;
}

/**
 * @brief Get transmitted data count
 *
 * @return uint32_t Number of transmitted data
 */
uint32_t HAL_ARM_USART_GetTxCount(void)
{
    uint32_t tx_count = 0U;

    tx_count = s_usartTxCount;

    return tx_count;
}

/**
 * @brief Get received data count
 *
 * @return uint32_t Number of received data
 */
uint32_t HAL_ARM_USART_GetRxCount(void)
{
    uint32_t rx_count = 0U;

    rx_count = s_usartRxCount;

    return rx_count;
}

/**
 * @brief Get USART status
 *
 * @return ARM_USART_STATUS Current USART status
 */
ARM_USART_STATUS HAL_ARM_USART_GetStatus(void)
{
    ARM_USART_STATUS status;

    status = s_usartStatus;

    return status;
}

/**
 * @brief Set modem control signals
 *
 * @param control Modem control command
 * @return int32_t ARM_DRIVER_OK on success, error code otherwise
 */
int32_t HAL_ARM_USART_SetModemControl(ARM_USART_MODEM_CONTROL control)
{
    int32_t result = ARM_DRIVER_ERROR_UNSUPPORTED;

    (void)control;

    return result;
}

/**
 * @brief Get modem status signals
 *
 * @return ARM_USART_MODEM_STATUS Current modem status
 */
ARM_USART_MODEM_STATUS HAL_ARM_USART_GetModemStatus(void)
{
    ARM_USART_MODEM_STATUS status;

    status.cts = 0U;
    status.dsr = 0U;
    status.ri  = 0U;
    status.dcd = 0U;

    return status;
}
