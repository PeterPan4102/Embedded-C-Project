/* HAL_GPIO.c */
#include "HAL_GPIO.h"

/* Bảng ánh xạ pin ảo -> chân thật trên S32K144EVB */
static const pin_map_t s_pinMap[HAL_GPIO_MAX_PINS] = {
    [0] = { IP_PTD, IP_PORTD,  0 },  /* LED xanh  - PTD0  */
    [1] = { IP_PTD, IP_PORTD, 15 },  /* LED đỏ    - PTD15 */
    [2] = { IP_PTD, IP_PORTD, 16 },  /* LED xanhL - PTD16 */
    [3] = { IP_PTC, IP_PORTC, 12 },  /* Nút BTN0   - PTC12 */
    [4] = { IP_PTC, IP_PORTC, 13 },  /* Nút BTN1   - PTC13 */
    /* Các pin ảo khác để mặc định = 0 (không dùng) */
};

/* Mảng lưu callback theo pin ảo (sẽ dùng khi làm interrupt) */
static HAL_GPIO_Callback_t s_gpioCallbacks[HAL_GPIO_MAX_PINS];

/* Hàm tiện ích: lấy map pin */
static const pin_map_t *HAL_GPIO_GetMap(ARM_GPIO_Pin_t vpin) {
    if (vpin >= HAL_GPIO_MAX_PINS) {
        return NULL;
    }

    /* ?????? */
    if ((s_pinMap[vpin].gpio == NULL) || (s_pinMap[vpin].port == NULL)) {
        return NULL;
    }
    return &s_pinMap[vpin];
}

/* Bật clock cho PORT tương ứng */
static void HAL_GPIO_EnablePortClock(const pin_map_t *map) {
    if (map->port == IP_PORTC) {
        IP_PCC->PCCn[PCC_PORTC_INDEX] |= PCC_PCCn_CGC_MASK;
    } else if (map->port == IP_PORTD) {
        IP_PCC->PCCn[PCC_PORTD_INDEX] |= PCC_PCCn_CGC_MASK;
    }
    /* Nếu dùng PORTA/B/E thì thêm else if tương ứng */
}

/* ----- API HAL triển khai thực tế ----- */

int32_t HAL_GPIO_Setup(ARM_GPIO_Pin_t pin, HAL_GPIO_Callback_t cb_event) {
    const pin_map_t *map = HAL_GPIO_GetMap(pin);
    if (map == NULL) {
        return ARM_GPIO_ERROR_PIN;
    }

    /* Bật clock cho PORT */
    HAL_GPIO_EnablePortClock(map);

    /* Cấu hình MUX = 1 (GPIO) */
    uint32_t pcr = map->port -> PCR[map->pin];
    pcr &= ~PORT_PCR_MUX_MASK;
    pcr |= PORT_PCR_MUX(1);
    map->port -> PCR[map->pin] = pcr;

    /* Lưu callback (hiện tại chưa dùng đến) */
    s_gpioCallbacks[pin] = cb_event;

    return ARM_DRIVER_OK;
}

int32_t HAL_GPIO_SetDirection(ARM_GPIO_Pin_t pin, ARM_GPIO_DIRECTION direction) {
    const pin_map_t *map = HAL_GPIO_GetMap(pin);
    if (map == NULL) {
        return ARM_GPIO_ERROR_PIN;
    }

    switch (direction) {
    case ARM_GPIO_INPUT:
        map->gpio -> PDDR &= ~(1UL << map->pin); /* 0 = input */
        break;
    case ARM_GPIO_OUTPUT:
        map->gpio -> PDDR |=  (1UL << map->pin); /* 1 = output */
        break;
    default:
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    return ARM_DRIVER_OK;
}

int32_t HAL_GPIO_SetOutputMode(ARM_GPIO_Pin_t pin, ARM_GPIO_OUTPUT_MODE mode) {
    const pin_map_t *map = HAL_GPIO_GetMap(pin);
    (void)map;

    if (map == NULL) {
        return ARM_GPIO_ERROR_PIN;
    }

    /* S32K144 GPIO mặc định là push-pull, không có open-drain */
    if (mode == ARM_GPIO_PUSH_PULL) {
        return ARM_DRIVER_OK;
    } else {
        return ARM_DRIVER_ERROR_PARAMETER;
    }
}

int32_t HAL_GPIO_SetPullResistor(ARM_GPIO_Pin_t pin, ARM_GPIO_PULL_RESISTOR resistor) {
    const pin_map_t *map = HAL_GPIO_GetMap(pin);
    if (map == NULL) {
        return ARM_GPIO_ERROR_PIN;
    }

    uint32_t pcr = map->port->PCR[map->pin];

    /* Xóa cấu hình pull cũ */
    pcr &= ~(PORT_PCR_PE_MASK | PORT_PCR_PS_MASK);

    switch (resistor) {
    case ARM_GPIO_PULL_NONE:
        /* PE = 0: không bật pull */
        break;
    case ARM_GPIO_PULL_UP:
        /* PE=1, PS=1: bật pull-up */
        pcr |= (PORT_PCR_PE_MASK | PORT_PCR_PS_MASK);
        break;
    case ARM_GPIO_PULL_DOWN:
        /* PE=1, PS=0: bật pull-down */
        pcr |= PORT_PCR_PE_MASK;
        break;
    default:
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    map->port->PCR[map->pin] = pcr;

    return ARM_DRIVER_OK;
}

int32_t HAL_GPIO_SetEventTrigger(ARM_GPIO_Pin_t pin, ARM_GPIO_EVENT_TRIGGER trigger) {
    const pin_map_t *map = HAL_GPIO_GetMap(pin);
    if (map == NULL) {
        return ARM_GPIO_ERROR_PIN;
    }

    uint32_t pcr = map->port->PCR[map->pin];

    /* Clear trường IRQC (interrupt config) */
    pcr &= ~PORT_PCR_IRQC_MASK;

    switch (trigger) {
    case ARM_GPIO_TRIGGER_NONE:
        /* IRQC = 0: không interrupt */
        break;
    default:
        /* Đơn giản: chưa cấu hình interrupt trong ví dụ này,
         * nên tạm thời không hỗ trợ các mode khác.
         */
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    map->port->PCR[map->pin] = pcr;

    return ARM_DRIVER_OK;
}

void HAL_GPIO_Write(ARM_GPIO_Pin_t pin, uint32_t val) {
    const pin_map_t *map = HAL_GPIO_GetMap(pin);
    if (map == NULL) {
        return;
    }

    if (val) {
        /* Ghi 1 -> set pin lên mức 1 */
        map->gpio -> PTOR = (1UL << map->pin);
    } else {
        /* Ghi 0 -> clear pin về 0 */
        map->gpio -> PTOR = (1UL << map->pin);
    }
}

uint32_t HAL_GPIO_Read(ARM_GPIO_Pin_t pin) {
    const pin_map_t *map = HAL_GPIO_GetMap(pin);
    if (map == NULL) {
        return 0U;
    }

    uint32_t pdir = map->gpio->PDIR;
    return (pdir >> map->pin) & 1UL;
}
