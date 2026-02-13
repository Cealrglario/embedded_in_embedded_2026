/**
 * @file main.c
 */

#include <inttypes.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/sys/printk.h>

#include "touchscreen_defines.h"
#include "BTN.h"
#include "LED.h"
#include "lv_data_obj.h"

/*

LVGL setup

*/

// Retrieve the drivers for the LCD (ILI9341) from the Zephyr device tree
static const struct device* display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));

// An LVGL object representing the LCD we will be displaying content onto
static lv_obj_t* screen = NULL;

/*

Touchscreen-specific I2C setup

*/

static const struct device* i2c_dev = DEVICE_DT_GET(ARDUINO_I2C_NODE); // Select an I2C "device" from the on-board peripheral

// To send commands to and read responses from the I2C slave
void touch_control_cmd_rsp(uint8_t cmd, uint8_t* rsp) {
  struct i2c_msg cmd_rsp_msg[2] = {
    [0] = {.buf=&cmd, .len=1, .flags=I2C_MSG_WRITE},
    [1] = {.buf=rsp, .len=1, .flags=I2C_MSG_RESTART | I2C_MSG_READ | I2C_MSG_STOP}
  };

  // Send a command to and receive a response from the I2C slave 
  i2c_transfer(i2c_dev, cmd_rsp_msg, 2, TD_ADDR);
}

int main(void) {
  // If the I2C peripheral is not yet ready
  if(!device_is_ready(i2c_dev)) {
    printk("I2C device not yet ready.\n");
    return 0;
  }

  // Configure I2C device with 100KHz clock on Master mode
  if(0 > i2c_configure(i2c_dev, I2C_SPEED_SET(I2C_SPEED_STANDARD) | I2C_MODE_CONTROLLER)) {
    printk("Error while configuring I2C device.\n");
  }

  if(!device_is_ready(display_dev)) {
    printk("LCD drivers not yet ready.\n");
    return 0;
  }

  // If we get to this point, the LCD drivers are ready and we can initialize the LVGL screen object
  screen = lv_screen_active();
  
  if (screen == NULL) {
    printk("Failed to initialize LVGL screen.\n");
    return 0;
  }

  if (0 > BTN_init()) {
    printk("Buttons not yet ready.\n");
    return 0;
  }

  if (0 > LED_init()) {
    printk("LEDs not yet ready.\n");
    return 0;
  }

  // We would initialize objects to display onto the LVGL screen here

  // "Turn on" the screen so we can actually see things on it
  display_blanking_off(display_dev);

  while (1) {
    uint8_t touch_status;
    touch_control_cmd_rsp(TD_STATUS, &touch_status); // Get touchscreen status (is it ready?)

    if (touch_status == 1) {
      uint8_t x_pos_h;
      uint8_t x_pos_l;
      uint8_t y_pos_h;
      uint8_t y_pos_l;
      
      touch_control_cmd_rsp(P1_XH, &x_pos_h);
      touch_control_cmd_rsp(P1_XL, &x_pos_l);
      touch_control_cmd_rsp(P1_YH, &y_pos_h);
      touch_control_cmd_rsp(P1_YL, &y_pos_l);

      // Build the full (presumably)  9-bit x and y position 
      uint16_t x_pos = ((x_pos_h & TOUCH_POS_MSB_MASK) << 8) + x_pos_l;
      uint16_t y_pos = ((y_pos_h & TOUCH_POS_MSB_MASK) << 8) + y_pos_l;

      printk("Touch at %u, %u\n", x_pos, y_pos);
    }

    // Check touches and refresh LCD every SLEEP_MS milliseconds
    k_msleep(SLEEP_MS);
  }
  return 0;
}
