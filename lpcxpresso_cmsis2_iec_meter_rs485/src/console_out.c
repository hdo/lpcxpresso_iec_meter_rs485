#include "LPC17xx.h"
#include "console_out.h"
#include "queue.h"

uint8_t output_buffer_data[CONSOLE_OUT_BUFFER_SIZE];
ringbuffer_t output_rbuffer = {.buffer=output_buffer_data, .head=0, .tail=0, .count=0, .size=CONSOLE_OUT_BUFFER_SIZE};

void   console_out_reset() {
	queue_reset(&output_rbuffer);
}

void   console_out_put(uint8_t data) {
	queue_put(&output_rbuffer, data);
}

uint8_t console_out_isEmpty() {
	return queue_isEmpty(&output_rbuffer);
}

uint8_t console_out_isFull() {
	return queue_isFull(&output_rbuffer);
}

uint8_t console_out_read() {
	return queue_read(&output_rbuffer);
}

uint8_t console_out_dataAvailable() {
	return queue_dataAvailable(&output_rbuffer);
}

uint8_t console_out_count() {
	return queue_count(&output_rbuffer);
}
