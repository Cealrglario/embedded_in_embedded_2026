/*
 * main.c
 */

#include <zephyr/kernel.h>
#include <BTN.h>
#include <LED.h>

#define SLEEP_MS 1

// Define some bitmasks to let LEDs represent binary positions
#define LED_0_COUNT 0b0001
#define LED_1_COUNT 0b0010  
#define LED_2_COUNT 0b0100
#define LED_3_COUNT 0b1000

int main(void) {
  // Initialize button API
  if (0 > BTN_init()) {
    return 0;
  }

  // Initialize led API
  if (0 > LED_init()) {
    return 0;
  }

uint8_t counter = 0b0;

  while(1) {
    if (BTN_check_clear_pressed(BTN0)) {
      counter++;

      if (counter & LED_0_COUNT) {
        LED_set(LED0, LED_ON);
      } else {
        LED_set(LED0, LED_OFF);
      }

      if (counter & LED_1_COUNT) {
        LED_set(LED1, LED_ON);
      } else {
        LED_set(LED1, LED_OFF);
      }

      if (counter & LED_2_COUNT) {
        LED_set(LED2, LED_ON);
      } else {
        LED_set(LED2, LED_OFF);
      }

      if (counter & LED_3_COUNT) {
        LED_set(LED3, LED_ON);
      } else {
        LED_set(LED3, LED_OFF);
      }

      if (15 == counter) {
        counter = 0;
      }
    }

    k_msleep(SLEEP_MS);
  }
  
	return 0;
}
