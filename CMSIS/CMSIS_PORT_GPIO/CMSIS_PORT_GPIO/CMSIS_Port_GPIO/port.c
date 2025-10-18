/**
 * @file port.c
 * @author Dung Pham Van (dungpv00@gmail.com)
 * @brief  Triển khai PORT driver cho vi điều khiển S32K144 theo chuẩn CMSIS.
 *         Quản lý bật clock và cấu hình chức năng từng chân PORT.
 * @version 0.1
 * @date 2025-10-18
 *
 * @details
 *  - Cung cấp hàm:
 *      + `PORT_EnableClock()`: Bật clock cho module PORT tương ứng (A–E).
 *      + `PORT_ConfigPin()`: Cấu hình từng chân PORT về chức năng, pull và chế độ ngắt.
 *  - Sử dụng bảng ánh xạ nội bộ để xác định chỉ số PCC tương ứng cho mỗi PORT.
 *  - Mỗi thanh ghi PCR điều khiển cấu hình riêng cho từng chân.
 *
 * @note PORT chỉ thiết lập chức năng chân (multiplexing),
 *       còn điều khiển mức logic (set/reset/toggle) được thực hiện qua module **GPIO**.
 *
 * @copyright Copyright (c) 2025
 */

#include "port.h"

/* Map PORTx -> PCC index */
static inline uint32_t port_to_pcc_index(PORT_Type *PORTx){
	if (PORTx == IP_PORTA) return PCC_PORTA_INDEX;
	if (PORTx == IP_PORTB) return PCC_PORTB_INDEX;
	if (PORTx == IP_PORTC) return PCC_PORTC_INDEX;
	if (PORTx == IP_PORTD) return PCC_PORTD_INDEX;
	return PCC_PORTE_INDEX;
}

void PORT_EnableClock(PORT_Type *PORTx){
    IP_PCC->PCCn[port_to_pcc_index(PORTx)] |= PCC_PCCn_CGC_MASK;
}

void PORT_ConfigPin(PORT_Type *PORTx, uint32_t pin, const PORT_ConfigPin_t *cfg){
	uint32_t pcr = 0u;

	pcr |= PORT_PCR_MUX(cfg -> mux & 0x7u);

	if (cfg -> pull != PORT_Pull_Disabled){
		pcr |= PORT_PCR_PE_MASK;
		if (cfg -> pull == PORT_Pull_Up) pcr |= PORT_PCR_PS_MASK;
	}

	pcr |= PORT_PCR_IRQC(cfg -> irqMode & 0xFu);

	PORTx -> PCR[pin] = pcr;
}
