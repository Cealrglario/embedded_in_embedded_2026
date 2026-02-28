/**
 * @file state_machine.h
 */

#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

/**
 * Includes
 */

#include <zephyr/smf.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/gpio.h>
#include <lvgl.h>

#include "touchscreen_defines.h"
#include "lv_data_obj.h"
#include "BTN.h"
#include "ble_peripheral.h"

/**
 * Function prototypes
 * 
 * (Declare any functions here that will be used in main.c)
 */

void state_machine_init();

int state_machine_run();

/**
 * Defines
 */

#define SW0_NODE DT_ALIAS(sw0) // device tree identifier for button 0 (physical button 1)
#define SCALAR_METRIC_MAX_LENGTH 24

#endif  