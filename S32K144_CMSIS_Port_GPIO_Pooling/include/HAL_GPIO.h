/*
 * HAL_GPIO.h
 *
 *  Created on: Nov 29, 2025
 *      Author: ADMIN
 */

#ifndef HAL_GPIO_H_
#define HAL_GPIO_H_

#include "S32K144.h"
#include "device_registers.h"
#include "Driver_Common.h"
#include "Driver_GPIO.h"   // để dùng ARM_GPIO_Pin_t, các enum

#ifdef __cplusplus
extern "C" {
#endif

/* Số pin ảo tối đa */
#define HAL_GPIO_MAX_PINS   64U

/* Cấu trúc map pin ảo -> port và chân thực */
typedef struct {
    GPIO_Type *gpio;   /* PTD, PTC, ... */
    PORT_Type *port;   /* PORTD, PORTC, ... */
    uint8_t    pin;    /* số bit trong port */
} pin_map_t;

/* Callback HAL dùng lại kiểu callback của driver */
typedef ARM_GPIO_SignalEvent_t HAL_GPIO_Callback_t;

/* API tầng HAL – driver sẽ gọi các hàm này */
int32_t  HAL_GPIO_Setup        (ARM_GPIO_Pin_t pin, HAL_GPIO_Callback_t cb_event);
int32_t  HAL_GPIO_SetDirection (ARM_GPIO_Pin_t pin, ARM_GPIO_DIRECTION direction);
int32_t  HAL_GPIO_SetOutputMode(ARM_GPIO_Pin_t pin, ARM_GPIO_OUTPUT_MODE mode);
int32_t  HAL_GPIO_SetPullResistor(ARM_GPIO_Pin_t pin, ARM_GPIO_PULL_RESISTOR resistor);
int32_t  HAL_GPIO_SetEventTrigger(ARM_GPIO_Pin_t pin, ARM_GPIO_EVENT_TRIGGER trigger);
void     HAL_GPIO_Write        (ARM_GPIO_Pin_t pin, uint32_t val);
uint32_t HAL_GPIO_Read         (ARM_GPIO_Pin_t pin);

#ifdef __cplusplus
}
#endif

#endif /* HAL_GPIO_H_ */
