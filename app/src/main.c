/**
 * @file main.c
 */

#include <inttypes.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#include "BTN.h"
#include "LED.h"

#define SLEEP_MS 1

// LCD commands
#define CMD_SOFTWARE_RESET 0x01
#define CMD_SLEEP_OUT 0x11
#define CMD_DISPLAY_ON 0x29
#define CMD_COLUMN_ADDRESS_SET 0x2A
#define CMD_ROW_ADDRESS_SET 0x2B
#define CMD_MEMORY_WRITE 0x2C

#define ARDUINO_SPI_NODE DT_NODELABEL(arduino_spi)
#define ZEPHYR_USER_NODE DT_PATH(zephyr_user)

static const struct gpio_dt_spec dcx_gpio = GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE, dcx_gpios);
static const struct spi_cs_control cs_ctrl = (struct spi_cs_control) {
  .gpio = GPIO_DT_SPEC_GET(ARDUINO_SPI_NODE, cs_gpios),
  .delay = 1u,
};

static const struct device * dev = DEVICE_DT_GET(ARDUINO_SPI_NODE);
static const struct spi_config spi_cfg = {
  .frequency = 1000000, // Using a 1MHz clock

  // We are using SPI mode 0, which doesn't need to be explicitly set due to a 0 value
  
  // Configure as SPI master with 8 byte words and MSB transferred out first
  .operation = SPI_OP_MODE_MASTER | SPI_WORD_SET(8) | SPI_TRANSFER_MSB,
  .slave = 0,
  .cs = &cs_ctrl,
};

static void lcd_cmd(uint8_t cmd, struct spi_buf * data) {
  struct spi_buf cmd_buf[1] = {[0]={.buf=&cmd, .len=1}};
  struct spi_buf_set cmd_set = {.buffers=cmd_buf, .count=1};

  // D/C select must be low to send command
  gpio_pin_set_dt(&dcx_gpio, 0);

  spi_write(dev, &spi_cfg, &cmd_set);

  if (data != NULL) {
    struct spi_buf_set data_set = {.buffers=data, .count=1};

    // D/C select must be high to send data
    gpio_pin_set_dt(&dcx_gpio, 1);

    spi_write(dev, &spi_cfg, &data_set);
  }
}

int main(void) {
  // If the SPI peripheral is not yet ready
  if(!device_is_ready(dev)) {
    printk("SPI not yet ready.\n");
    return 0;
  }

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

  lcd_cmd(CMD_SOFTWARE_RESET, NULL);
  k_msleep(120);  // Software reset command can take up to 120 ms before any more commands can be sent

  lcd_cmd(CMD_SLEEP_OUT, NULL);
  k_msleep(120);

  lcd_cmd(CMD_DISPLAY_ON, NULL);

  uint8_t column_data[] = {[0]=0x00, [1]=0x95, [2]=0x00, [3]=0x9f}; // Column 149 to 159
  uint8_t row_data[] = {[0]=0x00, [1]=0x75, [2]=0x00, [3]=0x7f}; // Row 117 to 127
  uint8_t color_data[300];

  // For each pixel in our designated region above
  for (int i = 0; i < 300; i+=3) {
    color_data[i] = 0xFC; // R
    color_data[i+1] = 0;  // G
    color_data[i+2] = 0;  // B
  }

  struct spi_buf column_data_buf = {.buf=column_data, .len=4};
  struct spi_buf row_data_buf = {.buf=row_data, .len=4};
  struct spi_buf color_data_buf = {.buf=color_data, .len=300};

  lcd_cmd(CMD_COLUMN_ADDRESS_SET, &column_data_buf);
  lcd_cmd(CMD_ROW_ADDRESS_SET, &row_data_buf);
  lcd_cmd(CMD_MEMORY_WRITE, &color_data_buf);

  while(1) {
    k_msleep(SLEEP_MS);
  }

  return 0;
}
