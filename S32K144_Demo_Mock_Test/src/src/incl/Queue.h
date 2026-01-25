/**
 * @file Queue.h
 * @author Dung Van Pham (dungpv00@gmail.com)
 * @brief Queue header for SREC lines
 * @version 0.1
 * @date 2026-01-11
 *
 * @copyright Copyright (c) 2026
 *
 */
#ifndef QUEUE_H_
#define QUEUE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define QUEUE_MAX_ELEMENTS         (4U)
#define QUEUE_MAX_LINE_LENGTH      (256U)

/*******************************************************************************
 * Types
 ******************************************************************************/
/**
 * @brief Queue status enumeration
 */
typedef enum
{
    QUEUE_STATUS_OK = 0,
    QUEUE_STATUS_ERROR,
    QUEUE_STATUS_FULL,
    QUEUE_STATUS_EMPTY,
    QUEUE_STATUS_PARAM
} Queue_Status_t;

/**
 * @brief Queue element structure
 */
typedef struct
{
    uint8_t  data[QUEUE_MAX_LINE_LENGTH];
    uint32_t length;
} Queue_Element_t;

/**
 * @brief SREC Queue structure
 */
typedef struct
{
    Queue_Element_t elements[QUEUE_MAX_ELEMENTS];
    uint32_t        head;
    uint32_t        tail;
    uint32_t        count;
} Queue_Srec_t;

/*******************************************************************************
 * APIs
 ******************************************************************************/
Queue_Status_t Queue_Srec_Init(Queue_Srec_t *queue);
Queue_Status_t Queue_Srec_Push(Queue_Srec_t *queue, const uint8_t *line, uint32_t length);
Queue_Status_t Queue_Srec_Pop(Queue_Srec_t *queue, uint8_t *out_line, uint32_t out_line_max, uint32_t *out_length);
Queue_Status_t Queue_Srec_IsEmpty(const Queue_Srec_t *queue, uint8_t *is_empty);
Queue_Status_t Queue_Srec_IsFull(const Queue_Srec_t *queue, uint8_t *is_full);

#ifdef __cplusplus
}
#endif

#endif /* QUEUE_H_ */
