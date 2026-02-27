/**
 * @file state_machine.c
 */

#include <zephyr/smf.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/gpio.h>
#include <lvgl.h>

#include "state_machine.h"
#include "touchscreen_defines.h"
#include "lv_data_obj.h"
#include "BTN.h"

/**
 * Function prototypes
 * 
 * (Declare any functions here that are only to be used within this source file and nowhere else)
 */

// Main menu
static void main_menu_on_state_entry(void* o);
static enum smf_state_result main_menu_on_state_run(void* o);

// Performance metrics page
static void performance_metrics_on_state_entry(void* o);
static enum smf_state_result performance_metrics_on_state_run(void* o);

// Media controls page
static void media_controls_on_state_entry(void* o);
static enum smf_state_result media_controls_on_state_run(void* o);

// Button press menu transition callback
void lv_change_menu_cb(lv_event_t* event);

/**
 * Typedefs
 */

enum ui_state_machine_states {
    MAIN_MENU,
    PERFORMANCE_METRICS,
    MEDIA_CONTROLS
};

// Object that Zephyr uses to keep track of current state (this is what is constantly ran inside the super loop)
typedef struct {
    // Context variable used by Zephyr to track state machine state, must be declared first
    struct smf_ctx ctx;

    // Other variables that can help in keeping track of proper state should be added here

} ui_state_object_t;

/**
 * Local variables
 */

// Struct representing the menu "back" button
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(SW0_NODE, gpios);

// Static struct that holds the current state to run during runtime
static ui_state_object_t ui_state_object;

// An LVGL object representing the LCD we will be displaying content onto
extern lv_obj_t* screen;

// Struct definition to organize our performance metrics data to display onto our LCD
typedef struct {
    // The only things that should be stored in this struct are labels/text that need to be constantly updated during runtime,
    // i.e. our performance metrics updated over bluetooth

    // Scalar metrics (top container)
    lv_obj_t* label_cpu_clock;
    lv_obj_t* label_cpu_power;
    lv_obj_t* label_cpu_temp;
    lv_obj_t* label_gpu_temp;
    lv_obj_t* label_net_upload;
    lv_obj_t* label_net_download;
    
    // Percentage-range metrics (bottom container)
    lv_obj_t* bar_cpu_usage;
    lv_obj_t* bar_gpu_usage;
    lv_obj_t* bar_ram_usage;
} perf_metrics_ui_t;

// Static struct that ACTUALLY holds our performance metrics data that we're updated during runtime
static perf_metrics_ui_t perf_metrics_ui;

// Struct that holds the actual states that Zephyr will traverse throughout runtime
static const struct smf_state ui_states[] = {
    [MAIN_MENU] = SMF_CREATE_STATE(main_menu_on_state_entry, main_menu_on_state_run, NULL, NULL, NULL),
    [PERFORMANCE_METRICS] = SMF_CREATE_STATE(performance_metrics_on_state_entry, performance_metrics_on_state_run, NULL, NULL, NULL),
    [MEDIA_CONTROLS] = SMF_CREATE_STATE(media_controls_on_state_entry, media_controls_on_state_run, NULL, NULL, NULL)
};

// Indicates the next state to transition to
static enum ui_state_machine_states next_state = -1; // -1 indicates no state change request

// Objects that hold the enum value of states that can be transitioned to for button callback pointers
static enum ui_state_machine_states perf_metrics_state = PERFORMANCE_METRICS;
static enum ui_state_machine_states media_controls_state = MEDIA_CONTROLS;

void state_machine_init() {
    // Set initial state to be the main menu
    smf_set_initial(SMF_CTX(&ui_state_object), &ui_states[MAIN_MENU]);
}

int state_machine_run() {
    // When we run the state machine, we just want to return the state currently held in the ui_state_object
    return smf_run_state(SMF_CTX(&ui_state_object));
}

// Definition of button press menu transition callback
void lv_change_menu_cb(lv_event_t* event) {
    // Retrieve the next state data from the associated button
    lv_obj_t* transition_state_obj = (lv_obj_t*) lv_event_get_user_data(event);
    enum ui_state_machine_states transition_state_data = *(enum ui_state_machine_states*) lv_data_obj_get_data_ptr(transition_state_obj);

    // Transition to the next state
    next_state = transition_state_data;
}

/**
 * Main menu states
 */

