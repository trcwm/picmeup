#ifndef timer_h
#define timer_h

// timer support, primarily used for delay
// or time-out functions.

#include <stdint.h>
#include <stdbool.h>

// setup timer (timer counter 1) to generate ticks
void TIMER_setup();

// reset the timer/counter 1 value to 0
uint16_t TIMER_read();

// get the current count for timer/counter 1
void TIMER_reset();

// wait a number of milliseconds
// this resets timer/counter 1
void wait_ms(uint16_t ms);

// returns true if a time-out has occurred.
// you need to reset timer before this works.
volatile bool checkTimeout(uint16_t ms);

#endif