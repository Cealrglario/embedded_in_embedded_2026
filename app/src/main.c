/**
 * @file main.c
 */

#include <inttypes.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>

#include <lvgl.h>
#include "lv_data_obj.h"

#include "BTN.h"
#include "LED.h"

#define SLEEP_MS 100

static const struct device* display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
static lv_obj_t* screen = NULL;

extern const lv_image_dsc_t Joey;

// void lv_button_callback(lv_event_t* event) {
//   lv_obj_t* data_obj = (lv_obj_t*) lv_event_get_user_data(event);
//   led_id led = *(led_id*) lv_data_obj_get_data_ptr(data_obj);

//   LED_toggle(led);
// }

int main(void) {
  if(!device_is_ready(display_dev)) {
    return 0;
  }

  screen = lv_screen_active();

  if (screen == NULL) {
    return 0;
  }

  if (0 > BTN_init()) {
    return 0;
  }
  if (0 > LED_init()) {
    return 0;
  }

  // // Add buttons to the screen
  // for (uint8_t i = 0; i < NUM_LEDS; i++) {
  //   // Create buttons that are children of the "screen"
  //   lv_obj_t* ui_btn = lv_button_create(screen);

  //   // Place the buttons in a 2x2 grid in the center of the screen
  //   // matching the orientations of the LEDs on the board
  //   lv_obj_align(ui_btn, LV_ALIGN_CENTER, 50 * (i % 2 ? 1 : -1), 20 * (i < 2 ? -1 : 1));
  //   lv_obj_t* button_label = lv_label_create(ui_btn);
  //   char label_text[10];
  //   snprintf(label_text, 10, "LED %d", i);
  //   lv_label_set_text(button_label, label_text);
  //   lv_obj_align(button_label, LV_ALIGN_CENTER, 0, 0);

  //   led_id led = (led_id) i;  // For each button i, we want to associate an LED i

  //   // Associate the LED i with the button i as a child of the button
  //   lv_obj_t* data_obj = lv_data_obj_create_alloc_assign(ui_btn, &led, sizeof(led_id));

  //   // When a button is clicked, call the button callback previously defined and send the associated LED id to the callback
  //   lv_obj_add_event_cb(ui_btn, lv_button_callback, LV_EVENT_CLICKED, data_obj);
  // }

  // Display image to screen
  lv_obj_t* image = lv_image_create(screen);
  lv_image_set_src(image, &Joey);
  lv_obj_align(image, LV_ALIGN_CENTER, 0, 0);

  display_blanking_off(display_dev);

  while (1) {
    lv_timer_handler();
    k_msleep(SLEEP_MS);
  }
  return 0;
}
