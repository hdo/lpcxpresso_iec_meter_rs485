#ifndef __QUEUE_H
#define __QUEUE_H

typedef struct ring_buffer {
	uint8_t *buffer;
    uint8_t head;
    uint8_t tail;
    uint8_t count;
    uint8_t size;
} ringbuffer_t;


void   queue_reset(ringbuffer_t *rbuffer);
void   queue_put(ringbuffer_t *rbuffer, uint8_t data);
uint8_t queue_isEmpty(ringbuffer_t *rbuffer);
uint8_t queue_isFull(ringbuffer_t *rbuffer);
uint8_t queue_read(ringbuffer_t *rbuffer);
uint8_t queue_dataAvailable(ringbuffer_t *rbuffer);
uint8_t queue_count(ringbuffer_t *rbuffer);
uint8_t queue_peek(ringbuffer_t *rbuffer, uint8_t index);


#endif /* end __QUEUE_H */
/*****************************************************************************
**                            End Of File
******************************************************************************/
