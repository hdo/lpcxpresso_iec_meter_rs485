#include "LPC17xx.h"
#include "logger.h"
#include <stdlib.h>

uint8_t rbuffer[LOGGER_BUFFER_SIZE];
uint8_t rbuffer_head=0;
uint8_t rbuffer_tail=0;
uint8_t rbuffer_count=0;


/**
 * expected zero terminated string
 */
void logger_logString(char* data) {
	while(!logger_isFull() && *data) {
		logger_logByte(*data++);
	}
}

void logger_logStringln(char* data) {
	logger_logString(data);
	logger_logCRLF();
}
void logger_logNumber(uint32_t value) {
	char buf[10];
	uitoa(value, buf, 10);
	logger_logString((char*) buf);
}

void logger_logNumberln(uint32_t value) {
	logger_logNumber(value);
	logger_logCRLF();
}

void logger_logCRLF() {
	logger_logByte(13);
	logger_logByte(10);
}

void logger_logByte(uint8_t data) {
	if (!logger_isFull()) {
		rbuffer[rbuffer_tail++] = data;
		rbuffer_count++;
		if (rbuffer_tail >= LOGGER_BUFFER_SIZE) {
			rbuffer_tail %= LOGGER_BUFFER_SIZE;
		}
	}
}

uint8_t logger_read() {
	if (rbuffer_count > 0) {
		uint8_t data = rbuffer[rbuffer_head++];
		rbuffer_count--;
		if (rbuffer_head >= LOGGER_BUFFER_SIZE) {
			rbuffer_head %= LOGGER_BUFFER_SIZE;
		}
		return data;
	}
	return 0;
}

uint8_t logger_isEmpty() {
	return rbuffer_count == 0;
}

uint8_t logger_isFull() {
	return rbuffer_count == LOGGER_BUFFER_SIZE;
}

uint8_t logger_dataAvailable() {
	return rbuffer_count > 0;
}

uint8_t logger_count() {
	return rbuffer_count;
}
