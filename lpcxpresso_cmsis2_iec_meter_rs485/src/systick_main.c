// CMSIS headers required for setting up SysTick Timer
#include "LPC17xx.h"

#include <cr_section_macros.h>
#include <NXP/crp.h>

// Variable to store CRP value in. Will be placed automatically
// by the linker when "Enable Code Read Protect" selected.
// See crp.h header for more information
__CRP const unsigned int CRP_WORD = CRP_NO_CRP ;

#include "leds.h"
#include "uart.h"
#include "logger.h"
#include "console_out.h"
#include "s0_input.h"
#include "drs155m.h"
#include "version.h"

volatile uint32_t msTicks; // counter for 1ms SysTicks
extern volatile unsigned int eint3_count;
extern volatile uint8_t UART0Count, UART1Count, UART2Count, UART3Count, UART0TxEmpty, UART1TxEmpty, UART2TxEmpty;
extern volatile uint8_t UART0Buffer[BUFSIZE], UART1Buffer[BUFSIZE], UART2Buffer[BUFSIZE];
extern volatile uint32_t UART1LastReceived;



// ****************
//  SysTick_Handler - just increment SysTick counter
void SysTick_Handler(void) {
  msTicks++;
}

// ****************
// systick_delay - creates a delay of the appropriate number of Systicks (happens every 10 ms)
__INLINE static void systick_delay (uint32_t delayTicks) {
  uint32_t currentTicks;

  currentTicks = msTicks;	// read current tick counter
  // Now loop until required number of ticks passes.
  while ((msTicks - currentTicks) < delayTicks);
}


// ****************
int main(void) {
	
	// Setup SysTick Timer to interrupt at 10 msec intervals
	if (SysTick_Config(SystemCoreClock / 100)) {
	    while (1);  // Capture error
	}

	led_init();	// Setup GPIO for LED2
	led_on(0);		// Turn LED2 on
	//led_on(0);
	//led_on(1);

	systick_delay(100);
	led_off(0);


	UARTInit(0, 115200); // baud rate setting
	UARTInit(2, 9600);	 // baud rate setting, PC
	UARTSendCRLF(0);
	UARTSendCRLF(0);
	UARTSendStringln(0, "UART2 online ...");

	iec_init();

	logger_setEnabled(1);
	logger_logStringln("logger online ...");
	logger_logString("BUILD ID: ");
	logger_logStringln(VERSION_BUILD_ID);
	led_off(7);

	uint8_t iCounter;

	uint8_t request_data_table[] = {0, 1, 2, 3, 4, 10, 31, 32};
	uint8_t current_data_request_index = 0;

	while(1) {

		/* process logger */
		if (console_out_dataAvailable() && UARTTXReady(0)) {
			// fill transmit FIFO with 14 bytes
			for(iCounter = 0; iCounter < 14 && console_out_dataAvailable(); iCounter++) {
				UARTSendByte(0, console_out_read());
			}
		}

		process_leds(msTicks);

		process_s0(msTicks);

		process_iec(msTicks);

		uint32_t triggerValue = s0_triggered(0);
		if (triggerValue) {
			logger_logString("s0_0:");
			logger_logNumberln(triggerValue);
			led_signal(1, 30, msTicks);
			if (iec_get_connect_status() == CON_STAT_DISCONNECTED) {
				iec_connect("001511420144");
				//iec_connect("");
			}

			if (iec_get_connect_status() == CON_STAT_CONNECTED) {
				logger_logStringln("connected!");

				if (iec_is_ready()) {
					if (current_data_request_index < 8) {
						iec_request_data_at_address(request_data_table[current_data_request_index++]);
					}
				}

				if (iec_is_data_available()) {
					logger_logString("data at address: ");
					logger_logNumberln(iec_get_current_address());
					uint32_t data = iec_get_data_as_int();
					logger_logString("data as int: ");
					logger_logNumberln(data);
					logger_logString("data as string: ");
					logger_logStringln(iec_get_data_as_string());
					iec_clear_data();
				}
			}

		}


		triggerValue = s0_triggered(1);
		if (triggerValue) {
			logger_logString("s0_1:");
			logger_logNumberln(triggerValue);
			led_signal(2, 30, msTicks);
		}


		/* logger echo */
		if ( UART0Count != 0 ) {
			led2_on();
			LPC_UART0->IER = IER_THRE | IER_RLS;				/* Disable RBR */

			int i = 0;
			for(; i < UART0Count; i++) {
				logger_logByte(UART0Buffer[i]);
			}
			UART0Count = 0;
			LPC_UART0->IER = IER_THRE | IER_RLS | IER_RBR;		/* Re-enable RBR */
			led2_off();
		}



	}
	return 0 ;
}
