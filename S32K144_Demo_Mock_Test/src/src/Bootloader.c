/**
 * @file Bootloader.c
 * @author Dung Van Pham (dungpv00@gmail.com)
 * @brief Bootloader implementation for S32K144
 * @version 0.1
 * @date 2026-01-11
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "incl/Bootloader.h"
#include "s32_core_cm4.h"
#include "S32K144_features.h"
#include "S32K144.h"
#include <stdint.h>
#include "incl/FLASH.h"
#include "incl/NVIC.h"
#include "incl/srec.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define S32K144_SRAM_START        (0x1FFF8000UL)
#define S32K144_SRAM_END_EXCL     (0x20007000UL)

#define S32K144_FLASH_START       (0x00000000UL)
#define S32K144_FLASH_END_EXCL    (0x00080000UL) /* end-exclusive */

#define BOOTLOADER_SIZE           (0xA000UL)      /* bootloader size */
#define APP_START_ADDR            (S32K144_FLASH_START + BOOTLOADER_SIZE)
#define APP_END_EXCL              (S32K144_FLASH_END_EXCL)

#define FLASH_SECTOR_SIZE         (FTFC_P_FLASH_SECTOR_SIZE)

#define APP_SECTOR_COUNT          (((APP_END_EXCL - APP_START_ADDR) + (FLASH_SECTOR_SIZE - 1UL)) / FLASH_SECTOR_SIZE)

#define NULL                      ((void*)0)

/*******************************************************************************
 * Variables
 ******************************************************************************/
static uint8_t s_update_active = 0U;
static uint8_t s_seen_data     = 0U;

/*******************************************************************************
 * Local functions
 ******************************************************************************/
static uint8_t Bootloader_IsAddressInRange(uint32_t addr, uint32_t start, uint32_t end_exclusive)
{
    uint8_t result = 0U;

    if ((addr >= start) && (addr < end_exclusive))
    {
        return 1U;
    }

    return result;
}
/*******************************************************************************
 * API
 ******************************************************************************/
/**
 * @brief Check if user application is valid by inspecting initial MSP and Reset Handler
 *
 * @param app_base_addr Base address of user application (vector table)
 * @return uint8_t 1 if valid, 0 otherwise
 */
uint8_t Bootloader_IsUserAppValid(uint32_t app_base_addr)
{
    uint32_t app_msp;
    uint32_t app_reset;

    app_msp   = *((uint32_t *)(app_base_addr + 0UL));
    app_reset = *((uint32_t *)(app_base_addr + 4UL));

    /* Basic erased checks */
    if ((0x00000000UL == app_msp) || (0xFFFFFFFFUL == app_msp))
    {
        return 0U;
    }

    /* MSP can be exactly at top-of-stack (end address) */
    if ((app_msp < S32K144_SRAM_START) || (app_msp > S32K144_SRAM_END_EXCL))
    {
        return 0U;
    }

    /* Reset handler must be Thumb */
    if (0UL == (app_reset & 0x1UL))
    {
        return 0U;
    }

    /* Mask thumb bit for range check */
    if (0U == Bootloader_IsAddressInRange((app_reset & 0xFFFFFFFEUL), S32K144_FLASH_START, S32K144_FLASH_END_EXCL))
    {
        return 0U;
    }

    return 1U;
}

/**
 * @brief Jump to user application at given base address
 *
 * @param app_base_addr Base address of user application (vector table)
 */
void Bootloader_JumpToUserApp(uint32_t app_base_addr)
{
    uint32_t app_msp;
    uint32_t app_reset;
    JumpToPtr JumpTo;

    app_msp   = *((uint32_t *)(app_base_addr + 0UL));
    app_reset = *((uint32_t *)(app_base_addr + 4UL));

    DISABLE_INTERRUPTS();

    /* Relocate vector table */
    S32_SCB->VTOR = app_base_addr;

    /* Set MSP manually */
    __asm volatile (
        "msr msp, %0\n"
        "dsb\n"
        "isb\n"
        :
        : "r" (app_msp)
        : "memory"
    );

    /* Jump to Reset_Handler */
    JumpTo = (JumpToPtr)app_reset;
    JumpTo();

    while (1)
    {
        /* Should never return */
    }
}

