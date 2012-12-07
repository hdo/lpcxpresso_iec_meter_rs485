#include "LPC17xx.h"
#include "s0_input.h"
#include "math_utils.h"

#define DEBOUNCE_CONSTANT 10

uint32_t s0_msticks[] = {0, 0, 0, 0};
uint32_t s0_diff[] = {0, 0, 0, 0};
uint32_t s0_oldState = 0;
uint32_t s0_newState = 0;
uint32_t s0_inputs[] = {S0_INPUT0, S0_INPUT1, S0_INPUT2, S0_INPUT3};
uint8_t s0_input_count = sizeof(s0_inputs) / sizeof(uint32_t);


void s0_init(void) {
	// no need to set PINSEL, since GPIO (00) is selected as default
	// no need to set PINMODE, since Pull-Up (00) is selected as default
	// no need to set FIODIR, since INPUT (0) is selected as default
}

uint32_t read_s0_status() {
	// we're are using GPIO0 here !!!
	// note that data is inverted (logic 0 -> 1) since we're are using pull-ups
	return ~LPC_GPIO0->FIOPIN & (S0_INPUT0 | S0_INPUT1 | S0_INPUT2 | S0_INPUT3 );
}


/**
 * process s0 inputs with DEBOUNCE
 */
void process_s0(uint32_t msticks) {
	uint8_t i;
	uint32_t d;
	for(i = 0; i < s0_input_count; i++) {
		if (s0_msticks[i] != 0) {
			d = math_calc_diff(msticks, s0_msticks[i]);
			if (d > DEBOUNCE_CONSTANT) {
				s0_diff[i] = d;
				s0_msticks[i] = 0;
			}
		}
	}
	s0_newState = read_s0_status();
	if (s0_newState != s0_oldState) {
		for(i = 0; i < s0_input_count; i++) {
			if (s0_newState & s0_inputs[i]) {
				// 0 to 1 transition
				if ((s0_oldState & s0_inputs[i]) == 0) {
					s0_msticks[i] = msticks;
					s0_diff[i] = 0;
				}
			} else {
				// 1 to 0 transition
				if (s0_oldState & s0_inputs[i]) {
					s0_msticks[i] = 0;
				}
			}
		}
		s0_oldState = s0_newState;
	}
}

uint32_t s0_triggered(uint8_t index) {
	if (s0_diff[index] > 1) {
		s0_diff[index] = 0;
		return 1;
	}
	return 0;
}
