/*
 * main.c
 */

#include <zephyr/kernel.h>

#include "BTN.h"
#include "LED.h"
#include "state_machine.h"

#define SLEEP_MS 1

int main(void) {
  // Initialize button API
  if (0 > BTN_init()) {
    return 0;
  }

  // Initialize led API
  if (0 > LED_init()) {
    return 0;
  }

  // Initialize all LEDs to off
  for (int i = 0; i < NUM_LEDS; i++) {
    LED_set(i, LED_OFF);
  }

  init_state_machine();

  while(1) {
    int ret = run_state_machine();
    if (0 > ret) {
      return 0;
    }

    k_msleep(SLEEP_MS);
  }
  
  return 0;
}

