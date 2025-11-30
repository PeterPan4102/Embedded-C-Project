/**
 * @file    main.c
 * @author
 * @brief   Application entry point.
 * @details Hiện thực driver cho gpio và ứng dụng vào chương trình nhấp nháy led dựa trên sự kiện từ nút nhấn,
 * áp dụng các kiến thức về ngắt và gpio.
 */


#include "S32K144.h"
#include <stdio.h>

#include "Driver_GPIO.h"

extern ARM_DRIVER_GPIO Driver_GPIO0;

#if defined (__ghs__)
    #define __INTERRUPT_SVC  __interrupt
    #define __NO_RETURN _Pragma("ghs nowarning 111")
#elif defined (__ICCARM__)
    #define __INTERRUPT_SVC  __svc
    #define __NO_RETURN _Pragma("diag_suppress=Pe111")
#elif defined (__GNUC__)
    #define __INTERRUPT_SVC  __attribute__ ((interrupt ("SVC")))
    #define __NO_RETURN
#else
    #define __INTERRUPT_SVC
    #define __NO_RETURN
#endif

#define CPU_CLOCK_FREQ      8000000UL
#define CYCLES_PER_MS       (CPU_CLOCK_FREQ / 1000UL)

#define PIN_BUTTON1	 			0
#define PIN_BUTTON2	 			1
#define PIN_LED_RED				2
#define PIN_LED_GREEN			3

#define BUTTON_PRESSED			1
#define BUTTON_RELEASE			0

#define LED_OFF					0
#define LED_ON					1

void System_DelayMs(uint32_t delay_in_ms);
void App_GPIO_CallBack(uint32_t pin, uint32_t event);

volatile uint8_t g_is_button1_press_flag = BUTTON_RELEASE;
volatile uint8_t g_is_button2_press_flag = BUTTON_RELEASE;



int main(void) {
	ARM_DRIVER_GPIO* gpio_drv = &Driver_GPIO0;

	gpio_drv->Setup(PIN_LED_RED, NULL);
	gpio_drv->SetDirection(PIN_LED_RED, ARM_GPIO_OUTPUT);
	gpio_drv->SetOutput(PIN_LED_RED, 0);

	gpio_drv->Setup(PIN_LED_GREEN, NULL);
	gpio_drv->SetDirection(PIN_LED_GREEN, ARM_GPIO_OUTPUT);
	gpio_drv->SetOutput(PIN_LED_GREEN, 0);

	gpio_drv->Setup(PIN_BUTTON1, App_GPIO_CallBack);
	gpio_drv->SetDirection(PIN_BUTTON1, ARM_GPIO_INPUT);
	gpio_drv->SetPullResistor(PIN_BUTTON1, ARM_GPIO_PULL_NONE);
	gpio_drv->SetEventTrigger(PIN_BUTTON1, ARM_GPIO_TRIGGER_FALLING_EDGE);

	gpio_drv->Setup(PIN_BUTTON2, App_GPIO_CallBack);
	gpio_drv->SetDirection(PIN_BUTTON2, ARM_GPIO_INPUT);
	gpio_drv->SetPullResistor(PIN_BUTTON2, ARM_GPIO_PULL_NONE);
	gpio_drv->SetEventTrigger(PIN_BUTTON2, ARM_GPIO_TRIGGER_FALLING_EDGE);

	uint8_t led_green_status = LED_OFF;
	uint8_t led_red_status = LED_OFF;

	for(;;)
	{
		if(BUTTON_PRESSED == g_is_button1_press_flag)
		{
			g_is_button1_press_flag = BUTTON_RELEASE;
			led_red_status = (led_red_status == LED_OFF) ? LED_ON : LED_OFF;

			gpio_drv->SetOutput(PIN_LED_RED, led_red_status);

		}

		if(BUTTON_PRESSED == g_is_button2_press_flag)
		{
			g_is_button2_press_flag = BUTTON_RELEASE;
			led_green_status = (led_green_status == LED_OFF) ? LED_ON : LED_OFF;

			gpio_drv->SetOutput(PIN_LED_GREEN, led_green_status);

		}

		System_DelayMs(100);

	}

    /* to avoid the warning message for GHS and IAR: statement is unreachable*/
    __NO_RETURN
    return 0;

}

void System_DelayMs(uint32_t delay_in_ms)
{
    volatile uint32_t loop_count;
    while(delay_in_ms--)
    {
        for(loop_count = 0; loop_count < CYCLES_PER_MS; loop_count++)
        {
            __asm("nop");
        }
    }
}

void App_GPIO_CallBack(uint32_t pin, uint32_t event)
{
	if (PIN_BUTTON1 == pin)
	{
		g_is_button1_press_flag = BUTTON_PRESSED;

	}
	else if (PIN_BUTTON2 == pin)
	{
		g_is_button2_press_flag = BUTTON_PRESSED;

	}
}
