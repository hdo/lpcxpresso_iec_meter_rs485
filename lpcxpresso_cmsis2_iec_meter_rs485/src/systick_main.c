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
#include "math_utils.h"

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

	drs155m_init();

	logger_setEnabled(1);
	logger_logStringln("logger online ...");
	logger_logString("BUILD ID: ");
	logger_logStringln(VERSION_BUILD_ID);
	led_off(7);

	uint16_t loop_count = 0;
	uint8_t do_start = 0;

#define METER_COUNT 4

	uint8_t error_count[METER_COUNT] = {0,0,0};

	uint8_t current_meter_index = 0;


	drs155m_t power_meters[METER_COUNT];
	power_meters[0].meter_id = "001511420141";
	power_meters[1].meter_id = "001511420142";
	power_meters[2].meter_id = "001511420143";
	power_meters[3].meter_id = "001511420149";

	drs155m_t* current_power_meter = &power_meters[0];

	uint32_t delay_value = 0;

	while(1) {

		/* process logger */
		if (console_out_dataAvailable() && UARTTXReady(0)) {
			uint8_t iCounter;
			// fill transmit FIFO with 14 bytes
			for(iCounter = 0; iCounter < 14 && console_out_dataAvailable(); iCounter++) {
				UARTSendByte(0, console_out_read());
			}
		}

		led_process(msTicks);

		s0_process(msTicks);

		drs155m_process(msTicks);

		uint32_t triggerValue = s0_triggered(0);
		if (triggerValue) {
			logger_logString("s0_0:");
			logger_logNumberln(triggerValue);
			led_signal(1, 30, msTicks);
			if (do_start) {
				do_start = 0;
			}
			else {
				do_start = 1;
			}
		}

		if (drs155m_is_ready() && do_start && math_calc_diff(msTicks, delay_value) > 20) {
			loop_count++;
			current_meter_index++;
			if (current_meter_index >= METER_COUNT) {
				current_meter_index = 0;
			}
			current_power_meter = &power_meters[current_meter_index];
			drs155m_request_data(current_power_meter);
		}

		if (drs155m_is_data_available()) {
			logger_logStringln("Meter data: ");
			logger_logNumberln(current_power_meter->voltage);
			logger_logNumberln(current_power_meter->ampere);
			logger_logNumberln(current_power_meter->frequency);
			logger_logNumberln(current_power_meter->active_power);
			logger_logNumberln(current_power_meter->reactive_power);
			logger_logNumberln(current_power_meter->total_energy);
			logger_logString("operation took ");
			logger_logNumber(drs155m_get_duration());
			logger_logStringln(" ticks");
			logger_logString("current loop: ");
			logger_logNumberln(loop_count);
			logger_logString("error count 0: ");
			logger_logNumberln(error_count[0]);
			logger_logString("error count 1: ");
			logger_logNumberln(error_count[1]);
			logger_logString("error count 2: ");
			logger_logNumberln(error_count[2]);
			logger_logString("error count 3: ");
			logger_logNumberln(error_count[3]);
			drs155m_reset();
			delay_value = msTicks;
		}

		if (drs155m_is_error()) {
			logger_logStringln("error reading meter data");
			error_count[current_meter_index]++;
			drs155m_reset();
			delay_value = msTicks;
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
