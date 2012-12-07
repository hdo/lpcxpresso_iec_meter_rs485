#ifndef __LOGGER_H
#define __LOGGER_H

#define LOGGER_BUFFER_SIZE 256


void logger_logString(char* data);
void logger_logStringln(char* data);
void logger_logNumber(uint32_t value);
void logger_logNumberln(uint32_t value);
void logger_logCRLF();
void logger_logByte(uint8_t data);
uint8_t logger_isEmpty();
uint8_t logger_isFull();
uint8_t logger_read();
uint8_t logger_dataAvailable();
uint8_t logger_count();

#endif /* end __LOGGER_H */
/*****************************************************************************
**                            End Of File
******************************************************************************/
