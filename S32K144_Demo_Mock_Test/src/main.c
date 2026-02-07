/**
 * @file main.c
 * @author Dung Van Pham (dungpv00@gmail.com)
 * @brief Main file for UART LED control example on S32K144 using CMSIS Driver
 * @version 0.1
 * @date 2025-12-20
 *
 * @copyright Copyright (c) 2025
 *
 */
/*============================================================================
 * INCLUDE
 =============================================================================*/
#include "S32K144.h"
#include "S32K144_features.h"
#include "device_registers.h"

#include "src/incl/app.h"
#include "src/incl/Bootloader.h"
#include "src/incl/clocks_and_modes.h"
#include "src/incl/Driver_GPIO.h"
#include "src/incl/Driver_USART.h"
#include "src/incl/NVIC.h"
#include "src/incl/Queue.h"
#include "src/incl/srec.h"
#include "src/incl/FLASH.h"

/*============================================================================
 * DEFINES
 =============================================================================*/
#define UART_CMD_BUFFER_SIZE   (256U)
#define BUTTON_1_PIN           (3U)   /* Virtual pin -> PTC12: BTN0      */

/* User application base address */
#define APP_BASE_ADDR          (0x0000A000UL)
#define APP_END_ADDR            0x0003FFFFUL   /* inclusive */

/*============================================================================
 * EXTERN DRIVER INSTANCES
 =============================================================================*/
extern ARM_DRIVER_USART        Driver_USART0;
extern ARM_DRIVER_GPIO         Driver_GPIO0;

/*============================================================================
 * Variables
 =============================================================================*/
static Queue_Srec_t             s_srec_queue;
static srec_record_t            rec;

/*============================================================================
 * PROTOTYPES
 =============================================================================*/
void App_InitClock(void);

/*============================================================================
 * FUNCTIONS
 =============================================================================*/
/**
 * @brief Disable the watchdog timer
 *
 */
void WDOG_disable(void)
{
    IP_WDOG->CNT   = 0xD928C520UL; /* Unlock watchdog */
    IP_WDOG->TOVAL = 0x0000FFFFUL; /* Maximum timeout value */
    IP_WDOG->CS    = 0x00002100UL; /* Disable watchdog */
}

/**
 * @brief Initialize system clock to 80 MHz
 *
 */
void App_InitClock(void)
{
    SOSC_init_8MHz();
    SPLL_init_160MHz();
    NormalRUNmode_80MHz();
}

/**
 * @brief Initialize LEDs (GPIOs)
 *
 */
void Boot_PinInit(void)
{
    /* Configure BUTTON 1 */
    Driver_GPIO0.Setup(BUTTON_1_PIN, NULL);
    Driver_GPIO0.SetDirection(BUTTON_1_PIN, ARM_GPIO_INPUT);
    Driver_GPIO0.SetPullResistor(BUTTON_1_PIN, ARM_GPIO_PULL_UP);
}

/**
 * @brief Check if boot mode is requested by button pin
 *
 * @return uint8_t 1: boot mode requested, 0: normal mode
 */
uint8_t Bootloader_IsBootModeByPin()
{
    return (Driver_GPIO0.GetInput(BUTTON_1_PIN));
}

/**
 * @brief Process one SREC line from queue: pop line, parse and handle it
 *
 * @param q Pointer to SREC queue
 * @param r Pointer to SREC record structure
 * @param entry_point Pointer to entry point variable
 */
static void Boot_ProcessOneQueuedLine(Queue_Srec_t *q, srec_record_t *r, uint32_t *entry_point)
{
    uint8_t  line[QUEUE_MAX_LINE_LENGTH];
    uint32_t line_len;

    if (QUEUE_STATUS_OK == Queue_Srec_Pop(q, line, QUEUE_MAX_LINE_LENGTH, &line_len))
    {
        if (line_len < QUEUE_MAX_LINE_LENGTH)
        {
            line[line_len] = '\0';
        }
        else
        {
            line[QUEUE_MAX_LINE_LENGTH - 1U] = '\0';
        }

        if (SREC_STATUS_OK == Srec_ParseSrecLine(line, r))
        {
            Bootloader_HandleRecord(r, entry_point);
        }
    }
}


/*===============================================================================
 * MAIN CODE
 ================================================================================*/
