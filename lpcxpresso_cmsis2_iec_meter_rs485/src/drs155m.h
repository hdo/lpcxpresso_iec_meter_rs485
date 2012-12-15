#ifndef DRS155M_H_
#define DRS155M_H_

typedef struct DRS155M {
	char* meter_id;
    uint32_t voltage;
    uint32_t ampere;
    uint32_t frequency;
    uint32_t active_power;
    uint32_t reactive_power;
    uint32_t total_energy;
} drs155m_t;


void drs155m_init();
void drs155m_process(uint32_t ms_ticks);
void drs155m_request_data(drs155m_t* meter);
uint8_t drs155m_is_ready();
uint8_t drs155m_is_data_available();
uint8_t drs155m_is_error();
void drs155m_reset();
uint32_t drs155m_get_duration();

#endif /*DRS155M_H_*/
