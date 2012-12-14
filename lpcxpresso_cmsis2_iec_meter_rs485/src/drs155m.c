#include "LPC17xx.h"
#include "drs155m.h"
#include "iec62056.h"
#include "math_utils.h"
#include "logger.h"

// array of addresses to be read from meter
const uint8_t address_list[] = {0, 1, 2, 3, 4, 10};

#define ADDRESS_LIST_COUNT sizeof(address_list)/sizeof(uint8_t)


uint8_t drs_flag_active = 0;
uint8_t drs_flag_ready = 1;
uint8_t drs_flag_data_available = 0;
uint8_t drs_flag_request_started = 0;
uint8_t drs_flag_error = 0;
uint8_t current_address_index = 0;

drs155m_t* current_meter;

void drs155m_init() {
	iec_init();
}

void drs155m_process(uint32_t ms_ticks) {
	iec_process(ms_ticks);

	if (drs_flag_active) {

		if (iec_get_connect_status() == CON_STAT_DISCONNECTED) {
			if (!drs_flag_request_started) {
				drs_flag_request_started = 1;
				iec_connect(current_meter->meter_id);
			}
			else {
				// error occured due time out?
				drs_flag_error = 1;
			}
		}

		if (iec_get_connect_status() == CON_STAT_CONNECTED) {
			if (iec_is_ready()) {
				if (current_address_index < ADDRESS_LIST_COUNT) {
					iec_request_data_at_address(address_list[current_address_index]);
				}
				else {
					iec_disconnect();
					current_address_index = 0;
					drs_flag_active = 0;
					drs_flag_data_available = 1;
				}
			}

			if (iec_is_data_available()) {
				uint8_t address = iec_get_current_address();
				uint32_t value = iec_get_data_as_int();
				logger_logString("data at address: ");
				logger_logNumberln(address);
				logger_logString("data as string: ");
				logger_logStringln(iec_get_data_as_string());
				logger_logString("data as int: ");
				logger_logNumberln(value);

				switch(address) {
				case 0: current_meter->voltage = value; break;
				case 1: current_meter->ampere = value; break;
				case 2: current_meter->frequency = value; break;
				case 3: current_meter->active_power = value; break;
				case 4: current_meter->reactive_power = value; break;
				case 10: current_meter->total_energy = value; break;
				}

				iec_clear_data();
				current_address_index++;
			}
		}
	}

}

void drs155m_request_data(drs155m_t* meter) {
	if (iec_get_connect_status() == CON_STAT_DISCONNECTED && drs_flag_ready) {
		current_meter = meter;
		meter->voltage = 0;
		meter->ampere = 0;
		meter->frequency = 0;
		meter->active_power = 0;
		meter->reactive_power = 0;
		meter->total_energy = 0;

		drs_flag_active = 1;
		drs_flag_ready = 0;
		drs_flag_data_available = 0;
	}
}

uint8_t drs155m_is_ready() {
	return drs_flag_ready;
}

uint8_t drs155m_is_data_available() {
	return drs_flag_data_available;
}

uint8_t drs155m_is_error() {
	return drs_flag_error;
}

void drs155m_reset() {
	drs_flag_ready = 1;
	drs_flag_active = 0;
	drs_flag_data_available = 0;
	drs_flag_request_started = 0;
}

