/**
 * @file Bootloader.h
 * @author Dung Van Pham (dungpv00@gmail.com)
 * @brief Bootloader interface for S32K144
 * @version 0.1
 * @date 2026-01-27
 *
 * @copyright Copyright (c) 2026
 *
 */
#ifndef BOOTLOADER_H_
#define BOOTLOADER_H_

#include <stdint.h>
#include "srec.h"

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define BOOTLOADER_APP_BASE_ADDR   (0x00008000UL)

typedef enum
{
    BL_OK = 0,
    BL_ERR_PARAM = -1,
    BL_ERR_RANGE = -2,
    BL_ERR_FLASH = -3,
    BL_ERR_VERIFY = -4
} bl_status_t;

/*******************************************************************************
 * API
 ******************************************************************************/
uint8_t Bootloader_IsUserAppValid(uint32_t app_base_addr);
void Bootloader_JumpToUserApp(uint32_t app_base_addr);
typedef void (*JumpToPtr)(void);
void Bootloader_HandleRecord(const srec_record_t *record, uint32_t *entry_point);

#ifdef __cplusplus
}
#endif

#endif /* BOOTLOADER_H_ */
