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

    while (1)
    {
        /* Bật LED */
        IP_PTD->PCOR = (1 << 0);   // ghi 0 để bật LED (tùy board, có thể ngược)
        delay_ms(3000);         // delay 3 giây

        /* Tắt LED */
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
