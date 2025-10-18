/**
 * @file gpio.h
 * @author Dung Pham Van (dungpv00@gmail.com)
 * @brief  GPIO driver cho vi điều khiển S32K144 theo chuẩn CMSIS.
 *         Cung cấp API cấu hình hướng chân và thao tác mức logic.
 * @version 0.1
 * @date 2025-10-18
 *
 * @details
 *  - Cho phép cấu hình từng chân GPIO:
 *      + Hướng chân: Input hoặc Output.
 *      + Mức khởi tạo: High hoặc Low.
 *  - Cung cấp các hàm inline để thao tác nhanh:
 *      + `GPIO_SetPin()`   : Đặt mức logic 1 (High).
 *      + `GPIO_ClearPin()` : Đặt mức logic 0 (Low).
 *      + `GPIO_TogglePin()`: Đảo trạng thái chân.
 *      + `GPIO_ReadPin()`  : Đọc mức logic hiện tại.
 *  - Khi dùng output active-low (ví dụ LED), mức logic 0 sẽ bật LED.
 *
 * @note GPIO chỉ điều khiển mức logic,
 *       còn việc cấu hình chức năng chân (mux, pull, ngắt)
 *       được thực hiện qua module **PORT**.
 *
 * @copyright Copyright (c) 2025
 */

#ifndef GPIO_H_
#define GPIO_H_

#include <stdint.h>
#include <stdbool.h>
#include "S32K144.h"

typedef enum {
	GPIO_Input	= 0,
	GPIO_Output	= 1,
} GPIO_Direction_t;

typedef struct {
	GPIO_Direction_t dir;
	bool initHigh;
} GPIO_ConfigPin_t;

void GPIO_ConfigPin(GPIO_Type *GPIOx, uint32_t pin, const GPIO_ConfigPin_t *cfg);
static inline void GPIO_SetPin(GPIO_Type *GPIOx, uint32_t pin){
	GPIOx -> PSOR = (1u << pin);
}
static inline void GPIO_ClearPin(GPIO_Type *GPIOx, uint32_t pin){
	GPIOx -> PCOR = (1u << pin);
}
static inline void GPIO_TogglePin(GPIO_Type *GPIOx, uint32_t pin){
	GPIOx -> PTOR = (1u << pin);
}
static inline bool GPIO_ReadPin(GPIO_Type *GPIOx, uint32_t pin){
	return (GPIOx -> PDIR & (1 << pin));
}

#endif /* GPIO_H_ */
