/**
 * @file srec.c
 * @author Dung (dungpv00@gmail.com)
 * @brief SREC parsing implementation
 * @version 0.1
 * @date 2025-12-28
 *
 * @copyright Copyright (c) 2025
 *
 */
/*============================================================================
 * INCLUDE
 =============================================================================*/
#include "incl/srec.h"

#include <stdint.h>
#include <stdio.h>

/*============================================================================
 * LOCAL FUNCTIONS
 =============================================================================*/

 /**
  * @brief Check if character is a valid hexadecimal character
  *
  * @param c Character to check
  * @return uint8_t 1 if valid hex character, 0 otherwise
  */
static uint8_t Srec_IsHexChar(uint8_t c)
{
    uint8_t result = 0U;

    if (((c >= '0') && (c <= '9')) ||
        ((c >= 'A') && (c <= 'F')) ||
        ((c >= 'a') && (c <= 'f')))
    {
        result = 1U;
    }

    return result;
}

/**
 * @brief Convert a hexadecimal character to its numerical value
 *
 * @param c Hexadecimal character
 * @param val Pointer to store the numerical value
 * @return uint8_t 1 if conversion successful, 0 otherwise
 */
static uint8_t Srec_HexCharToVal(uint8_t c, uint8_t *val)
{
    uint8_t result = 0U;

    if (val != NULL)
    {
        if ((c >= '0') && (c <= '9'))
        {
            *val = (uint8_t)(c - '0');
            result = 1U;
        }
        else if ((c >= 'A') && (c <= 'F'))
        {
            *val = (uint8_t)(c - 'A' + 10);
            result = 1U;
        }
        else if ((c >= 'a') && (c <= 'f'))
        {
            *val = (uint8_t)(c - 'a' + 10);
            result = 1U;
        }
        else
        {
            result = 0U;
        }
    }

    return result;
}

/**
 * @brief Convert two hexadecimal characters to a byte
 *
 * @param high High nibble character
 * @param low Low nibble character
 * @param out_byte Pointer to store the resulting byte
 * @return Srec_Status_t Status of the conversion
 */
static Srec_Status_t Srec_HexToByte(uint8_t high, uint8_t low, uint8_t *out_byte)
{
    Srec_Status_t status = SREC_STATUS_PARAM;
    uint8_t hi = 0U;
    uint8_t lo = 0U;

    if (out_byte != NULL)
    {
        if ((Srec_HexCharToVal(high, &hi) != 0U) &&
            (Srec_HexCharToVal(low,  &lo) != 0U))
        {
            *out_byte = (uint8_t)((hi << 4U) | lo);
            status = SREC_STATUS_OK;
        }
        else
        {
            status = SREC_STATUS_FORMAT;
        }
    }

    return status;
}

/**
 * @brief Get length of SREC line (excluding CR/LF)
 *
 * @param cmd_buffer Pointer to SREC line buffer
 * @return uint32_t Length of SREC line
 */
uint32_t Srec_LineLen(const uint8_t *cmd_buffer)
{
    uint32_t len;

    len = 0U;

    if (cmd_buffer != NULL)
    {
        while ((cmd_buffer[len] != SREC_NUL) &&
               (cmd_buffer[len] != SREC_CR)  &&
               (cmd_buffer[len] != SREC_LF))
        {
            len++;
        }
    }

    return len;
}

/*============================================================================
 * API FUNCTIONS
 =============================================================================*/
/**
 * @brief Get length of SREC line (excluding CR/LF)
 *
 * @param cmd_buffer Pointer to SREC line buffer
 * @param rec Pointer to output SREC record structure
 * @return Srec_Status_t Status of parsing
 */