int main(void)
{
	ARM_DRIVER_USART *usart;
	int32_t           status;
	uint8_t           rx_char;
	uint8_t           c;
	uint8_t           cmd_buffer[UART_CMD_BUFFER_SIZE];
	uint32_t          cmd_index;
	uint8_t           line[QUEUE_MAX_LINE_LENGTH];
	uint32_t          line_len;
	uint32_t          entry_point;

	line_len   = 0U;

	usart = &Driver_USART0;

	WDOG_disable();
	App_InitClock();

	Boot_PinInit();

	/* Initialize USART */
	status = usart->Initialize(USART1_SignalEvent);

	ENABLE_INTERRUPTS();

	/* Set USART power to full, enable clock + Tx/Rx */
	if (ARM_DRIVER_OK == status)
	{
		status = usart->PowerControl(ARM_POWER_FULL);
	}

	if (ARM_DRIVER_OK == status)
	{
		status = usart->Control(ARM_USART_MODE_ASYNCHRONOUS |
				                ARM_USART_DATA_BITS_8       |
				                ARM_USART_PARITY_NONE       |
				                ARM_USART_STOP_BITS_1,
				                19200U);
	}

	/* Enable interrupt in Tx and Rx */
	if (ARM_DRIVER_OK == status)
	{
		status = usart->Control(ARM_USART_CONTROL_TX, 1U);
	}

	if (ARM_DRIVER_OK == status)
	{
		status = usart->Control(ARM_USART_CONTROL_RX, 1U);
	}

	if (ARM_DRIVER_OK != status)
	{
		while (1)
		{
			/* USART initialization failed */
		}
	}

	/* Check if need to enter bootloader mode */
	if (1U == Bootloader_IsBootModeByPin())
	{
		if (0U != Bootloader_IsUserAppValid(APP_BASE_ADDR))
		{
			Bootloader_JumpToUserApp(APP_BASE_ADDR);
		}
		else
		{
			/* Stay in bootloader */
		}
	}

	Queue_Srec_Init(&s_srec_queue);

	cmd_index   = 0U;
	entry_point = 0UL;

	cmd_buffer[0] = '\0';

	App_SendString("\r\n*** ERASE PROCESSING ***\r\n");

	Mem_43_INFLS_IPW_LoadAc();
	(void)Erase_Multi_Sector(APP_BASE_ADDR, (APP_END_ADDR - APP_BASE_ADDR)/FTFC_P_FLASH_SECTOR_SIZE);

	App_SendString("\r\n*** UART BOOTLOADER READY TO SENT ***\r\n");
	App_SendString("\r\n*** PLEASE SEND SREC FILE ***\r\n");

	status = usart->Receive(&rx_char, 1U);
	if (status != ARM_DRIVER_OK)
	{
		while (1)
		{
			/* USART receive initialization failed */
		}
	}

	while (1)
	{
	    uint32_t ev = usart_events;

	    if ((ev & ARM_USART_EVENT_RECEIVE_COMPLETE) != 0U)
	    {
	        /* clear event */
	        usart_events &= ~ARM_USART_EVENT_RECEIVE_COMPLETE;

	        c = rx_char;

	        (void)usart->Receive(&rx_char, 1U);

	        if ((c == (uint8_t)'\r') || (c == (uint8_t)'\n'))
	        {
	            if (cmd_index > 0U)
	            {
	                Queue_Status_t qst;

	                qst = Queue_Srec_Push(&s_srec_queue, cmd_buffer, cmd_index);

	                while (qst == QUEUE_STATUS_FULL)
	                {
	                    Boot_ProcessOneQueuedLine(&s_srec_queue, &rec, &entry_point);
	                    qst = Queue_Srec_Push(&s_srec_queue, cmd_buffer, cmd_index);
	                }

	                cmd_index = 0U;

	                while (Queue_Srec_Pop(&s_srec_queue, line, QUEUE_MAX_LINE_LENGTH, &line_len) == QUEUE_STATUS_OK)
	                {
	                    if (line_len < QUEUE_MAX_LINE_LENGTH) { line[line_len] = '\0'; }
	                    else { line[QUEUE_MAX_LINE_LENGTH - 1U] = '\0'; }

	                    if (SREC_STATUS_OK == Srec_ParseSrecLine(line, &rec))
	                    {
	                        Bootloader_HandleRecord(&rec, &entry_point);
	                    }
	                }
	            }
	            else
	            {
	                /* empty line -> ignore */
	            }
	        }
	        else
	        {
	            /* normal char */
	            if (cmd_index < (UART_CMD_BUFFER_SIZE - 1U))
	            {
	                cmd_buffer[cmd_index++] = c;
	            }
	            else
	            {
	                /* overflow: reset */
	                cmd_index = 0U;
	            }
	        }
	    }
	}

	return 0;
}
