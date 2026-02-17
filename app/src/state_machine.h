/**
 * @file state_machine.h
 */

#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

/**
 * Function prototypes
 */

void state_machine_init();

int state_machine_run();

// Main menu
static void main_menu_on_state_entry(void* o);
static enum smf_state_result main_menu_on_state_run(void* o);

// Performance metrics page
static void performance_metrics_on_state_entry(void* o);
static enum smf_state_result performance_metrics_on_state_run(void* o);

// Media controls page
static void media_controls_on_state_entry(void* o);
static enum smf_state_result media_controls_on_state_run(void* o);

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

#endif  