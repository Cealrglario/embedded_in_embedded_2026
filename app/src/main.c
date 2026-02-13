/**
 * @file main.c
 */

#include <inttypes.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#include "defines.h"
#include "I2C_defines.h"
#include "SPI_defines.h"
#include "BTN.h"
#include "LED.h"
#include "lv_data_obj.h"

/*

LCD-specific SPI setup

*/

// Retrieve the dc/x pin from the device tree
static const struct gpio_dt_spec dcx_gpio = GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE, dcx_gpios);

// Retrieve the chip select pin from the device tree
static const struct spi_cs_control cs_ctrl = (struct spi_cs_control) {
  .gpio = GPIO_DT_SPEC_GET(ARDUINO_SPI_NODE, cs_gpios),
  .delay = 1u,
};

static const struct device* spi_dev = DEVICE_DT_GET(ARDUINO_SPI_NODE); // Selects the SPI peripheral to talk to

// Configure our SPI communication
static const struct spi_config spi_cfg = {
  .frequency = 1000000, // Using a 1MHz clock

  // We are using SPI mode 0, which doesn't need to be explicitly set due to a 0 value
  
  // Configure as SPI master with 8 byte words and MSB transferred out first
  .operation = SPI_OP_MODE_MASTER | SPI_WORD_SET(8) | SPI_TRANSFER_MSB,
  .slave = 0,
  .cs = cs_ctrl,
};

// To send commands/data to the LCD
static void lcd_cmd(uint8_t cmd, struct spi_buf* data) {
  struct spi_buf cmd_buf[1] = {[0]={.buf=&cmd, .len=1}};
  struct spi_buf_set cmd_set = {.buffers=cmd_buf, .count=1};

  // D/C select must be low to send command
  gpio_pin_set_dt(&dcx_gpio, 0);

  spi_write(spi_dev, &spi_cfg, &cmd_set);

  if (data != NULL) {
    struct spi_buf_set data_set = {.buffers=data, .count=1};

    // D/C select must be high to send data
    gpio_pin_set_dt(&dcx_gpio, 1);

    spi_write(spi_dev, &spi_cfg, &data_set);
  }
}

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
  // If the SPI peripheral is not yet ready
  if(!device_is_ready(spi_dev)) {
    printk("SPI not yet ready.\n");
    return 0;
  }

  // If the I2C peripehral is not yet ready
  if(!device_is_ready(i2c_dev)) {
    printk("I2C device not yet ready.\n");
    return 0;
  }

  // Configure I2C device with 100KHz clock on Master mode
  if(0 > i2c_configure(i2c_dev, I2C_SPEED_SET(I2C_SPEED_STANDARD) | I2C_MODE_CONTROLLER)) {
    printk("Error while configuring I2C device.\n");
  }

  // If the dc/x GPIO is not yet ready
  if(!gpio_is_ready_dt(&dcx_gpio)) {
    printk("GPIO not yet ready.\n");
    return 0;
  }

  if(gpio_pin_configure_dt(&dcx_gpio, GPIO_OUTPUT_LOW)) {
    printk("Pin configuration not successful.\n");
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

  // Reset the LCD state
  lcd_cmd(CMD_SOFTWARE_RESET, NULL);
  k_msleep(120);  // Software reset command can take up to 120 ms before any more commands can be sent

  lcd_cmd(CMD_SLEEP_OUT, NULL);
  lcd_cmd(CMD_DISPLAY_ON, NULL);

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

    // Check touches every 100ms
    k_msleep(SLEEP_MS);
  }
  return 0;
}
