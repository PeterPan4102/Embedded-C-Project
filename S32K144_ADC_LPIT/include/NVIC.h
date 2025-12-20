/**
 * @file NVIC.h
 * @author Dung Van Pham (dungpv00@gmail.com)
 * @brief NVIC register definition for S32K144
 * @version 0.1
 * @date 2025-12-20
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef NVIC_H_
#define NVIC_H_

/*==================================================================================================
* INCLUDES
==================================================================================================*/
#include <stdint.h>

/*==================================================================================================
* NVIC REGISTER MAP
==================================================================================================*/

/**
 * @brief NVIC Register Structure
 *
 */
typedef struct
{
    volatile uint32_t ISER[8U];          /* Interrupt Set-Enable Registers (enable IRQs) */
    uint32_t          RESERVED0[24U];
    volatile uint32_t ICER[8U];          /* Interrupt Clear-Enable Registers (disable IRQs) */
    uint32_t          RESERVED1[24U];
    volatile uint32_t ISPR[8U];          /* Interrupt Set-Pending Registers */
    uint32_t          RESERVED2[24U];
    volatile uint32_t ICPR[8U];          /* Interrupt Clear-Pending Registers */
    uint32_t          RESERVED3[24U];
    volatile const uint32_t IABR[8U];    /* Interrupt Active Bit Registers (read-only) */
    uint32_t          RESERVED4[56U];
    volatile uint8_t  IP[240U];          /* Interrupt Priority Registers (one byte per IRQ) */
} MY_NVIC_Type;

/*==================================================================================================
* BASE ADDRESS / INSTANCE
==================================================================================================*/
/* NVIC base address for external interrupts (Cortex-M). */
#define MY_NVIC_BASE   (0xE000E100UL)
#define MY_NVIC        ((MY_NVIC_Type *)MY_NVIC_BASE)

#endif /* NVIC_H_ */
