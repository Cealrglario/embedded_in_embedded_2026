/**
 * @file state_machine.c
 */

#include <zephyr/smf.h>
#include <zephyr/drivers/display.h>
#include <lvgl.h>

#include "state_machine.h"
#include "touchscreen_defines.h"
#include "lv_data_obj.h"

/**
 * Local variables
 */

static ui_state_object_t ui_state_object;

// An LVGL object representing the LCD we will be displaying content onto
extern lv_obj_t* screen;

// Struct that holds the actual states that Zephyr will traverse throughout runtime
static const struct smf_state ui_states[] = {
    [MAIN_MENU] = SMF_CREATE_STATE(main_menu_on_state_entry, main_menu_on_state_run, NULL, NULL, NULL),
    [PERFORMANCE_METRICS] = SMF_CREATE_STATE(performance_metrics_on_state_entry, performance_metrics_on_state_run, NULL, NULL, NULL),
    [MEDIA_CONTROLS] = SMF_CREATE_STATE(media_controls_on_state_entry, media_controls_on_state_run, NULL, NULL, NULL)
};

// Indicates the next state to transition to
static enum ui_state_machine_states next_state = -1; // -1 indicates no state change request

void state_machine_init() {
    // Set initial state to be the main menu
    smf_set_initial(SMF_CTX(&ui_state_object), &ui_states[MAIN_MENU]);
}

int state_machine_run() {
    // When we run the state machine, we just want to return the state currently held in the ui_state_object
    return smf_run_state(SMF_CTX(&ui_state_object));
}

/**
 * Main menu states
 */

static void main_menu_on_state_entry(void* o) {
    // Clear any existing screen contents to display the new menu
    lv_obj_clean(screen);

    // We would initialize objects to display onto the LVGL screen here
    for (uint8_t i = 0; i < HOME_SCREEN_BUTTONS; i++) {
        lv_obj_t* ui_btn = lv_button_create(screen); // Add a button to the screen

        // Place the buttons in the center of the screen
        lv_obj_align(ui_btn, LV_ALIGN_CENTER, 0, VERTICAL_SPACING_MULTIPLIER * (i % 2 ? 1 : -1));

        // Add text to the button
        lv_obj_t* button_label = lv_label_create(ui_btn); 
        char label_text[BUTTON_TEXT_MAX_LENGTH];
        snprintf(label_text, BUTTON_TEXT_MAX_LENGTH, i % 2 ? "Performance Metrics" : "Media Controls");
        lv_label_set_text(button_label, label_text);
        lv_obj_align(button_label, LV_ALIGN_CENTER, 0, 0);
    }
}

static enum smf_state_result main_menu_on_state_run(void* o) {
    lv_timer_handler();
    
    if (next_state == PERFORMANCE_METRICS) {
        next_state = -1; // Clear the next state flag since we're now handling the transition
        smf_set_state(SMF_CTX(&ui_state_object), &ui_states[PERFORMANCE_METRICS]);
    }
    else if (next_state == MEDIA_CONTROLS) {
        next_state = -1; // Clear the next state flag since we're now handling the transition
        smf_set_state(SMF_CTX(&ui_state_object), &ui_states[MEDIA_CONTROLS]);
    }

    return SMF_EVENT_HANDLED;
}

/**
 * Performance metrics states
 */
static void performance_metrics_on_state_entry(void* o) {
    // Clear any existing screen contents to display the new menu
    lv_obj_clean(screen);
}

static enum smf_state_result performance_metrics_on_state_run(void* o) {
    lv_timer_handler();
    
    if (next_state == MAIN_MENU) {
        next_state = -1; // Clear the next state flag since we're now handling the transition
        smf_set_state(SMF_CTX(&ui_state_object), &ui_states[MAIN_MENU]);
    }
    else if (next_state == MEDIA_CONTROLS) {
        next_state = -1; // Clear the next state flag since we're now handling the transition
        smf_set_state(SMF_CTX(&ui_state_object), &ui_states[MEDIA_CONTROLS]);
    }

    return SMF_EVENT_HANDLED;
}

/**
 * Media controls states
 */
static void media_controls_on_state_entry(void* o) {
    // Clear any existing screen contents to display the new menu
    lv_obj_clean(screen);
}

static enum smf_state_result media_controls_on_state_run(void* o) {
    lv_timer_handler();
    
    if (next_state == MAIN_MENU) {
        next_state = -1; // Clear the next state flag since we're now handling the transition
        smf_set_state(SMF_CTX(&ui_state_object), &ui_states[MAIN_MENU]);
    }
    else if (next_state == PERFORMANCE_METRICS) {
        next_state = -1; // Clear the next state flag since we're now handling the transition
        smf_set_state(SMF_CTX(&ui_state_object), &ui_states[PERFORMANCE_METRICS]);
    }

    return SMF_EVENT_HANDLED;
}