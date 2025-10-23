/*
 * main.c
 */

#include <zephyr/kernel.h>
#include <BTN.h>
#include <LED.h>

#define SLEEP_MS 1

// Define some bitmasks to let represent bit positions
#define BIT_0 0b0001
#define BIT_1 0b0010  
#define BIT_2 0b0100
#define BIT_3 0b1000

// Define length of password
#define PASSWORD_LENGTH 3

typedef enum state_t {
  locked,
  waiting,
} state;

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

  int password[PASSWORD_LENGTH] = {1, 0, 1};
  int user_entry[PASSWORD_LENGTH] = {0, 0, 0};

  state currentState = locked;

// uint8_t counter = 0b0;

  // while(1) {
  //   if (BTN_check_clear_pressed(BTN0)) {
  //     counter++;

  //     if (counter & BIT_0) {
  //       LED_set(LED0, LED_ON);
  //     } else {
  //       LED_set(LED0, LED_OFF);
  //     }

  //     if (counter & BIT_1) {
  //       LED_set(LED1, LED_ON);
  //     } else {
  //       LED_set(LED1, LED_OFF);
  //     }

  //     if (counter & BIT_2) {
  //       LED_set(LED2, LED_ON);
  //     } else {
  //       LED_set(LED2, LED_OFF);
  //     }

  //     if (counter & BIT_3) {
  //       LED_set(LED3, LED_ON);
  //     } else {
  //       LED_set(LED3, LED_OFF);
  //     }

  //     if (15 < counter) {
  //       counter = 0;
  //     }
  //   }

  //   k_msleep(SLEEP_MS);

  while(1) {
    switch(currentState) {
      case locked:
        printk("Entering state locked...\n");
        LED_set(LED0, LED_ON);

        for (int i = 1; i < NUM_LEDS; i++) {
          k_cpu_idle();

          if(BTN_check_clear_pressed(i)) {
            LED_set(i, LED_ON);
            user_entry[i - 1] = 1;
          }
        }

        for (int i = 0; i < PASSWORD_LENGTH; i++) {
          if (password[i] != user_entry[i]) {
            printk("Incorrect!\n");
            currentState = waiting;
            break;
          }
        }

        printk("Correct!\n");
        break;

      case waiting:
        printk("Entering state waiting...\n");
        LED_set(LED0, LED_OFF);

        k_cpu_idle();

        for (int i = 0; i < NUM_LEDS; i++) {
          if(BTN_check_clear_pressed(i)) {
            currentState = locked;
          }
        }
        break;

      default:
        printk("CPU defaulting to idle...\n");
        k_cpu_idle();
    }
  }
  
  return 0;
}

