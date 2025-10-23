/*
 * main.c
 */

#include <zephyr/kernel.h>
#include <BTN.h>
#include <LED.h>

#define SLEEP_MS 1

// Define some bitmasks to let LEDs represent binary positions
#define BIT_0 0b0001
#define BIT_1 0b0010  
#define BIT_2 0b0100
#define BIT_3 0b1000

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

      if (counter & BIT_0) {
        LED_set(LED0, LED_ON);
      } else {
        LED_set(LED0, LED_OFF);
      }

      if (counter & BIT_1) {
        LED_set(LED1, LED_ON);
      } else {
        LED_set(LED1, LED_OFF);
      }

      if (counter & BIT_2) {
        LED_set(LED2, LED_ON);
      } else {
        LED_set(LED2, LED_OFF);
      }

      if (counter & BIT_3) {
        LED_set(LED3, LED_ON);
      } else {
        LED_set(LED3, LED_OFF);
      }

      if (15 < counter) {
        counter = 0;
      }
    }

    k_msleep(SLEEP_MS);
  }
  
	return 0;
}
