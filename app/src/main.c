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
  if (0 > BTN_init()) {
    return 0;
  }
  if (0 > LED_init()) {
    return 0;
  }

  while (1) {
    k_msleep(SLEEP_MS);
  }
  return 0;
}
