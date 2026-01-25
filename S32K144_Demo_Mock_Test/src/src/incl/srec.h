/**
 * @file srec.h
 * @author Dung (dungpv00@gmail.com)
 * @brief SREC parsing interface
 * @version 0.1
 * @date 2025-12-28
 *
 * @copyright Copyright (c) 2025
 *
 */
#ifndef SREC_H_
#define SREC_H_

#include <stdint.h>
/*============================================================================
 * DEFINES
 =============================================================================*/
#define SREC_MAX_DATA_BYTES    (256U)
#define SREC_NUL               ((uint8_t)'\0')
#define SREC_CR                ((uint8_t)'\r')
#define SREC_LF                ((uint8_t)'\n')

/*============================================================================
 * ENUMERATIONS
 =============================================================================*/
typedef enum
{
    SREC_STATUS_OK = 0,
    SREC_STATUS_FORMAT,
    SREC_STATUS_PARAM,
    SREC_STATUS_CHECKSUM
} Srec_Status_t;

/*============================================================================
 * TYPES
 =============================================================================*/
typedef struct
{
    uint8_t     type;
    uint32_t    address;
    uint8_t     data[SREC_MAX_DATA_BYTES];
    uint32_t    data_len;
} srec_record_t;

/*============================================================================
 * FUNCTIONS PROTOTYPES
 =============================================================================*/
Srec_Status_t Srec_ParseSrecLine(const uint8_t *cmd_buffer, srec_record_t *rec);

#endif /* SREC_H_ */
