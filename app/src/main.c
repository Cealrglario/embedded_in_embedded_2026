/*
 *  main.c
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#define LED0_NODE DT_ALIAS(led0)

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios); // get LED GPIO connection info

int main(void) {
    int ret;

    if(!gpio_is_ready_dt(&led0)) {                          // if the GPIO pin connected to LED0 isn't ready for configuration, return -1
        return -1;
    }
    
    ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE); // set the GPIO pin connected to LED0 as output

    if (ret < 0) {
        return ret;
    }

    while(1) {
        gpio_pin_toggle_dt(&led0);                          // Toggle the state of the GPIO pin connected to LED0

        k_msleep(500);                                      // Wait 500ms
    }

    return 0;
}