Srec_Status_t Srec_ParseSrecLine(const uint8_t *cmd_buffer, srec_record_t *rec)
{
    Srec_Status_t status;
    uint32_t      line_len;
    uint8_t       count;
    uint8_t       checksum;
    uint8_t       sum;
    uint8_t       addr_bytes[4U];
    uint32_t      address;
    uint32_t      address_bytes;
    uint32_t      data_bytes;
    uint32_t      i;
    uint32_t      pos;
    uint32_t      expected_ascii_len;

    /* Defaults */
    status        = SREC_STATUS_PARAM;
    count         = 0U;
    checksum      = 0U;
    sum           = 0U;
    address       = 0U;
    address_bytes = 0U;
    data_bytes    = 0U;
    i             = 0U;
    pos           = 0U;
    expected_ascii_len = 0U;

    if ((NULL == cmd_buffer) || (NULL == rec))
    {
        /* PARAM */
    }
    else
    {
        /* Clear output first */
        rec->type    = 0;
        rec->address = 0U;
        rec->data_len = 0U;

        line_len = Srec_LineLen(cmd_buffer);

        /* Minimal: "S" + type + count(2 hex) => at least 4 chars */
        if (line_len < 4U)
        {
            status = SREC_STATUS_FORMAT;
        }
        else if ((cmd_buffer[0] != (uint8_t)'S'))
        {
            status = SREC_STATUS_FORMAT;
        }
        else
        {
            rec->type = (char)cmd_buffer[1];

            /* Address length by record type */
            switch (rec->type)
            {
                case '0': address_bytes = 2U; break; /* header */
                case '1': address_bytes = 2U; break;
                case '2': address_bytes = 3U; break;
                case '3': address_bytes = 4U; break;
                case '5': address_bytes = 2U; break; /* count */
                case '7': address_bytes = 4U; break; /* entry */
                case '8': address_bytes = 3U; break;
                case '9': address_bytes = 2U; break;
                default:
                    status = SREC_STATUS_FORMAT;
                    break;
            }

            if (SREC_STATUS_FORMAT != status)
            {
                /* Parse count */
                status = Srec_HexToByte((char)cmd_buffer[2], (char)cmd_buffer[3], &count);
                if (SREC_STATUS_OK == status)
                {
                    /* count must be at least addr_bytes + checksum(1) */
                    if (count < (uint8_t)(address_bytes + 1U))
                    {
                        status = SREC_STATUS_FORMAT;
                    }
                    else
                    {
                        data_bytes = (uint32_t)count - address_bytes - 1U;

                        if (data_bytes > SREC_MAX_DATA_BYTES)
                        {
                            status = SREC_STATUS_FORMAT;
                        }
                    }
                }
                else
                {
                    status = SREC_STATUS_FORMAT;
                }
            }

            /* Validate ASCII length: total chars (no CR/LF) should be 4 + 2*count */
            if (SREC_STATUS_OK == status)
            {
                expected_ascii_len = 4U + (2U * (uint32_t)count);

                if (line_len < expected_ascii_len)
                {
                    status = SREC_STATUS_FORMAT;
                }
            }

            /* Parse address */
            if (SREC_STATUS_OK == status)
            {
                pos = 4U;

                for (i = 0U; i < address_bytes; i++)
                {
                    status = Srec_HexToByte((char)cmd_buffer[pos], (char)cmd_buffer[pos + 1U], &addr_bytes[i]);
                    if (SREC_STATUS_OK != status)
                    {
                        status = SREC_STATUS_FORMAT;
                        break;
                    }
                    pos += 2U;
                }

                if (SREC_STATUS_OK == status)
                {
                    address = 0U;
                    for (i = 0U; i < address_bytes; i++)
                    {
                        address = (address << 8U) | (uint32_t)addr_bytes[i];
                    }
                    rec->address = address;
                }
            }

            /* Parse data */
            if (SREC_STATUS_OK == status)
            {
                rec->data_len = 0U;

                for (i = 0U; i < data_bytes; i++)
                {
                    status = Srec_HexToByte((char)cmd_buffer[pos], (char)cmd_buffer[pos + 1U], &rec->data[i]);
                    if (SREC_STATUS_OK != status)
                    {
                        status = SREC_STATUS_FORMAT;
                        break;
                    }
                    pos += 2U;
                }

                if (SREC_STATUS_OK == status)
                {
                    rec->data_len = data_bytes;
                }
            }

            /* Parse checksum */
            if (SREC_STATUS_OK == status)
            {
                status = Srec_HexToByte((char)cmd_buffer[pos], (char)cmd_buffer[pos + 1U], &checksum);
                if (SREC_STATUS_OK != status)
                {
                    status = SREC_STATUS_FORMAT;
                }
                else
                {
                    pos += 2U;
                }
            }

            /* Verify checksum: (sum of count + addr + data + checksum) & 0xFF == 0xFF */
            if (SREC_STATUS_OK == status)
            {
                sum = count;

                for (i = 0U; i < address_bytes; i++)
                {
                    sum = (uint8_t)(sum + addr_bytes[i]);
                }

                for (i = 0U; i < rec->data_len; i++)
                {
                    sum = (uint8_t)(sum + rec->data[i]);
                }

                if ((uint8_t)((uint8_t)(sum + checksum)) != (uint8_t)0xFFU)
                {
                    status = SREC_STATUS_CHECKSUM;
                }
            }
        }
    }

    /* If invalid, clear output */
    if (SREC_STATUS_OK != status)
    {
        if (rec != NULL)
        {
            rec->type     = 0;
            rec->address  = 0U;
            rec->data_len = 0U;
        }
    }

    return status;
}