/**
 * @brief Program flash memory with given data
 *
 * @param address Start address to program
 * @param data Pointer to data buffer
 * @param len Length of data in bytes
 * @return int32_t 0 if success, negative value if error
 */
static int32_t Bootloader_ProgramFlash(uint32_t address, const uint8_t *data, uint32_t len)
{
    bl_status_t result = BL_OK;

    uint32_t end_excl;
    uint32_t first_dw_addr;
    uint32_t last_dw_addr;
    uint32_t cur_dw;
    uint32_t i;

    uint8_t buf8[FTFC_WRITE_DOUBLE_WORD];

    if ((data == NULL) || (len == 0UL))
    {
        return BL_ERR_PARAM;
    }

    end_excl = address + len;
    if (end_excl < address)
    {
        return BL_ERR_PARAM; /* overflow */
    }

    /* Range check end-exclusive */
    if ((address < APP_START_ADDR) || (end_excl > APP_END_EXCL))
    {
        return BL_ERR_RANGE;
    }

    /* Program with 8-byte RMW blocks */
    first_dw_addr = address & ~(FTFC_WRITE_DOUBLE_WORD - 1UL);
    last_dw_addr  = (end_excl - 1UL) & ~(FTFC_WRITE_DOUBLE_WORD - 1UL);

    for (cur_dw = first_dw_addr; cur_dw <= last_dw_addr; cur_dw += FTFC_WRITE_DOUBLE_WORD)
    {
        uint32_t w0;
        uint32_t w1;

        w0 = Read_FlashAddress(cur_dw);
        w1 = Read_FlashAddress(cur_dw + 4UL);

        buf8[0] = (uint8_t)(w0 >> 0);
        buf8[1] = (uint8_t)(w0 >> 8);
        buf8[2] = (uint8_t)(w0 >> 16);
        buf8[3] = (uint8_t)(w0 >> 24);
        buf8[4] = (uint8_t)(w1 >> 0);
        buf8[5] = (uint8_t)(w1 >> 8);
        buf8[6] = (uint8_t)(w1 >> 16);
        buf8[7] = (uint8_t)(w1 >> 24);

        for (i = 0UL; i < FTFC_WRITE_DOUBLE_WORD; i++)
        {
            uint32_t byte_addr;

            byte_addr = cur_dw + i;

            if ((byte_addr >= address) && (byte_addr < end_excl))
            {
                buf8[i] = data[byte_addr - address];
            }
        }

        /* Program full 8 bytes at aligned address
         * Note: you said Program_LongWord_8B() returns 1 on success
         */
        if (Program_LongWord_8B(cur_dw, buf8) == 0U)
        {
            return BL_ERR_FLASH;
        }
    }

    /* Verify only [address, end_excl) */
    for (i = 0UL; i < len; i++)
    {
        uint32_t a;
        uint32_t w;
        uint8_t  rb;

        a = address + i;

        w = Read_FlashAddress(a & ~3UL);

        switch (a & 3UL)
        {
            case 0UL: rb = (uint8_t)(w >> 0);  break;
            case 1UL: rb = (uint8_t)(w >> 8);  break;
            case 2UL: rb = (uint8_t)(w >> 16); break;
            default:  rb = (uint8_t)(w >> 24); break;
        }

        if (rb != data[i])
        {
            return BL_ERR_VERIFY;
        }
    }

    return result;
}

void Bootloader_HandleRecord(const srec_record_t *record, uint32_t *entry_point)
{
    if ((NULL == record) || (NULL == entry_point))
    {
        return;
    }

    switch (record->type)
    {
        case '1':
        case '2':
        case '3':
        {
            s_update_active = 1U;

            if (BL_OK == Bootloader_ProgramFlash(record->address,
                                                 record->data,
                                                 record->data_len))
            {
                s_seen_data = 1U;
            }
            break;
        }


        case '7':
        case '8':
        case '9':
        {
            if ((1U == s_update_active) && (1U == s_seen_data))
            {
                Bootloader_JumpToUserApp(APP_START_ADDR);
            }
            break;
        }

        default:
        {
            break;
        }
    }
}

