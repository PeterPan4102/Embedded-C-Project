/**
 * @file main.c
 * @author Dung Pham Van (dungpv00@gmail.com)
 * @brief Demo chương trình điều khiển GPIO trên S32K144
 *        Sử dụng nút nhấn để bật/tắt LED theo chuẩn CMSIS.
 * @version 0.1
 * @date 2025-10-18
 *
 * @details
 *  - Bật clock cho PORTD.
 *  - Cấu hình:
 *      + PTD1, PTD2 làm input (nút nhấn, pull-up, active low).
 *      + PTD15, PTD16 làm output (LED, active low).
 *  - Khi nhấn nút:
 *      + Nút 1 → đảo trạng thái LED 1 (PTD15).
 *      + Nút 2 → đảo trạng thái LED 2 (PTD16).
 *  - Có delay 200ms chống dội phím (debounce).
 *
 * @note LED tắt ở mức logic 1 và sáng ở mức logic 0.
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "S32K144.h"
#include "port.h"
#include "gpio.h"

void delay_ms(uint32_t ms);

int main(void)
{
    /* Bật clock cho PORTD */
    PORT_EnableClock(IP_PORTD);

    /* Cấu hình chức năng input hoặc output */
    PORT_ConfigPin_t inCfg = {	.mux	= PORT_Mux_GPIO,
    							.pull	= PORT_Pull_Up,
								.irqMode= PORT_Irq_Disabled,
    };
    PORT_ConfigPin_t outCfg = {	.mux	= PORT_Mux_GPIO,
    							.pull	= PORT_Pull_Disabled,
								.irqMode= PORT_Irq_Disabled,
    };

    /* Cấu hình chân PTD1 và PTD2 làm GPIO input */
    PORT_ConfigPin(IP_PORTD, 1, &inCfg);
    PORT_ConfigPin(IP_PORTD, 2, &inCfg);

    /* Cấu hình chân PTD15 và PTD16 làm GPIO output */
    PORT_ConfigPin(IP_PORTD, 15, &outCfg);
    PORT_ConfigPin(IP_PORTD, 16, &outCfg);

    /* Cấu hình GPIO */
    GPIO_ConfigPin_t outCfgGPIO = { .dir		= GPIO_Output,
    								.initHigh	= true,
    };
    GPIO_ConfigPin(IP_PTD, 15, &outCfgGPIO);
    GPIO_ConfigPin(IP_PTD, 16, &outCfgGPIO);

    /* Tắt LED khi bắt đầu */
    GPIO_SetPin(IP_PTD, 15);
    GPIO_SetPin(IP_PTD, 16);

    while (1)
    {
    	if (GPIO_ReadPin(IP_PTD, 1) == 0) //Nút nhấn active low
    	{
    		delay_ms(200); // chống debounce nút nhấn
    		if (GPIO_ReadPin(IP_PTD, 1) == 0)
    		{
				GPIO_TogglePin(IP_PTD, 15);
				while (GPIO_ReadPin(IP_PTD, 1) == 0);

    		}
    	}

    	else if (GPIO_ReadPin(IP_PTD, 2) == 0) //Nút nhấn active low
    	{
    		delay_ms(200); // chống debounce nút nhấn
    		if (GPIO_ReadPin(IP_PTD, 2) == 0)
    		{
				GPIO_TogglePin(IP_PTD, 16);
				while (GPIO_ReadPin(IP_PTD, 2) == 0);

    		}
    	}
    }
}

void delay_ms(uint32_t ms)
{
    uint32_t i;
    for (i = 0; i < ms * 4000; i++) {
        NOP();
    }
}