static void main_menu_on_state_entry(void* o) {
    // Clear any existing screen contents to display the new menu
    lv_obj_clean(screen);

    // We would initialize objects to display onto the LVGL screen here
    // for (uint8_t i = 0; i < HOME_SCREEN_BUTTONS; i++) {
    //     lv_obj_t* ui_btn = lv_button_create(screen); // Add a button to the screen

    //     // Place the buttons in the center of the screen
    //     lv_obj_align(ui_btn, LV_ALIGN_CENTER, 0, VERTICAL_SPACING_MULTIPLIER * (i % 2 ? 1 : -1));

    //     // Add text to the button
    //     lv_obj_t* button_label = lv_label_create(ui_btn); 
    //     char label_text[BUTTON_TEXT_MAX_LENGTH];
    //     snprintf(label_text, BUTTON_TEXT_MAX_LENGTH, i % 2 ? "Performance Metrics" : "Media Controls");
    //     lv_label_set_text(button_label, label_text);
    //     lv_obj_align(button_label, LV_ALIGN_CENTER, 0, 0);
    // }

    lv_obj_t* button_container = lv_obj_create(screen);
    lv_obj_set_size(button_container, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(button_container, LV_FLEX_FLOW_COLUMN); // Display objects neatly top-to-bottom with flex
    lv_obj_set_flex_align(button_container, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER);

    // We want the buttons to be directly in the center of the screen
    lv_obj_set_align(button_container, LV_ALIGN_CENTER);
    
    // Create the Performance Metrics navigation button and associate the state
    lv_obj_t* perf_metrics_button = lv_button_create(button_container);
    lv_obj_t* perf_metrics_text = lv_label_create(perf_metrics_button); // add the button text
    lv_label_set_text(perf_metrics_text, "Performance Metrics");
    
    // Data to send to the menu change callback when the button is clicked
    lv_obj_t* perf_state = lv_data_obj_create_alloc_assign(perf_metrics_button, &perf_metrics_state, sizeof(PERFORMANCE_METRICS));

    // When the Performance Metrics button is clicked, we want to transition to that state/menu
    lv_obj_add_event_cb(perf_metrics_button, lv_change_menu_cb, LV_EVENT_CLICKED, perf_state);

    // // Create the Media Controls button and associate the state
    // lv_obj_t* media_controls_button = lv_button_create(button_container);
    // lv_obj_t* media_controls_text = lv_label_create(media_controls_button); // add the button text
    // lv_label_set_text(media_controls_text, "Media Controls");

    // // Data to send to the menu change callback when the button is clicked
    // lv_obj_t* media_state = lv_data_obj_create_alloc_assign(media_controls_button, &media_controls_state, sizeof(MEDIA_CONTROLS));

    // // When the Media Controls button is clicked, we want to transition to that state/menu
    // lv_obj_add_event_cb(media_controls_button, lv_change_menu_cb, LV_EVENT_CLICKED, media_state);
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

    // FLEX LAYOUTS should be used to organize UI widgets/elements, not hard-coded coordinates

    // Declare and initialize UI components here (when they're actually needed) so that RAM isn't wasted storing unused variables.
    // Similar to initializing and adding buttons to the screen in main_menu_entry, adding components to the "screen" here actually
    // adds them to the heap which LVGL handles on its own end. The components are NOT purely local stack variables just because
    // they're declared within this function. In reality, they persist on the heap until "cleared".

    /**
     * An exception to the above statements is our metrics widgets that must be constantly updated via bluetooth. These
     * metrics need to be explicitly declared statically similar to our "screen" variable so that their data is persistent 
     * throughout runtime since these hold actual data unlike other LVGL components which are essentially purely visual.
     * These metrics will be "fed" to the visual LVGL components to display metrics on our LCD.
     */

    /**
     * "Back" button initialization
     */

    

    /**
     * Top container initialization (scalar metrics)
     */

    lv_obj_t* perf_top_container = lv_obj_create(screen);
    lv_obj_set_size(perf_top_container, lv_pct(100), lv_pct(50));
    lv_obj_set_flex_flow(perf_top_container, LV_FLEX_FLOW_ROW_WRAP); // Display objects neatly side-by-side with flex
    lv_obj_set_flex_align(perf_top_container, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER);

    // Anchor this to the TOP of the screen so it actually appears on top
    lv_obj_align(perf_top_container, LV_ALIGN_TOP_MID, 0, 0);

    // Create labels, add to our static metrics struct so that we can update them with new data (via bluetooth) later.
    // These labels should be children of the container so they are contained within them.
    perf_metrics_ui.label_cpu_clock = lv_label_create(perf_top_container);
    lv_label_set_text(perf_metrics_ui.label_cpu_clock, "CPU Clock: --"); 

    perf_metrics_ui.label_cpu_power = lv_label_create(perf_top_container);
    lv_label_set_text(perf_metrics_ui.label_cpu_power, "CPU Power: --");

    perf_metrics_ui.label_cpu_temp = lv_label_create(perf_top_container);
    lv_label_set_text(perf_metrics_ui.label_cpu_temp, "CPU Temp: --");
    
    perf_metrics_ui.label_gpu_temp = lv_label_create(perf_top_container);
    lv_label_set_text(perf_metrics_ui.label_gpu_temp, "GPU Temp: --");

    perf_metrics_ui.label_net_download = lv_label_create(perf_top_container);
    lv_label_set_text(perf_metrics_ui.label_net_download, "Net Down: --");

    perf_metrics_ui.label_net_upload = lv_label_create(perf_top_container);
    lv_label_set_text(perf_metrics_ui.label_net_upload, "Net Up: --");

    /**
     * Bottom container initialization (percentage-based metrics)
     */

    lv_obj_t* perf_bottom_container = lv_obj_create(screen);
    lv_obj_set_size(perf_bottom_container, lv_pct(100), lv_pct(50));
    lv_obj_set_flex_flow(perf_bottom_container, LV_FLEX_FLOW_COLUMN); // Display bars nearly top-to-bottom in a vertical stack
    lv_obj_set_flex_align(perf_bottom_container, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER,
         LV_FLEX_ALIGN_CENTER);

    // Anchor this to the BOTTOM of the screen so it actually appears on the bottom
    lv_obj_align(perf_bottom_container, LV_ALIGN_BOTTOM_MID, 0, 0);

    lv_obj_t* cpu_usage_title = lv_label_create(perf_bottom_container);
    lv_label_set_text(cpu_usage_title, "CPU Usage (%):"); // We will only be updating the percentage bar, not the bar title,
                                                                // so this doesn't have to be stored in our static struct

    // Dynamic CPU usage percentage bar (store in static struct for future updates)
    perf_metrics_ui.bar_cpu_usage = lv_bar_create(perf_bottom_container);
    lv_obj_set_size(perf_metrics_ui.bar_cpu_usage, lv_pct(90), 20);
    lv_bar_set_range(perf_metrics_ui.bar_cpu_usage, 0, 100);

    lv_obj_t* gpu_usage_title = lv_label_create(perf_bottom_container);
    lv_label_set_text(gpu_usage_title, "GPU Usage (%):"); // We will only be updating the percentage bar, not the bar title,
                                                                // so this doesn't have to be stored in our static struct
                                                                
    // Dynamic GPU usage percentage bar (store in static struct for future updates)
    perf_metrics_ui.bar_gpu_usage = lv_bar_create(perf_bottom_container);
    lv_obj_set_size(perf_metrics_ui.bar_gpu_usage, lv_pct(90), 20);
    lv_bar_set_range(perf_metrics_ui.bar_gpu_usage, 0, 100);

    lv_obj_t* ram_usage_title = lv_label_create(perf_bottom_container);
    lv_label_set_text(ram_usage_title, "RAM Usage (%):"); // We will only be updating the percentage bar, not the bar title,
                                                                // so this doesn't have to be stored in our static struct
                                                                
    // Dynamic RAM usage percentage bar (store in static struct for future updates)
    perf_metrics_ui.bar_ram_usage = lv_bar_create(perf_bottom_container);
    lv_obj_set_size(perf_metrics_ui.bar_ram_usage, lv_pct(90), 20);
    lv_bar_set_range(perf_metrics_ui.bar_ram_usage, 0, 16);

}

static enum smf_state_result performance_metrics_on_state_run(void* o) {
    lv_timer_handler();
    
    if (gpio_pin_get_dt(&button)) {
        // Go back to the main menu
        smf_set_state(SMF_CTX(&ui_state_object), &ui_states[MAIN_MENU]);
    }

    return SMF_EVENT_HANDLED;
}

/**
 * Media controls states
 */
static void media_controls_on_state_entry(void* o) {
    // Clear any existing screen contents to display the new menu
    lv_obj_clean(screen);

    // FLEX LAYOUTS should be used to organize UI widgets/elements, not hard-coded coordinates

    // Declare and initialize UI components here (when they're actually needed) so that RAM isn't wasted storing unused variables.
    // Similar to initializing and adding buttons to the screen in main_menu_entry, adding components to the "screen" here actually
    // adds them to the heap which LVGL handles on its own end. The components are NOT purely local stack variables just because
    // they're declared within this function. In reality, they persist on the heap until "cleared".

    /**
     * An exception to the above statements is our metrics widgets that must be constantly updated via bluetooth. These
     * metrics need to be explicitly declared statically similar to our "screen" variable so that their data is persistent 
     * throughout runtime since these hold actual data unlike other LVGL components which are essentially purely visual.
     * These metrics will be "fed" to the visual LVGL components to display metrics on our LCD.
     */
}

static enum smf_state_result media_controls_on_state_run(void* o) {
    lv_timer_handler();
    
    if (gpio_pin_get_dt(&button)) {
        // Go back to the main menu
        smf_set_state(SMF_CTX(&ui_state_object), &ui_states[MAIN_MENU]);
    }

    return SMF_EVENT_HANDLED;
}