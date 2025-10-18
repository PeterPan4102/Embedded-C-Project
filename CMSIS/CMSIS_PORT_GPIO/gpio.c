/**
 * @file gpio.c
 * @author Dung Pham Van (dungpv00@gmail.com)
 * @brief  Triển khai GPIO driver cho vi điều khiển S32K144 theo chuẩn CMSIS.
 *         Quản lý hướng chân (input/output) và giá trị khởi tạo ban đầu.
 * @version 0.1
 * @date 2025-10-18
 *
 * @details
 *  - Cung cấp hàm:
 *      + `GPIO_ConfigPin()`: Thiết lập hướng chân GPIO (input/output)
 *        và mức logic khởi tạo (High/Low).
 *  - Khi cấu hình chân output:
 *      + Ghi mức khởi tạo (PSOR/PCOR) **trước khi** set bit hướng (PDDR)
 *        để tránh xung nhiễu (glitch) tại thời điểm cấu hình.
 *  - Khi cấu hình chân input:
 *      + Xóa bit tương ứng trong thanh ghi PDDR.
 *
 * @note Các thao tác đọc, ghi, và đảo bit (read/set/clear/toggle)
 *       được định nghĩa trong file **gpio.h** dưới dạng hàm inline.
 *
 * @copyright Copyright (c) 2025
 */

#include "gpio.h"

void GPIO_ConfigPin(GPIO_Type *GPIOx, uint32_t pin, const GPIO_ConfigPin_t *cfg){
	if (cfg -> dir == GPIO_Output){
		// mức khởi tạo trước rồi mới set direction để tránh glitch
		if (cfg -> initHigh)	GPIOx -> PSOR = (1u << pin);
		else					GPIOx -> PCOR = (1u << pin);
		GPIOx -> PDDR |= (1u << pin);
	} else {
		GPIOx -> PDDR &= ~(1u << pin);
	}
}
