#ifndef __LOGGER_H
#define __LOGGER_H

void logger_logString(char* data);
void logger_logStringln(char* data);
void logger_logNumber(uint32_t value);
void logger_logNumberln(uint32_t value);
void logger_logCRLF();
void logger_logByte(uint8_t data);
void logger_setEnabled(uint8_t enabled);

#endif /* end __LOGGER_H */
/*****************************************************************************
**                            End Of File
******************************************************************************/
