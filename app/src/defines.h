#ifndef DEFINES_H
#define DEFINES_H

#define SLEEP_MS 100

#define ARDUINO_SPI_NODE DT_NODELABEL(arduino_spi)
#define ARDUINO_I2C_NODE DT_NODELABEL(arduino_i2c)   
#define ZEPHYR_USER_NODE DT_PATH(zephyr_user)

// LCD commands
#define CMD_SOFTWARE_RESET 0x01
#define CMD_SLEEP_OUT 0x11
#define CMD_DISPLAY_ON 0x29
#define CMD_COLUMN_ADDRESS_SET 0x2A
#define CMD_ROW_ADDRESS_SET 0x2B
#define CMD_MEMORY_WRITE 0x2C

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

typedef enum {
  TOUCH_EVENT_PRESS_DOWN = 0b00u,
  TOUCH_EVENT_LIFT_UP = 0b01u,
  TOUCH_EVENT_CONTACT = 0b10u,
  TOUCH_EVENT_NO_EVENT = 0b11u
} touch_event_t;

#endif