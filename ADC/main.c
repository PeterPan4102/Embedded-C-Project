/**
 * @file main.c
 * @author Dung Pham Van (dungpv00@gmail.com)
 * @brief Ví dụ đọc giá trị ADC và điều khiển LED dựa trên mức điện áp đầu vào
 *
 * @version 1.0
 * @date 2025-10-26
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "S32K144.h"
#include "port.h"
#include "gpio.h"

static void Clock_Init (void) {
	/* Bật clock cho PORTD */
	PORT_EnableClock(IP_PORTD);

	/* Bật clock cho PORTB */
	PORT_EnableClock(IP_PORTB);
}

static void LED_Init (void) {
    /* Cấu hình chức năng hoặc output */
	PORT_ConfigPin_t outCfg = {	.mux	= PORT_Mux_GPIO,
	    							.pull	= PORT_Pull_Disabled,
									.irqMode= PORT_Irq_Disabled,
	};

    /* Cấu hình chân PTD0, PTD15 và PTD16 làm GPIO output */
    PORT_ConfigPin(IP_PORTD, 0, &outCfg);
    PORT_ConfigPin(IP_PORTD, 15, &outCfg);
    PORT_ConfigPin(IP_PORTD, 16, &outCfg);

    /* Cấu hình GPIO */
    GPIO_ConfigPin_t outCfgGPIO = { .dir		= GPIO_Output,
        								.initHigh	= true,
    };

    GPIO_ConfigPin(IP_PTD, 0, &outCfgGPIO);
    GPIO_ConfigPin(IP_PTD, 15, &outCfgGPIO);
    GPIO_ConfigPin(IP_PTD, 16, &outCfgGPIO);

    /* Tắt LED khi bắt đầu */
    GPIO_SetPin(IP_PTD, 0);	 //BLUE
    GPIO_SetPin(IP_PTD, 15); //RED
    GPIO_SetPin(IP_PTD, 16); //GREEN
}

static void Config_ADC(void){
	IP_PCC -> PCCn[PCC_ADC0_INDEX] = PCC_PCCn_PCS (1) 	// chọn 1 vì SOSCDIV2_CLK là 1 = 8 MHz
									| PCC_PCCn_CGC_MASK; // bật interface clock

	/* Tham chiếu REFSEL. Chọn software trigger */
	IP_ADC0 -> SC2 = ADC_SC2_REFSEL (0)  // 0 là để mặc định VREFH/VREFL (chân VREFH nối 3.3V, VREFL nối GND
					|ADC_SC2_ADTRG(0);	// chọn software trigger

	/* Nguồn và chia clock ADC. Độ phân giải và thời gian lấy mẫu */
	IP_ADC0 -> CFG1 = ADC_CFG1_ADICLK(1)
					| ADC_CFG1_ADIV(1)	// chia 2 -> ADCK = 4MHz
					| ADC_CFG1_MODE(1); // 12-bit mode

	/* Calibration & Adveraging */
	IP_ADC0 -> SC3 = ADC_SC3_CAL_MASK; 			// CAL = 1
	while(IP_ADC0 -> SC3 & ADC_SC3_CAL_MASK){}	// đợi xong

	IP_ADC0 -> SC3 |= ADC_SC3_AVGE_MASK | ADC_SC3_AVGS(1); // AVGS=1 -> 4 mẫu
}

static uint16_t ADC0_ReadRaw(uint8_t ch) {
	 /* Ghi SC1A để bắt đầu chuyển đổi (software trigger) */
	 IP_ADC0->SC1[0] = ADC_SC1_ADCH(ch);  // AIEN=0 (polling)

	 /* Đợi COCO = 1 */
	 while (!(IP_ADC0->SC1[0] & ADC_SC1_COCO_MASK)) {}

	 /* Đọc kết quả 12-bit → tự clear COCO của SC1A */
	 return (uint16_t)IP_ADC0->R[0];
}

static uint32_t ADC0_Read_mV(uint8_t ch)
{
    uint16_t code = ADC0_ReadRaw(ch);
    /* mV = code * Vref / 4095 */
    return ( (uint32_t)code * (uint32_t)5000u ) / (uint32_t)4095u;
}

typedef enum { LED_NONE=0, LED_RED, LED_GREEN, LED_BLUE } LedSel_t;

static void LED_Show(LedSel_t l){
    /* Tắt LED khi bắt đầu */
    GPIO_SetPin(IP_PTD, 0);	 //BLUE
    GPIO_SetPin(IP_PTD, 15); //RED
    GPIO_SetPin(IP_PTD, 16); //GREEN

    switch(l){
    case LED_RED: GPIO_ClearPin(IP_PTD, 15);
    case LED_BLUE: GPIO_ClearPin(IP_PTD, 0);
    case LED_GREEN: GPIO_ClearPin(IP_PTD, 16);
    default: break;
    }
}

int main(void)
{
    Clock_Init();
    LED_Init();
    Config_ADC();

    for(;;){
        uint32_t mv = ADC0_Read_mV(4); //ADC0_SE4 là chân PTB0

        if      (mv >= 3750u) LED_Show(LED_RED);
        else if (mv >= 2500u) LED_Show(LED_GREEN);
        else if (mv >= 1250u) LED_Show(LED_BLUE);
        else                  LED_Show(LED_NONE);
    }
}
