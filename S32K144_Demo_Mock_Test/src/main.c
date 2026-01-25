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

#define APP_START_ADDR          0x0000A000UL
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

uint8_t Bootloader_IsBootModeByPin()
{
    return (Driver_GPIO0.GetInput(BUTTON_1_PIN));
}

/*===============================================================================
 * MAIN CODE
 ================================================================================*/
int main(void)
{
	ARM_DRIVER_USART *usart;
	int32_t           status;
	uint8_t           rx_char;
	uint8_t           cmd_buffer[UART_CMD_BUFFER_SIZE];
	uint32_t          cmd_index;
	uint8_t           line[QUEUE_MAX_LINE_LENGTH];
	uint32_t          line_len;
	uint8_t           line_ready;
	uint32_t          entry_point;

	line_ready = 0U;
	line_len   = 0U;

	usart = &Driver_USART0;

	WDOG_disable();
	App_InitClock();

	Boot_PinInit();

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

	status = usart->Initialize(NULL);
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
				2400U);
	}

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
		}
	}

	cmd_index   = 0U;
	entry_point = 0UL;

	cmd_buffer[0] = '\0';

	App_SendString("\r\n*** UART BOOTLOADER READY TO SENT ***\r\n");
	App_SendPrompt();

	while (1)
	{
		status = usart->Receive(&rx_char, 1U);

		if (ARM_DRIVER_OK == status)
		{
			/* Ignore CR, treat LF as end-of-line */
			if ((uint8_t)'\r' == rx_char)
			{
				/* do nothing */
			}
			else if ((uint8_t)'\n' == rx_char)
			{
				if (cmd_index > 0U)
				{
					Queue_Status_t qst;

					qst = Queue_Srec_Push(&s_srec_queue, cmd_buffer, cmd_index);

					if (qst == QUEUE_STATUS_FULL)
					{
						if(0U == line_ready)
						{
							if(QUEUE_STATUS_OK == Queue_Srec_Pop(&s_srec_queue, line, QUEUE_MAX_LINE_LENGTH, &line_len))
							{
								line_ready = 1U;
							}
						}
						else
						{
							if (SREC_STATUS_OK == Srec_ParseSrecLine(line, &rec))
							{
								Bootloader_HandleRecord(&rec, &entry_point);
							}
						}

						/* retry push new line */
						(void)Queue_Srec_Push(&s_srec_queue, cmd_buffer, cmd_index);
					}

					/* clear buffer for next line */
					for (uint32_t i = 0U; i < cmd_index; i++)
					{
						cmd_buffer[i] = 0U;
					}
					cmd_index = 0U;
				}
			}

			else
			{
				if (cmd_index < UART_CMD_BUFFER_SIZE)
				{
					cmd_buffer[cmd_index] = rx_char;
					cmd_index++;
				}
				else
				{
					/* Buffer overflow, reset */
					cmd_index = 0U;
					App_SendString("\r\nError: Command too long\r\n");
					App_SendPrompt();
				}
			}
		}

		if(0U == line_ready)
		{
			if(QUEUE_STATUS_OK == Queue_Srec_Pop(&s_srec_queue, line, QUEUE_MAX_LINE_LENGTH, &line_len))
			{
				line_ready = 1;

			}
		}
		else if(1U == line_ready)
		{
			if (SREC_STATUS_OK == Srec_ParseSrecLine(line, &rec))
			{
				Bootloader_HandleRecord(&rec, &entry_point);
			}
			else
			{
				/* Do nothing */
			}
			line_ready = 0U;
		}
	}

	return 0;
}


