/*
 *  main.c
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)
#define LED3_NODE DT_ALIAS(led3)

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios); // get LED0 GPIO connection info
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios); // get LED1 GPIO connection info
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios); // get LED2 GPIO connection info
static const struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(LED3_NODE, gpios); // get LED3 GPIO connection info

int main(void) {
    int ret0;
    int ret1;
    int ret2;
    int ret3;

    if(!gpio_is_ready_dt(&led0) || !gpio_is_ready_dt(&led1) || !gpio_is_ready_dt(&led2) || !gpio_is_ready_dt(&led3)) {
        // if the GPIO pin connected to LED isn't ready for configuration, return -1
        return -1;
    }
    
    ret0 = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE); // set the GPIO pin connected to LED0 as output
    ret1 = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE); // set the GPIO pin connected to LED1 as output
    ret2 = gpio_pin_configure_dt(&led2, GPIO_OUTPUT_ACTIVE); // set the GPIO pin connected to LED2 as output
    ret3 = gpio_pin_configure_dt(&led3, GPIO_OUTPUT_ACTIVE); // set the GPIO pin connected to LED3 as output

    if (ret0 < 0) {
        return ret0;
    }

    if (ret1 < 0) {
        return ret1;
    }

    if (ret2 < 0) {
        return ret2;
    }
    
    if (ret3 < 0) {
        return ret3;
    }

    while(1) {
        gpio_pin_toggle_dt(&led0);                          // Toggle the state of the GPIO pin connected to LED0

        k_msleep(250);                                      // Wait 250ms

        gpio_pin_toggle_dt(&led3);                          // Toggle the state of the GPIO pin connected to LED3

        k_msleep(250);                                      // Wait 250ms

        gpio_pin_toggle_dt(&led1);                          // Toggle the state of the GPIO pin connected to LED1

        k_msleep(250);                                      // Wait 250ms

        gpio_pin_toggle_dt(&led2);                          // Toggle the state of the GPIO pin connected to LED2
        
        k_msleep(250);                                      // Wait 250ms
    }

    return 0;
}