/**
 * @file main.c
 */

#include <inttypes.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/display.h>
#include <zephyr/sys/printk.h> 
#include <lvgl.h>

#include "touchscreen_defines.h"
#include "state_machine.h"
#include "BTN.h"
#include "LED.h"
#include "lv_data_obj.h"

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

/*

LVGL setup

*/

// Retrieve the drivers for the LCD (ILI9341) from the Zephyr device tree
static const struct device* display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));

// An LVGL object representing the LCD we will be displaying content onto
lv_obj_t* screen = NULL;

// To store the x and y position of a touch between checks
// We want these to be static so that the last touch position is stored instead of being set to (0, 0) every call
static uint16_t x_pos;
static uint16_t y_pos;

// LVGL callback that is called to check whether a button has been pressed
void touch_read_cb(lv_indev_t* indev, lv_indev_data_t* data) {
  /*
  LOGICAL STEPS:
  1. Read I2C touchscreen data
  2. Check if a valid touch is detected...
  3. Else if no touch is detected...
  */

  uint8_t touch_status;
  touch_control_cmd_rsp(TD_STATUS, &touch_status); // Get touchscreen status (is it ready?)

  // If screen is currently being held down
  if (touch_status == 1) {
    uint8_t x_pos_h;
    uint8_t x_pos_l;
    uint8_t y_pos_h;
    uint8_t y_pos_l;
    
    touch_control_cmd_rsp(P1_XH, &x_pos_h);
    touch_control_cmd_rsp(P1_XL, &x_pos_l);
    touch_control_cmd_rsp(P1_YH, &y_pos_h);
    touch_control_cmd_rsp(P1_YL, &y_pos_l);

    // Update the "last known" touch position, remember that coordinates are INVERTED on the LCD
    y_pos = ((x_pos_h & TOUCH_POS_MSB_MASK) << 8) + x_pos_l;
    x_pos = ((y_pos_h & TOUCH_POS_MSB_MASK) << 8) + y_pos_l;

    // Now that we know the x and y positions of a touch, send it to LVGL and report a touch
    data->point.x = x_pos;
    data->point.y = y_pos;
    data->state = LV_INDEV_STATE_PRESSED;
  }
  // If the screen isn't currently being touched (released)
  else if (touch_status == 0) {
    data->state = LV_INDEV_STATE_RELEASED;

    // We want to continue to record the last location where a press occured (static variables)
    // because LVGL inteprets a click as a sequence of press, hold, release all at the same x and y positions
    // If we didn't have static variables, and they were set to garbage or (0, 0), LVGL wouldn't properly
    // interpret that a click happened from a specific sequence of events
    data->point.x = x_pos;
    data->point.y = y_pos;

    printk("Press at %u, %u\n", x_pos, y_pos);
  }
}

int main(void) {
  /**
   * Initialization checks
   */

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

  // "Turn on" the screen so we can actually see things on it
  display_blanking_off(display_dev);

  if (0 > BTN_init()) {
    printk("Buttons not yet ready.\n");
    return 0;
  }

  if (0 > LED_init()) {
    printk("LEDs not yet ready.\n");
    return 0;
  }

  // Initialize the state machine
  state_machine_init();

  // Set up LVGL so that the button press callback is called every SLEEP_MS period
  // NOTE: "lv_indev" means "LVGL input device", and our input, a touchscreen is of type "pointer" (like a cursor)
  lv_indev_t* indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, touch_read_cb);

  /**
   * Run the state machine
   */

  while (1) {
    if (0 > state_machine_run()) {
      printk("Error occured while running state machine.\n");
      return 0;
    }

    k_msleep(SLEEP_MS);
  }
  return 0;
}
