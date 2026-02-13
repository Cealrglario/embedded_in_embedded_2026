#ifndef TOUCHSCREEN_DEFINES_H
#define TOUCHSCREEN_DEFINES_H

// We want to sleep for only 16ms at a time to emulate a ~60Hz response rate from the LCD/touchscreen
#define SLEEP_MS 16

// "Retrieve" the I2C peripheral from the device tree
#define ARDUINO_I2C_NODE DT_NODELABEL(arduino_i2c)   

// Touchscreen I2C registers
#define TD_ADDR 0x38  // "Touch device" I2C slave address
#define TD_STATUS 0x02
#define P1_XH 0x03
#define P1_XL 0x04
#define P1_YH 0x05
#define P1_YL 0x06

// Touchscreen relevant bit masks/shifts
#define TOUCH_EVENT_MASK 0xC0
#define TOUCH_POS_MSB_MASK 0x0F
#define TOUCH_EVENT_SHIFT 6

// UI button defines
#define HOME_SCREEN_BUTTONS 2
#define VERTICAL_SPACING_MULTIPLIER 25
#define BUTTON_TEXT_MAX_LENGTH 25

typedef enum {
  TOUCH_EVENT_PRESS_DOWN = 0b00u,
  TOUCH_EVENT_LIFT_UP = 0b01u,
  TOUCH_EVENT_CONTACT = 0b10u,
  TOUCH_EVENT_NO_EVENT = 0b11u
} touch_event_t;

#endif