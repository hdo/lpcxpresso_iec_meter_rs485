#include "LPC17xx.h"
#include "queue.h"

void queue_reset(ringbuffer_t *rbuffer) {
	rbuffer->count = 0;
	rbuffer->head = 0;
	rbuffer->tail = 0;
}

void queue_put(ringbuffer_t *rbuffer, uint8_t data) {
	if (!queue_isFull(rbuffer)) {
		rbuffer->buffer[rbuffer->tail++] = data;
		rbuffer->count++;
		rbuffer->tail %= rbuffer->size;
	}
}

uint8_t queue_read(ringbuffer_t *rbuffer) {
	if (rbuffer->count > 0) {
		uint8_t data = rbuffer->buffer[rbuffer->head++];
		rbuffer->count--;
		rbuffer->head %= rbuffer->size;
		return data;
	}
	return 0;
}

uint8_t queue_isEmpty(ringbuffer_t *rbuffer) {
	return rbuffer->count == 0;
}

uint8_t queue_isFull(ringbuffer_t *rbuffer) {
	return rbuffer->count == rbuffer->size;
}

uint8_t queue_dataAvailable(ringbuffer_t *rbuffer) {
	return rbuffer->count > 0;
}

uint8_t queue_count(ringbuffer_t *rbuffer) {
	return rbuffer->count;
}
