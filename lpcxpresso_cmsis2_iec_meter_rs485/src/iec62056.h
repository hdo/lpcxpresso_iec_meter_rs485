#ifndef IEC62056_H_
#define IEC62056_H_

#define RS485_OUTPUT_BUFFER_SIZE 64

#define DATA_SOH 0x01
#define DATA_STX 0x02
#define DATA_ETX 0x03
#define DATA_EOT 0x04
#define DATA_ACK 0x06
#define DATA_NAK 0x15
#define DATA_NAK 0x15

#define METER_DEFAULT_PASSWORD "00000000"
#define METER_MAX_RECEIVE_DATA_LENGTH 32

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


#define IEC_WAIT_FOR_DATA_TIME_OUT 300     // 200 ms ticks == 2000ms
#define IEC_BUFFER_FILL_IN_TIME_OUT 2 //
#define IEC_IDLE_TIME_OUT 350         //

void iec_init();
void iec_process(uint32_t ms_ticks);
uint8_t iec_is_data_available();
uint8_t iec_is_ready();
uint8_t iec_is_error_occured();
uint8_t iec_get_connect_status();
char* iec_get_data_as_string();
uint32_t iec_get_data_as_int();
uint8_t iec_get_current_address();
void iec_clear_data();
void iec_connect(char* meter_id);
void iec_disconnect();
void iec_request_data_at_address(uint8_t address);



#endif /*IEC62056_H_*/
