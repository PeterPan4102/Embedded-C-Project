/* main.c */
#include "S32K144.h"
#include "device_registers.h"
#include "Driver_GPIO.h"

extern ARM_DRIVER_GPIO Driver_GPIO0;

#define CPU_CLOCK_FREQ      8000000UL
#define CYCLES_PER_MS       (CPU_CLOCK_FREQ / 1000UL)

#define BTN_PRESSED         0u

/* Virtual pin mapping giống HAL_GPIO.c */
#define LED_RED_PIN         1U   /* PTD15  - LED đỏ */
#define LED_GREEN_PIN       2U   /* PTD16  - LED xanhL */

#define BUTTON_1_PIN        3U   /* PTC12 - BTN0      */
#define BUTTON_2_PIN        4U   /* PTC13 - BTN1      */

int main(void) {
    /* --- Cấu hình LED --- */
    Driver_GPIO0.Setup(LED_RED_PIN, NULL);                        /* Không dùng interrupt nên cb_event = NULL */
    Driver_GPIO0.SetDirection(LED_RED_PIN, ARM_GPIO_OUTPUT);
    Driver_GPIO0.SetOutputMode(LED_RED_PIN, ARM_GPIO_PUSH_PULL);
    /* Tắt LED: trên EVB, LED ON khi output = 0, OFF khi output = 1 */
    Driver_GPIO0.SetOutput(LED_RED_PIN, 1U);                      /* 1 = LED off */

    Driver_GPIO0.Setup(LED_GREEN_PIN, NULL);                        /* Không dùng interrupt nên cb_event = NULL */
    Driver_GPIO0.SetDirection(LED_GREEN_PIN, ARM_GPIO_OUTPUT);
    Driver_GPIO0.SetOutputMode(LED_GREEN_PIN, ARM_GPIO_PUSH_PULL);
    /* Tắt LED: trên EVB, LED ON khi output = 0, OFF khi output = 1 */
    Driver_GPIO0.SetOutput(LED_GREEN_PIN, 1U);                      /* 1 = LED off */

    /* --- Cấu hình Button --- */
    Driver_GPIO0.Setup(BUTTON_1_PIN, NULL);
    Driver_GPIO0.SetDirection(BUTTON_1_PIN, ARM_GPIO_INPUT);
    Driver_GPIO0.SetPullResistor(BUTTON_1_PIN, ARM_GPIO_PULL_UP);  /* input + pull-up */

    Driver_GPIO0.Setup(BUTTON_2_PIN, NULL);
    Driver_GPIO0.SetDirection(BUTTON_2_PIN, ARM_GPIO_INPUT);
    Driver_GPIO0.SetPullResistor(BUTTON_2_PIN, ARM_GPIO_PULL_UP);  /* input + pull-up */

    while (1)
    {
        uint32_t btn_1 = Driver_GPIO0.GetInput(BUTTON_1_PIN);
        uint32_t btn_2 = Driver_GPIO0.GetInput(BUTTON_2_PIN);

        /* Tùy cách mắc BTN:
         * Nếu dùng pull-up và nút kéo xuống GND:
         *  - Không nhấn: đọc = 1
         *  - Nhấn:      đọc = 0
         */

        if (btn_1 == BTN_PRESSED)
        {
        	DelayMs(20);

        	if (btn_1 == BTN_PRESSED)
        	{
                /* Nút nhấn -> bật LED (active low) */
                Driver_GPIO0.SetOutput(LED_RED_PIN, 1U);   /* 0 = LED on */
        	}
        } else
        {
            /* Nút không nhấn -> Do nothing */
        }

        if (btn_2 == BTN_PRESSED)
        {
        	DelayMs(20);
            if (btn_2 == BTN_PRESSED)
            {
                /* Nút nhấn -> bật LED (active low) */
                Driver_GPIO0.SetOutput(LED_GREEN_PIN, 1U);   /* 0 = LED on */
            }
        } else
        {
            /* Nút không nhấn -> Do nothing */
        }

        DelayMs(100);
    }

    return 0;
}

void DelayMs(uint32_t delay_in_ms)
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
