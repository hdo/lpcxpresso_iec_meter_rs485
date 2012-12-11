#include "LPC17xx.h"
#include "math_utils.h"
#include "drs155m.h"
#include "uart.h"
#include "queue.h"

extern volatile uint32_t UART1Count, UART1TxEmpty;
extern volatile uint8_t UART1Buffer[BUFSIZE];
extern volatile uint32_t UART1LastReceived;

// "/?<meter id>!\r\n"
char message_start[] = {'/', '?', '%', '!', '\r', '\n', '0'}; // NULL terminated string
// [ACK]0:1
char message_mode[] = {DATA_ACK, '0', ':', '1', '\r', '\n', '0'};
//[SOH]P1[STX](00000000)[ETX][BCC 0x61]
char message_login[] = {DATA_SOH, 'P', '1', DATA_STX, '(', '%', ')', DATA_ETX, '0'};
// [SOH]R1[STX]00000000()[ETX][BCC 0x63]
char message_read[] = {DATA_SOH, 'R', '1', DATA_STX, '%', '(', ')', DATA_ETX, '0'};
// [SOH]B0[ETX][BCC 0x71]
char message_exit[] = {DATA_SOH, 'B', '0', DATA_ETX, '0'};


uint8_t rs485out_buffer_data[RS485_OUTPUT_BUFFER_SIZE];
ringbuffer_t rs485out_rbuffer = {.buffer=rs485out_buffer_data, .head=0, .tail=0, .count=0, .size=RS485_OUTPUT_BUFFER_SIZE};

uint8_t iec_flag_busy = 0;
uint8_t iec_flag_data_available = 0;
uint8_t iec_flag_reading = 0;
uint8_t iec_flag_ready = 0;

uint8_t iec_read_address = 0;
char  read_address_string[9] = {0};
uint8_t iec_current_state = 0;
uint8_t iec_error_code = 0;
char* iec_current_meter_id = "";
uint8_t iec_received_data[DRS155M_MAX_RECEIVE_DATA_LENGTH];


uint8_t iec_get_error_code() {
	return iec_error_code;
}

/*
 * check whether there is an active communication
 */
uint8_t iec_is_busy() {
	return iec_flag_busy;
}

uint8_t iec_is_data_available() {
	return iec_flag_data_available;
}

uint8_t iec_is_ready() {
	return iec_flag_ready;
}

char* iec_read_data() {
	iec_flag_ready = 1;
	return "NA";
}


void set_address_string(uint8_t address) {
	read_address_string[8] = 0; // null termination
	uint8_t index = 7;
	for(; index >= 0; index--) {
		if (address > 0) {
			read_address_string[index] = address % 10 + '0';
			address /= 10;
		}
		else {
			read_address_string[index] = '0';
		}
	}
}

void iec_send(char* data) {
	queue_reset(&rs485out_rbuffer);
	while (*data) {
		queue_put(&rs485out_rbuffer, *data++);
	}
}

void iec_send_with_parameter(char* data, char* param) {
	queue_reset(&rs485out_rbuffer);
	uint8_t ch1, ch2;
	while (*data) {
		ch1 = *data++;
		if (ch2 == '%') {
			char* p1 = param;
			while (*p1) {
				ch2 = *p1++;
				queue_put(&rs485out_rbuffer, ch2);
			}
		}
		else {
			queue_put(&rs485out_rbuffer, ch1);
		}
	}
}

void iec_send_exit() {
	iec_flag_ready = 0;
	iec_flag_data_available = 0;
	iec_send(message_exit);
	iec_flag_busy = 0;
	iec_current_state = STATE_READY;
}

void iec_connect(char* meter_id) {
	if (!iec_flag_busy) {
		// send start
		iec_send_with_parameter(message_start, meter_id);
		iec_current_state = STATE_WAIT_IDENTIFICATION;
	}
}

void iec_disconnect() {
	if (!iec_flag_busy) {
		iec_send_exit();
	}
}

void iec_request_data_at_address(uint8_t address) {
	if (iec_flag_ready) {
		iec_flag_ready = 0;
		iec_read_address = address;
		set_address_string(address);
		iec_send_with_parameter(message_read, read_address_string);
		iec_current_state = STATE_WAIT_DATA_READ;
	}
}


void iec_prepare_send_mode() {
	// parse data
	if (1) {
		iec_send(message_mode);
		iec_current_state = STATE_WAIT_PASSWORD_PROMPT;
	}
	else {
		iec_send_exit();
	}
}

void iec_prepare_send_password() {
	// parse data
	if (1) {
		// send password command
		iec_send_with_parameter(message_login, DRS155M_DEFAULT_PASSWORD);
		iec_current_state = STATE_WAIT_PASSWORD_VERIFICATION;
	}
	else {
		iec_send_exit();
	}
}

void iec_prepare_password_verification() {
	// parse data for ACK
	if (1) {
		iec_flag_ready = 1;
	}
	else {
		iec_send_exit();
	}
}

void iec_parse_buffer() {

	LPC_UART1->IER = IER_THRE | IER_RLS; /* Disable RBR */

	// reset receive buffer
	uint8_t i, isdata, index = 0, d;
	for(i=0; i < DRS155M_MAX_RECEIVE_DATA_LENGTH; i++) {
		iec_received_data[i] = 0; // including null termination
	}

	// parse data
	for(i=0; i < UART1Count; i++) {
		d = UART1Buffer[i];
		if (d == '(') {
			isdata = 1;
			continue;
		}
		if (d == ')') {
			isdata = 0;
			continue;
		}
		if (isdata) {
			iec_received_data[index++] = d;
		}
	}
	UART1Count = 0;
	iec_flag_data_available = 1;

	LPC_UART1->IER = IER_THRE | IER_RLS | IER_RBR; /* Re-enable RBR */
}

void iec_init() {
	UARTInit(1, 9600);	 // baud rate setting, RS485
}

void process_iec(uint32_t ms_ticks) {
	UARTUpdateMsTicks(ms_ticks);

	if (queue_dataAvailable(&rs485out_rbuffer) && UARTTXReady(1)) {
		UARTSendByte(1, queue_read(&rs485out_rbuffer));
		if (queue_isEmpty(&rs485out_rbuffer)) {
			// if last transmission is done reset timer for time out
			iec_flag_reading = 1;
		}
	}

	if (iec_flag_reading) {
		iec_flag_reading = 0;
	    if (math_calc_diff(ms_ticks, UART1LastReceived) > 200) {
			// 2s time out
			// send exit message
	    	iec_send_exit();
		}
		else if (math_calc_diff(ms_ticks, UART1LastReceived) > 50) {
			// 500ms time out
			switch(iec_current_state) {
			case STATE_WAIT_IDENTIFICATION : iec_prepare_send_mode(); break;
			case STATE_WAIT_PASSWORD_PROMPT : iec_prepare_send_password(); break;
			case STATE_WAIT_PASSWORD_VERIFICATION : iec_prepare_password_verification(); break;
			case STATE_WAIT_DATA_READ : iec_parse_buffer(); break;
			default: break;
			}
		}
	}
}
