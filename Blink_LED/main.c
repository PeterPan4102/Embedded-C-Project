#include "S32K144.h"
#include "device_registers.h"            /* include peripheral declarations S32K144 */

void delay_ms(uint32_t ms);

int main(void)
{
    /* Bật clock cho PORTD */
    IP_PCC->PCCn[PCC_PORTD_INDEX] |= PCC_PCCn_CGC_MASK;

    /* Cấu hình chân PTD0 làm GPIO output */
    IP_PTD->PDDR |= (1 << 0);          // output
    IP_PORTD->PCR[0] = PORT_PCR_MUX(1); // MUX = 1 (GPIO)
    IP_PTD->PDDR |= (1 << 15);          // output
    IP_PORTD->PCR[15] = PORT_PCR_MUX(1); // MUX = 1 (GPI15)
    IP_PTD->PDDR |= (1 << 16);          // output
    IP_PORTD->PCR[16] = PORT_PCR_MUX(1); // MUX = 1 (GPI16)

    while (1)
    {
        /* Bật LED RED*/
        IP_PTD->PCOR = (1 << 15);   // ghi 0 để bật LED
        delay_ms(3000);         // delay 3 giây

        /* Tắt LED RED*/
        IP_PTD->PSOR = (1 << 15);
        delay_ms(3000);

        /* Bật LED GREEN*/
        IP_PTD->PCOR = (1 << 16);   // ghi 0 để bật LED
        delay_ms(3000);         // delay 3 giây

        /* Tắt LED GREEN*/
        IP_PTD->PSOR = (1 << 16);
        delay_ms(3000);

        /* Bật LED BLUE*/
        IP_PTD->PCOR = (1 << 0);   // ghi 0 để bật LED
        delay_ms(3000);         // delay 3 giây

        /* Tắt LED BLUE*/
        IP_PTD->PSOR = (1 << 0);
        delay_ms(3000);
    }
}

void delay_ms(uint32_t ms)
{
    uint32_t i;
    for (i = 0; i < ms * 4000; i++) {
        NOP();
    }
}
