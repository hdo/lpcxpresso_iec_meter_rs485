#ifndef LEDS_H_
#define LEDS_H_


//#define USE_ONBOARD_LED 1

// define max number of external leds (excluding onboard led)
#define MAX_LEDS 8

#define LED_ONBOARD 22
#define LED0 0
#define LED1 1
#define LED2 2
#define LED3 3
#define LED4 4
#define LED5 5
#define LED6 6
#define LED7 7


void led_init (void);
void led2_on (void);
void led2_off (void);
void led2_invert (void);
void led_on (uint8_t channel);
void led_off (uint8_t channel);
void led_invert (uint8_t channel);
void led_all_on ();
void led_all_off ();
void led_all_invert ();
void led_signal (uint8_t channel, uint32_t timeout, uint32_t msticks);
void led_process(uint32_t msticks);



#endif /*LEDS_H_*/
