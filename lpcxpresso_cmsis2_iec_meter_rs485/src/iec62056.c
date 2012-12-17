#include "LPC17xx.h"
#include "math_utils.h"
#include "iec62056.h"
#include "uart.h"
#include "queue.h"
#include "logger.h"

extern volatile uint8_t UART1Count, UART1TxEmpty;
extern volatile uint8_t UART1Buffer[BUFSIZE];
extern volatile uint32_t UART1LastReceived;

// "/?<meter id>!\r\n"
const char message_start[] = {'/', '?', '%', '!', '\r', '\n', 0}; // NULL terminated string
// [ACK]0:1
const char message_mode[] = {DATA_ACK, '0', ':', '1', '\r', '\n', 0};
//[SOH]P1[STX](00000000)[ETX][BCC 0x61]
const char message_login[] = {DATA_SOH, 'P', '1', DATA_STX, '(', '%', ')', DATA_ETX, 0};
// [SOH]R1[STX]00000000()[ETX][BCC 0x63]
const char message_read[] = {DATA_SOH, 'R', '1', DATA_STX, '%', '(', ')', DATA_ETX, 0};
// [SOH]B0[ETX][BCC 0x71]
const char message_exit[] = {DATA_SOH, 'B', '0', DATA_ETX, 0};

const uint8_t hex_data[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

uint8_t rs485out_buffer_data[RS485_OUTPUT_BUFFER_SIZE];
ringbuffer_t rs485out_rbuffer = {.buffer=rs485out_buffer_data, .head=0, .tail=0, .count=0, .size=RS485_OUTPUT_BUFFER_SIZE};

uint8_t iec_connect_status = CON_STAT_DISCONNECTED;
uint8_t iec_flag_data_available = 0;
uint8_t iec_flag_ready = 0;
uint8_t iec_flag_error = 0;
uint8_t iec_flag_reading = 0;
uint8_t iec_flag_expect_response = 0;

uint8_t logging_bcc_out = 0;
uint8_t logging_bcc_in = 0;

uint8_t iec_current_address = 0;
char read_address_string[9];
uint8_t iec_current_state = 0;
char* iec_current_meter_id = "";
uint8_t iec_received_data[METER_MAX_RECEIVE_DATA_LENGTH];
uint32_t iec_last_active = 0;


uint8_t iec_is_data_available() {
	return iec_flag_data_available;
}

uint8_t iec_is_ready() {
	return iec_flag_ready;
}

uint8_t iec_is_error_occured() {
	return iec_flag_error;
}

uint8_t iec_get_connect_status() {
	return iec_connect_status;
}

char* iec_get_data_as_string() {
	return (char*) iec_received_data;
}

uint32_t iec_get_data_as_int() {
	uint32_t value = 0;
	uint8_t d;
	char* temp = (char*) iec_received_data;
	while(*temp) {
		d = *temp++;
		if (d >= '0' && d <= '9') {
			value *= 10;
			value += (d - '0');
		}
	}
	return value;
}

void iec_clear_data() {
	iec_flag_data_available = 0;
	iec_flag_ready = 1;
}

uint8_t iec_get_current_address(){
	return iec_current_address;
}

void set_address_string(uint8_t address) {
	read_address_string[8] = 0; // null termination
	uint8_t index;

	// init string
	for(index = 0; index < 8; index++) {
		read_address_string[index] = '0';
	}

	/* WARNING: don't use index >= 0 since index is unsigned!!!
	 * Since we're are only using uint8_t (255) there should not be no a problem
	 */
	for(index = 7; index > 0; index--) {
		if (address > 0) {
			read_address_string[index] = (address % 10) + '0';
			address /= 10;
		}
		else {
			read_address_string[index] = '0';
		}
	}
}


void log_iec_data(uint8_t data, uint8_t is_out, uint8_t is_bcc) {
	uint8_t p_o = '[';
	uint8_t p_c = ']';

	if (!is_out) {
		p_o = '{';
		p_c = '}';
	}

	if (is_bcc) {
		uint8_t i1 = data & 0xF;
		uint8_t i2 = data >> 4;
		logger_logByte(p_o);
		logger_logString("BCC 0x");
		logger_logByte(hex_data[i2]);
		logger_logByte(hex_data[i1]);
		logger_logByte(p_c);
		logger_logCRLF();
	}
	else {
		switch(data) {
		case 0x01 : logger_logByte(p_o); logger_logString("SOH"); logger_logByte(p_c); break;
		case 0x02 : logger_logByte(p_o); logger_logString("STX"); logger_logByte(p_c); break;
		case 0x03 : logger_logByte(p_o); logger_logString("ETX"); logger_logByte(p_c); break;
		case 0x04 : logger_logByte(p_o); logger_logString("EOT"); logger_logByte(p_c); break;
		case 0x06 : logger_logByte(p_o); logger_logString("ACK"); logger_logByte(p_c); break;
		case 0x15 : logger_logByte(p_o); logger_logString("NAK"); logger_logByte(p_c); break;
		default:
			logger_logByte(data);
		}
	}
}

void log_incomming_data() {
	logger_logStringln("<<<");
	uint8_t has_bcc = 0;
	if (UART1Count > 1) {
		has_bcc = UART1Buffer[UART1Count-2] == DATA_ETX || UART1Buffer[UART1Count-2] == DATA_EOT;
	}
	uint8_t i;
	for(i=0; i < UART1Count; i++) {
		uint8_t data = UART1Buffer[i];
		if (has_bcc && i == (UART1Count - 1)) {
			log_iec_data(data, 0, 1);
		}
		else {
			log_iec_data(data, 0, 0);
		}
	}
	logger_logCRLF();
}


void log_outgoing_data() {
	logger_logStringln(">>>");

	uint8_t count = queue_count(&rs485out_rbuffer);
	uint8_t has_bcc = 0;
	if (UART1Count > 1) {
		has_bcc = queue_peek(&rs485out_rbuffer, count-2) == DATA_ETX || queue_peek(&rs485out_rbuffer, count-2) == DATA_EOT;
	}

	uint8_t i;
	for(i=0; i < count; i++) {
		uint8_t data = queue_peek(&rs485out_rbuffer, i);
		if (has_bcc && i == (count - 1)) {
			log_iec_data(data, 1, 1);
		}
		else {
			log_iec_data(data, 1, 0);
		}
	}
	logger_logCRLF();
}

void iec_send(const char* data, uint8_t expect_response) {
	iec_flag_expect_response = expect_response;
	queue_reset(&rs485out_rbuffer);
	while (*data) {
		queue_put(&rs485out_rbuffer, *data++);
	}
	log_outgoing_data();
}

void iec_send_with_parameter(const char* data, char* param, uint8_t expect_response, uint8_t send_bcc) {
	iec_flag_expect_response = expect_response;
	// also send BCC
	queue_reset(&rs485out_rbuffer);
	uint8_t ch1, ch2, bcc = 0, start_bcc = 0;

	while (*data) {
		ch1 = *data++;
		if (ch1 == '%') {
			char* p1 = param;
			while (*p1) {
				ch2 = *p1++;
				queue_put(&rs485out_rbuffer, ch2);
				if (start_bcc) {
					bcc ^= ch2;
				}
			}
		}
		else {
			queue_put(&rs485out_rbuffer, ch1);
			if (start_bcc) {
				bcc ^= ch1;
			}
		}
		// skip first byte for bcc calculation
		start_bcc = 1;
	}
	if (send_bcc) {
		// send BCC
		queue_put(&rs485out_rbuffer, bcc);
	}
	log_outgoing_data();
}

void iec_send_exit() {
	iec_flag_ready = 0;
	iec_flag_data_available = 0;
	iec_flag_error = 0;
	iec_flag_reading = 0;
	iec_send(message_exit, 0);
	iec_connect_status = CON_STAT_DISCONNECTED;
	iec_current_state = STATE_DISCONNECTED;
}

void iec_connect(char* meter_id) {
	if (iec_connect_status == CON_STAT_DISCONNECTED) {
		// send start
		iec_send_with_parameter(message_start, meter_id, 1, 0);
		iec_current_state = STATE_WAIT_IDENTIFICATION;
		iec_connect_status = CON_STAT_CONNECTING;
		logger_logStringln("done sending start command");
	}
}

void iec_disconnect() {
	if (iec_connect_status == CON_STAT_CONNECTED) {
		iec_send_exit();
	}
}

void iec_request_data_at_address(uint8_t address) {
	if (iec_flag_ready) {
		logger_logString("request data at address ");
		iec_flag_ready = 0;
		iec_flag_data_available = 0;
		iec_flag_error = 0;
		iec_current_address = address;
		set_address_string(address);
		logger_logStringln(read_address_string);
		iec_send_with_parameter(message_read, read_address_string, 1, 1);
		iec_current_state = STATE_WAIT_DATA_READ;
	}
}


void iec_prepare_send_mode() {
	// check wether response is like "/YTL....."

	// expecting at leass 4 characters
	if (UART1Count > 3 &&
			UART1Buffer[0] == '/' &&
			UART1Buffer[1] == 'Y' &&
			UART1Buffer[2] == 'T' &&
			UART1Buffer[3] == 'L') {

		iec_send(message_mode, 1);
		iec_current_state = STATE_WAIT_PASSWORD_PROMPT;
		iec_connect_status = CON_STAT_CONNECTED;
	}
	else {
		iec_send_exit();
	}
}

void iec_prepare_send_password() {
	if (UART1Count > 3 &&
			UART1Buffer[0] == DATA_SOH &&
			UART1Buffer[1] == 'P' &&
			UART1Buffer[2] == '0' &&
			UART1Buffer[UART1Count-1] == 0x60
			)
	{
		// send password command
		iec_send_with_parameter(message_login, METER_DEFAULT_PASSWORD, 1, 1);
		iec_current_state = STATE_WAIT_PASSWORD_VERIFICATION;
	}
	else {
		iec_send_exit();
	}
}

void iec_prepare_password_verification() {
	if (UART1Count == 1 &&
			UART1Buffer[0] == DATA_ACK)  {
		iec_flag_ready = 1;
	}
	else {
		iec_send_exit();
	}
}

void iec_parse_buffer() {
	// reset receive buffer
	uint8_t i, isdata = 0, index = 0, d, parseCount = 0;
	for(i=0; i < METER_MAX_RECEIVE_DATA_LENGTH; i++) {
		iec_received_data[i] = 0; // including null termination
	}

	// parse data
	for(i=0; i < UART1Count; i++) {
		d = UART1Buffer[i];
		if (d == '(') {
			isdata = 1;
			parseCount++;
			continue;
		}
		if (d == ')') {
			isdata = 0;
			parseCount++;
			continue;
		}
		if (isdata) {
			iec_received_data[index++] = d;
		}
	}

	if (index > 0 && parseCount == 2) {
		iec_flag_data_available = 1;
	}
	else {
		// parse error
		iec_flag_error = 1;
	}
}

void iec_init() {
	UARTInit(1, 9600);	 // baud rate setting, RS485
}

void iec_process(uint32_t ms_ticks) {
	UARTUpdateMsTicks(ms_ticks);

	if (queue_dataAvailable(&rs485out_rbuffer) && UARTTXReady(1)) {
		iec_last_active = ms_ticks;
		uint8_t index;
		uint8_t data;
		// fill transmit FIFO with 14 bytes
		for(index = 0; index < 14 && queue_dataAvailable(&rs485out_rbuffer); index++) {
			data = queue_read(&rs485out_rbuffer);
			UARTSendByte(1, data);
		}
		if (queue_isEmpty(&rs485out_rbuffer) && iec_flag_expect_response) {
			iec_flag_expect_response = 0;
			// if last byte is added to FIFO  reset timer for time out
			UART1LastReceived = ms_ticks;
			iec_flag_reading = 1;
			UART1Count = 0;

			return;
		}
	}

	if (iec_flag_reading) {
	    if (math_calc_diff(ms_ticks, UART1LastReceived) > IEC_WAIT_FOR_DATA_TIME_OUT) {
	    	iec_last_active = ms_ticks;
	    	logger_logString("receiver time out: ");
	    	logger_logNumberln(math_calc_diff(ms_ticks, UART1LastReceived));
			// 3000ms time out
			// send exit message
	    	iec_send_exit();
			iec_flag_reading = 0;
			iec_flag_data_available = 0;
			iec_flag_error = 0;

			// clear RX buffer
			UART1Count = 0;
		}
		else if (math_calc_diff(ms_ticks, UART1LastReceived) > IEC_BUFFER_FILL_IN_TIME_OUT && UART1Count > 0) {
			// 100ms time out
	    	iec_last_active = ms_ticks;

			iec_flag_reading = 0;

			// disable RBR
			LPC_UART1->IER = IER_THRE | IER_RLS;

			// log incoming data
			log_incomming_data();


			switch(iec_current_state) {
			case STATE_WAIT_IDENTIFICATION : iec_prepare_send_mode(); break;
			case STATE_WAIT_PASSWORD_PROMPT : iec_prepare_send_password(); break;
			case STATE_WAIT_PASSWORD_VERIFICATION : iec_prepare_password_verification(); break;
			case STATE_WAIT_DATA_READ : iec_parse_buffer(); break;
			default: break;
			}

			// clear RX buffer
			UART1Count = 0;

			// re-enable RBR
			LPC_UART1->IER = IER_THRE | IER_RLS | IER_RBR;
		}
	}

	// auto disconnect if idle
	if (iec_connect_status == CON_STAT_CONNECTED && math_calc_diff(ms_ticks, iec_last_active) > IEC_IDLE_TIME_OUT) {
		logger_logStringln("disconnecting due idle timeout ...");
		iec_disconnect();
	}
}
