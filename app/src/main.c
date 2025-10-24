/*
 * main.c
 */

#include <inttypes.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#include "BTN.h"
#include "LED.h"
#include "state_machine.h"

#define SLEEP_MS 1

int main(void) {

  if (0 > BTN_init()) {
    return 0;
  }
  if (0 > LED_init()) {
    return 0;
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
