#include "LPC17xx.h"
#include "logger.h"
#include "console_out.h"
#include <stdlib.h>

uint8_t logger_enabled = 0;

/**
 * expected zero terminated string
 */
void logger_logString(char* data) {
	while(!console_out_isFull() && *data) {
		logger_logByte(*data++);
	}
}

void logger_logStringln(char* data) {
	logger_logString(data);
	logger_logCRLF();
}
void logger_logNumber(uint32_t value) {
	char buf[10];
	itoa(value, buf, 10);
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
	if (logger_enabled) {
		console_out_put(data);
	}
}

void logger_setEnabled(uint8_t enabled) {
	logger_enabled = enabled;
}
