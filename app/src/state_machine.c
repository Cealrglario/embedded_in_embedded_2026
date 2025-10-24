/**
 * @file state_machine.c
 */

#include <zephyr/smf.h>

#include "led.h"
#include "state_machine.h"

/*-------------------------------------------------------------------------------------
 * Function Prototypes
 *-----------------------------------------------------------------------------------*/
static void led_on_state_entry(void* o);
static enum smf_state_result led_on_state_run(void* o);
static void led_off_state_entry(void* o);
static enum smf_state_result led_off_state_run(void* o);

 /*-------------------------------------------------------------------------------------
 * Typedefs
 *-----------------------------------------------------------------------------------*/
enum led_state_machine_states {
    LED_ON_STATE = 0,
    LED_OFF_STATE,
};

typedef struct {
    // Context variable used by Zephyr to track state machine state (the current state). Must be first
    struct smf_ctx ctx;

    // Counter to keep track of when states need to be changed
    uint16_t count;
} led_state_object_t;

 /*-------------------------------------------------------------------------------------
 * Local Variables
 *-----------------------------------------------------------------------------------*/
static const struct smf_state led_states[] = {  
    [LED_ON_STATE] = SMF_CREATE_STATE(led_on_state_entry, led_on_state_run, NULL, NULL, NULL),
    [LED_OFF_STATE] = SMF_CREATE_STATE(led_off_state_entry, led_off_state_run, NULL, NULL, NULL)

};

static led_state_object_t led_state_object;

void init_state_machine() {
    led_state_object.count = 0;
    
    // Set the initial state of the state machine
    smf_set_initial(SMF_CTX(&led_state_object), &led_states[LED_ON_STATE]);
}

int run_state_machine() {
    return smf_run_state(SMF_CTX(&led_state_object));
}

static void led_on_state_entry(void* o) {
    LED_set(LED0, LED_ON);
}

static enum smf_state_result led_on_state_run(void* o) {
    if(led_state_object.count > 500) {
        // reset the count
        led_state_object.count = 0;

        // turn the LED off (go to the LED off state)
        smf_set_state(SMF_CTX(&led_state_object), &led_states[LED_OFF_STATE]);
    } else {
        led_state_object.count++;
    }

    // Standard return result for non-hierarchical state machines
    return SMF_EVENT_HANDLED;
}

static void led_off_state_entry(void* o) {
    LED_set(LED0, LED_OFF);
}

static enum smf_state_result led_off_state_run(void* o) {
    if(led_state_object.count > 500) {
        // reset the count
        led_state_object.count = 0;

        // turn the LED off (go to the LED off state)
        smf_set_state(SMF_CTX(&led_state_object), &led_states[LED_ON_STATE]);
    } else {
        led_state_object.count++;
    }
    
    // Standard return result for non-hierarchical state machines
    return SMF_EVENT_HANDLED;
}