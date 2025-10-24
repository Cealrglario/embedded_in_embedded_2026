/**
 * @file state_machine.c
 */

#include <zephyr/smf.h>

#include "led.h"
#include "BTN.h"
#include "state_machine.h"

// Define length of password
#define PASSWORD_LENGTH 3

/*-------------------------------------------------------------------------------------
 * Function Prototypes
 *-----------------------------------------------------------------------------------*/
static void locked_state_entry(void* o);
static enum smf_state_result locked_state_run(void* o);
static void waiting_state_entry(void* o);
static enum smf_state_result waiting_state_run(void* o);

 /*-------------------------------------------------------------------------------------
 * Typedefs
 *-----------------------------------------------------------------------------------*/
enum password_state_machine_states {
    LOCKED_STATE = 0,
    WAITING_STATE,
};

typedef struct {
    // Context variable used by Zephyr to track state machine state (the current state). Must be first
    struct smf_ctx ctx;
} password_state_object_t;

 /*-------------------------------------------------------------------------------------
 * Local Variables
 *-----------------------------------------------------------------------------------*/
static const struct smf_state password_states[] = {  
    [LOCKED_STATE] = SMF_CREATE_STATE(locked_state_entry, locked_state_run, NULL, NULL, NULL),
    [WAITING_STATE] = SMF_CREATE_STATE(waiting_state_entry, waiting_state_run, NULL, NULL, NULL)

};

static password_state_object_t password_state_object;
static int password[PASSWORD_LENGTH] = {1, 0, 1};
static int user_entry[PASSWORD_LENGTH] = {0, 0, 0};

void init_state_machine() {
    // Set the initial state of the state machine
    smf_set_initial(SMF_CTX(&password_state_object), &password_states[LOCKED_STATE]);
}

int run_state_machine() {
    return smf_run_state(SMF_CTX(&password_state_object));
}

static void locked_state_entry(void* o) {
    printk("Entering locked state...\n");
    LED_set(LED0, LED_ON);
}

static enum smf_state_result locked_state_run(void* o) {
    k_cpu_idle();

    // check buttons 0 to 2 for user input
    for (int i = 0; i < NUM_BTNS - 1; i++) {
        if(BTN_check_clear_pressed(i)) {
            user_entry[i] = 1;
        }

        k_cpu_idle();
    }

    for (int i = 0; i < PASSWORD_LENGTH; i++) {
        if(password[i] != user_entry[i]) {
            printk("Incorrect!\n");
            smf_set_state(SMF_CTX(&password_state_object), &password_states[WAITING_STATE]);
            return SMF_EVENT_HANDLED;
        }
    }

    printk("Correct!\n");
    smf_set_state(SMF_CTX(&password_state_object), &password_states[WAITING_STATE]);
    return SMF_EVENT_HANDLED;
}

static void waiting_state_entry(void* o) {
    printk("Entering waiting state...\n");
    LED_set(LED0, LED_OFF);
}

static enum smf_state_result waiting_state_run(void* o) {
    k_cpu_idle();

    for (int i = 0; i < NUM_BTNS; i++) {
        if(BTN_check_clear_pressed(i)) {
            smf_set_state(SMF_CTX(&password_state_object), &password_states[LOCKED_STATE]);
        }
    }

    return SMF_EVENT_HANDLED;
}