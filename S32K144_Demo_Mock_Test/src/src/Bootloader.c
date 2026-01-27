/**
 * @file Bootloader.c
 * @author Dung Van Pham
 * @brief Bootloader implementation for S32K144
 * @version 0.1
 * @date 2026-01-11
 */

#include "incl/Bootloader.h"
#include "s32_core_cm4.h"
#include "S32K144_features.h"
#include "S32K144.h"
#include <stdint.h>
#include <stddef.h>

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

#define PHRASE_SIZE               (FTFC_WRITE_DOUBLE_WORD) /* 8 bytes */

/*******************************************************************************
 * Types
 ******************************************************************************/
/**
 * @brief Phrase cache structure
 */
 typedef struct
{
    uint32_t base;                 /* aligned 8 */
    uint8_t  data[PHRASE_SIZE];    /* image to program */
    uint8_t  valid;
    uint8_t  dirty;
} phrase_cache_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void    PhraseCache_Reset(phrase_cache_t *c);
static int32_t PhraseCache_Flush(phrase_cache_t *c);

static void    Bootloader_StreamBegin(void);
static int32_t Bootloader_StreamEnd(void);
static int32_t Bootloader_StreamWrite(uint32_t address, const uint8_t *data, uint32_t len);

static uint8_t Bootloader_IsAddressInRange(uint32_t addr, uint32_t start, uint32_t end_exclusive);
static uint8_t PhraseCache_IsAlreadyProgrammedSame(uint32_t base, const uint8_t *buf8);
static uint8_t Bootloader_FlashIsOkAfterCmd(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
static uint8_t  s_update_active = 0U;
static uint8_t  s_seen_data     = 0U;
static uint32_t g_entry_point   = 0U;

static phrase_cache_t g_cache;
static uint8_t        g_cache_inited = 0U;

/*******************************************************************************
 * Local functions
 ******************************************************************************/
/**
 * @brief Reset phrase cache
 * @param c phrase cache to reset
 */
 static void PhraseCache_Reset(phrase_cache_t *c)
{
    uint32_t i;
    if (c != NULL)
    {
        c->base  = 0U;
        c->valid = 0U;
        c->dirty = 0U;

        for (i = 0U; i < PHRASE_SIZE; i++)
        {
            c->data[i] = 0xFFU;
        }
    }
}

/**
 * @brief Check if address in range [start, end_exclusive)
 *
 * @param addr address to check
 * @param start start of range
 * @param end_exclusive end of range (exclusive)
 * @return uint8_t 1: in range, 0: out of range
 */
static uint8_t Bootloader_IsAddressInRange(uint32_t addr, uint32_t start, uint32_t end_exclusive)
{
    uint8_t result = 0U;

    if ((addr >= start) && (addr < end_exclusive))
    {
        result = 1U;
    }

    return result;
}

/**
 * @brief Check if phrase at base address is already programmed with same data
 *
 * @param base base address of the phrase
 * @param buf8 pointer to the buffer containing the data to compare
 * @return uint8_t 1: same, 0: different
 */
static uint8_t PhraseCache_IsAlreadyProgrammedSame(uint32_t base, const uint8_t *buf8)
{
    uint8_t  same = 1U;
    uint32_t w0   = Read_FlashAddress(base);
    uint32_t w1   = Read_FlashAddress(base + 4UL);

    uint8_t cur[PHRASE_SIZE];
    cur[0] = (uint8_t)(w0 >> 0);
    cur[1] = (uint8_t)(w0 >> 8);
    cur[2] = (uint8_t)(w0 >> 16);
    cur[3] = (uint8_t)(w0 >> 24);
    cur[4] = (uint8_t)(w1 >> 0);
    cur[5] = (uint8_t)(w1 >> 8);
    cur[6] = (uint8_t)(w1 >> 16);
    cur[7] = (uint8_t)(w1 >> 24);

    if (buf8 != NULL)
    {
        for (uint32_t i = 0U; i < PHRASE_SIZE; i++)
        {
            if (cur[i] != buf8[i])
            {
                same = 0U;
                break;
            }
        }
    }
    else
    {
        same = 0U;
    }

    return same;
}

/**
 * @brief Check flash status register after command
 *
 * @return uint8_t 1: ok, 0: error
 */
static uint8_t Bootloader_FlashIsOkAfterCmd(void)
{
    uint8_t ok = 1U;
    uint8_t f  = IP_FTFC->FSTAT;

    /* CCIF must be set */
    if (0U == (f & 0x80U))
    {
        ok = 0U;
    }
    /* ACCERR */
    else if ((0U == (f & 0x20U)) || (0U == (f & 0x40U)))
    {
        ok = 0U;
    }
    /* FPVIOL */
    else if (0U == (f & 0x10U))
    {
        ok = 0U;
    }
    /* MGSTAT0 */
    else if (0U == (f & 0x01U))
    {
        ok = 0U;
    }
    else
    {
        /* ok */
    }

    return ok;
}

/**
 * @brief Flush phrase cache to flash
 *
 * @param c phrase cache to flush
 * @return int32_t status
 */
static int32_t PhraseCache_Flush(phrase_cache_t *c)
{
    int32_t status = BL_OK;

    if ((NULL == c) || (0U == c->valid) || (0U == c->dirty))
    {
        status = BL_OK;
    }
    else
    {
        if (PhraseCache_IsAlreadyProgrammedSame(c->base, c->data) != 0U)
        {
            c->dirty = 0U;
            status = BL_OK;
        }
        else
        {
            (void)Program_LongWord_8B(c->base, c->data);

            if (0U == Bootloader_FlashIsOkAfterCmd())
            {
                status = BL_ERR_FLASH;
            }
            else
            {
                c->dirty = 0U;
                status = BL_OK;
            }
        }
    }

    return status;
}

/**
 * @brief Initialize stream programming
 */
static void Bootloader_StreamBegin(void)
{
    PhraseCache_Reset(&g_cache);
    g_cache_inited = 1U;
}

/**
 * @brief Finalize stream programming
 */
static int32_t Bootloader_StreamEnd(void)
{
    int32_t status = BL_OK;

    if (0U == g_cache_inited)
    {
        status = BL_OK;
    }
    else
    {
        status = PhraseCache_Flush(&g_cache);
    }

    return status;
}

/**
 * @brief Program flash memory with given data (cached by 8-byte phrase)
 */
static int32_t Bootloader_StreamWrite(uint32_t address, const uint8_t *data, uint32_t len)
{
    int32_t  status   = BL_OK;
    uint32_t end_excl = 0U;

    if ((NULL == data) || (0UL == len))
    {
        status = BL_ERR_PARAM;
    }
    else
    {
        end_excl = address + len;
        if (end_excl < address)
        {
            status = BL_ERR_PARAM;
        }
        else if ((address < APP_START_ADDR) || (end_excl > APP_END_EXCL))
        {
            status = BL_ERR_RANGE;
        }
        else
        {
            if (g_cache_inited == 0U)
            {
                Bootloader_StreamBegin();
            }

            for (uint32_t i = 0U; i < len; i++)
            {
                uint32_t a    = address + i;
                uint32_t base = a & ~(PHRASE_SIZE - 1UL);
                uint32_t off  = a - base;

                if ((g_cache.valid != 0U) && (g_cache.base != base))
                {
                    status = PhraseCache_Flush(&g_cache);
                    if (status != BL_OK)
                    {
                        break;
                    }
                    PhraseCache_Reset(&g_cache);
                }

                if (g_cache.valid == 0U)
                {
                    g_cache.valid = 1U;
                    g_cache.base  = base;
                }

                g_cache.data[off] = data[i];
                g_cache.dirty = 1U;
            }
        }
    }

    return status;
}

/*******************************************************************************
 * API
 ******************************************************************************/
/**
 * @brief Check if user application is valid
 * @param app_base_addr Base address of user application
 * @return uint8_t 1: valid, 0: invalid
 */
 uint8_t Bootloader_IsUserAppValid(uint32_t app_base_addr)
{
    uint8_t  valid = 0U;
    uint32_t app_msp;
    uint32_t app_reset;

    app_msp   = *((uint32_t *)(app_base_addr + 0UL));
    app_reset = *((uint32_t *)(app_base_addr + 4UL));

    if ((0x00000000UL == app_msp) || (0xFFFFFFFFUL == app_msp))
    {
        valid = 0U;
    }
    else if ((app_msp < S32K144_SRAM_START) || (app_msp > S32K144_SRAM_END_EXCL))
    {
        valid = 0U;
    }
    else if ((app_reset & 0x1UL) == 0UL)
    {
        valid = 0U;
    }
    else if (Bootloader_IsAddressInRange((app_reset & 0xFFFFFFFEUL),
                                         S32K144_FLASH_START,
                                         S32K144_FLASH_END_EXCL) == 0U)
    {
        valid = 0U;
    }
    else
    {
        valid = 1U;
    }

    return valid;
}

/**
 * @brief Jump to user application
 * @param app_base_addr Base address of user application
 */
void Bootloader_JumpToUserApp(uint32_t app_base_addr)
{
    uint32_t  app_msp;
    uint32_t  app_reset;
    JumpToPtr JumpTo;

    app_msp   = *((uint32_t *)(app_base_addr + 0UL));
    app_reset = *((uint32_t *)(app_base_addr + 4UL));

    DISABLE_INTERRUPTS();

    /* Relocate vector table */
    S32_SCB->VTOR = app_base_addr;

    /* Set MSP */
    __asm volatile ("msr msp, %0\n" "dsb\n" "isb\n":: "r" (app_msp): "memory");

    /* Jump to Reset_Handler (already Thumb if app valid) */
    JumpTo = (JumpToPtr)app_reset;
    JumpTo();

    while (1)
    {
        /* Never return */
    }
}

/**
 * @brief Handle SREC record
 *
 * @param record Pointer to SREC record
 * @param entry_point Pointer to entry point variable
 */
void Bootloader_HandleRecord(const srec_record_t *record, uint32_t *entry_point)
{
    int32_t flush_st = BL_OK;

    if ((record == NULL) || (entry_point == NULL))
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

            if (Bootloader_StreamWrite(record->address, record->data, record->data_len) == BL_OK)
            {
                s_seen_data = 1U;
            }
            break;
        }

        case '7':
        case '8':
        case '9':
        {
            flush_st = Bootloader_StreamEnd();

            if ((flush_st == BL_OK) && (Bootloader_IsUserAppValid(APP_START_ADDR) != 0U))
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
