/**
 * @file Queue.c
 * @author Dung Van Pham (dungpv00@gmail.com)
 * @brief Queue implementation for SREC lines
 * @version 0.1
 * @date 2026-01-11
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "incl/Queue.h"

#include <string.h>

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/**
 * @brief Get minimum of two uint32_t values
 *
 * @param a First value
 * @param b Second value
 */
static uint32_t Queue_Srec_MinU32(uint32_t a, uint32_t b)
{
    uint32_t result;

    if (a < b)
    {
        result = a;
    }
    else
    {
        result = b;
    }

    return result;
}

/*******************************************************************************
 * Code
 ******************************************************************************/
/**
 * @brief Initialize SREC queue
 *
 * @param queue Pointer to queue instance
 * @return Queue_Status_t Status of operation
 */
 Queue_Status_t Queue_Srec_Init(Queue_Srec_t *queue)
{
    Queue_Status_t status;

    if (NULL == queue)
    {
        status = QUEUE_STATUS_PARAM;
    }
    else
    {
    	/* Delete queue */
        (void)memset(queue, 0, sizeof(Queue_Srec_t));
        status = QUEUE_STATUS_OK;
    }

    return status;
}

/**
 * @brief Check if SREC queue is empty
 *
 * @param queue Pointer to queue instance
 * @param is_empty Pointer to boolean indicating if queue is empty
 * @return Queue_Status_t Status of operation
 */
Queue_Status_t Queue_Srec_IsEmpty(const Queue_Srec_t *queue, uint8_t *is_empty)
{
    Queue_Status_t status;

    if ((NULL == queue) || (NULL == is_empty))
    {
        status = QUEUE_STATUS_PARAM;
    }
    else
    {
        if (0U == queue->count)
        {
            *is_empty = 1U;
        }
        else
        {
            *is_empty = 0U;
        }

        status = QUEUE_STATUS_OK;
    }

    return status;
}

/**
 * @brief Check if SREC queue is full
 *
 * @param queue Pointer to queue instance
 * @param is_full Pointer to boolean indicating if queue is full
 * @return Queue_Status_t Status of operation
 */
Queue_Status_t Queue_Srec_IsFull(const Queue_Srec_t *queue, uint8_t *is_full)
{
    Queue_Status_t status;

    if ((NULL == queue) || (is_full == NULL))
    {
        status = QUEUE_STATUS_PARAM;
    }
    else
    {
        if (queue->count >= QUEUE_MAX_ELEMENTS)
        {
            *is_full = 1U;
        }
        else
        {
            *is_full = 0U;
        }

        status = QUEUE_STATUS_OK;
    }

    return status;
}

/**
 * @brief Push SREC line into queue
 *
 * @param queue Pointer to queue instance
 * @param line Pointer to SREC line data
 * @param length Length of SREC line data
 * @return Queue_Status_t Status of operation
 */
Queue_Status_t Queue_Srec_Push(Queue_Srec_t *queue, const uint8_t *line, uint32_t length)
{
    Queue_Status_t status;
    uint8_t        is_full;

    is_full = 0U;

    if ((NULL == queue) || (NULL == line) || (0U == length))
    {
        status = QUEUE_STATUS_PARAM;
    }
    else if (length > QUEUE_MAX_LINE_LENGTH)
    {
        status = QUEUE_STATUS_PARAM;
    }
    else
    {
        Queue_Srec_IsFull(queue, &is_full);

        if (0U != is_full)
        {
            status = QUEUE_STATUS_FULL;
        }
        else
        {
            queue->count++;
            memcpy(queue->elements[queue->tail].data, line, length);
            queue->elements[queue->tail].length = length;

            queue->tail++;
            if (queue->tail >= QUEUE_MAX_ELEMENTS)
            {
                queue->tail = 0U;
            }

            status = QUEUE_STATUS_OK;
        }
    }

    return status;
}

/**
 * @brief Pop SREC line from queue
 *
 * @param queue Pointer to queue instance
 * @param out_line Pointer to output buffer for SREC line
 * @param out_line_max Maximum length of output buffer
 * @param out_length Pointer to variable to receive actual length of popped line
 * @return Queue_Status_t Status of operation
 */
Queue_Status_t Queue_Srec_Pop(Queue_Srec_t *queue, uint8_t *out_line, uint32_t out_line_max, uint32_t *out_length)
{
    Queue_Status_t status;
    uint8_t        is_empty;
    uint32_t       copy_len;

    is_empty = 0U;
    copy_len = 0U;

    if (NULL != out_length)
    {
        *out_length = 0U;
    }
    if ((NULL != out_line) && (out_line_max > 0U))
    {
        out_line[0] = 0U;
    }
    if ((NULL == queue) || (NULL == out_line) || (NULL == out_length))
    {
        status = QUEUE_STATUS_PARAM;
    }
    else if (out_line_max == 0U)
    {
        status = QUEUE_STATUS_PARAM;
    }
    else
    {
        Queue_Srec_IsEmpty(queue, &is_empty);

        if (0U != is_empty)
        {
            status = QUEUE_STATUS_EMPTY;
        }
        else
        {
            copy_len = Queue_Srec_MinU32(queue->elements[queue->head].length, out_line_max);

            memcpy(out_line, queue->elements[queue->head].data, copy_len);
            *out_length = copy_len;

            memset(queue->elements[queue->head].data, 0, QUEUE_MAX_LINE_LENGTH);
            queue->elements[queue->head].length = 0U;

            queue->head++;
            if (queue->head >= QUEUE_MAX_ELEMENTS)
            {
                queue->head = 0U;
            }

            if (queue->count > 0U)
            {
                queue->count--;
            }

            status = QUEUE_STATUS_OK;
        }
    }

    return status;
}
