/**
 * @file port.h
 * @author Dung Pham Van (dungpv00@gmail.com)
 * @brief  Driver cấu hình PORT cho vi điều khiển S32K144 theo chuẩn CMSIS.
 *         Cung cấp API bật clock, cấu hình chân và xử lý cờ ngắt cho PORT.
 * @version 0.1
 * @date 2025-10-18
 *
 * @details
 *  - Cho phép bật clock cho từng PORT (A–E).
 *  - Cấu hình từng chân PORT gồm:
 *      + Chức năng (mux): GPIO hoặc chức năng ngoại vi khác.
 *      + Pull-up/Pull-down hoặc không kéo.
 *      + Cấu hình chế độ ngắt (IRQC).
 *  - Hỗ trợ xóa cờ ngắt và đọc cờ ngắt theo mask bit.
 *
 * @note PORT chịu trách nhiệm cấu hình **chức năng chân**,
 *       còn việc điều khiển mức logic (0/1) thực hiện qua module **GPIO**.
 *
 * @copyright Copyright (c) 2025
 */

#ifndef PORT_H_
#define PORT_H_

#include <stdint.h>
#include <stdbool.h>
#include "S32K144.h"

typedef enum {
	PORT_Mux_GPIO		= 1u,
} PORT_Mux_t;

typedef enum {
	PORT_Pull_Disabled	= 0,
	PORT_Pull_Down		= 1,
	PORT_Pull_Up 		= 2,
} PORT_Pull_t;

typedef enum {
	PORT_Irq_Disabled		= 0,
	PORT_Irq_LogicZero		= 8,
	PORT_Irq_RisingEdge		= 9,
	PORT_Irq_FallingRdge	= 10,
	PORT_Irq_EitherEdge		= 11,
	PORT_Irq_LogicOne		= 12,
} PORT_IrqMode_t;

typedef struct {
	PORT_Mux_t		mux;		// 1 = GPIO
	PORT_Pull_t		pull;		// off / down / up
	PORT_IrqMode_t	irqMode;	// IRQC
} PORT_ConfigPin_t;

/* ---- API ---- */
void PORT_EnableClock(PORT_Type *PORTx);
void PORT_ConfigPin(PORT_Type *PORTx, uint32_t pin, const PORT_ConfigPin_t *cfg);
static inline void PORT_ClearIntFlag(PORT_Type *PORTx, uint32_t pin){
	PORTx -> PCR[pin] |= PORT_PCR_ISF_MASK; // ghi 1 để xóa cờ ngắt
}
static inline uint32_t PORT_GetMaskedFlags(PORT_Type *PORTx, uint32_t mask_bits){
	return PORTx -> ISFR & mask_bits;
}

#endif /* PORT_H_ */
