#ifndef DRS155M_H_
#define DRS155M_H_

#define RS485_OUTPUT_BUFFER_SIZE 64

#define DATA_SOH 0x01
#define DATA_STX 0x02
#define DATA_ETX 0x03
#define DATA_EOT 0x04
#define DATA_ACK 0x06
#define DATA_NAK 0x15
#define DATA_NAK 0x15

#define DRS155M_DEFAULT_PASSWORD "00000000"
#define DRS155M_MAX_RECEIVE_DATA_LENGTH 32

#define STATE_DISCONNECTED 10
// /YTL:001511420144
#define STATE_WAIT_IDENTIFICATION 11
// {SOH}P0{STX}(00000000){ETX}{BCC 0x60}
#define STATE_WAIT_PASSWORD_PROMPT 12
// {ACK}
#define STATE_WAIT_PASSWORD_VERIFICATION 13
// {STX}00000000(2257){ETX}{BCC 0x00}
#define STATE_WAIT_DATA_READ 14


#define CON_STAT_CONNECTING 1
#define CON_STAT_DISCONNECTING 2
#define CON_STAT_CONNECTED 3
#define CON_STAT_DISCONNECTED 4



void iec_init();
void process_iec(uint32_t ms_ticks);
uint8_t iec_is_data_available();
uint8_t iec_is_ready();
uint8_t iec_is_error_occured();
uint8_t iec_get_connect_status();
char* iec_read_data_as_string();
uint32_t iec_read_data_as_int();



#endif /*DRS155M_H_*/
