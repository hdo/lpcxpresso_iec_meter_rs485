#ifndef __CONSOLE_OUT_H
#define __CONSOLE_OUT_H

#define CONSOLE_OUT_BUFFER_SIZE 255

void   console_out_reset();
void   console_out_put(uint8_t data);
uint8_t console_out_isEmpty();
uint8_t console_out_isFull();
uint8_t console_out_read();
uint8_t console_out_dataAvailable();
uint8_t console_out_count();


#endif /* end __CONSOLE_OUT_H */
/*****************************************************************************
**                            End Of File
******************************************************************************/
