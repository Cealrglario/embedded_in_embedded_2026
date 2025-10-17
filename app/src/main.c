/*
 * main.c
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <inttypes.h>

#define SLEEP_TIME_MS   1000
#define SW0_NODE        DT_ALIAS(sw0)

static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(SW0_NODE, gpios);

static struct gpio_callback button_isr_data;

// *dev is a pointer to the GPIO port device that triggered the interrupt
// pins is a bitmask of the pin that triggered the interrupt on the GPIO port
void button_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
  printk("Button 0 pressed 😊\n");
}

int main(void) {
  // used to capture a return value
  int ret;  
  if (!gpio_is_ready_dt(&button)) {
    return 0;
  }

   // configure the button port as input
  ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
  if (0 > ret) {  // Yoda notation
    return 0;
  }

  // configure button interrupts on a RISING edge (button press)
  ret = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE); 
  if (0 > ret) {
    return 0;
  }

  gpio_init_callback(&button_isr_data, button_isr, BIT(button.pin));
  gpio_add_callback(button.port, &button_isr_data);

  while(1) {

  }
  
	return 0;
}
