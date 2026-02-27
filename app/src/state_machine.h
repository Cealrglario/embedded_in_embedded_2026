/**
 * @file state_machine.h
 */

#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

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

#endif  