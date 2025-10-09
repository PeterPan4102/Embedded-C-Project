#include "S32K144.h"

void delay_ms(uint32_t ms);

int main(void)
{
    /* Bật clock cho PORTD */
    PCC->PCCn[PCC_PORTD_INDEX] |= PCC_PCCn_CGC_MASK;

    /* Cấu hình chân PTD0 làm GPIO output */
    PTD->PDDR |= (1 << 0);          // output
    PORTD->PCR[0] = PORT_PCR_MUX(1); // MUX = 1 (GPIO)

    while (1)
    {
        /* Bật LED */
        PTD->PCOR = (1 << 0);   // ghi 0 để bật LED (tùy board, có thể ngược)
        delay_ms(3000);         // delay 3 giây

        /* Tắt LED */
        PTD->PSOR = (1 << 0);
        delay_ms(3000);
    }
}

void delay_ms(uint32_t ms)
{
    uint32_t i;
    for (i = 0; i < ms * 4000; i++) {
        __NOP();
    }
}