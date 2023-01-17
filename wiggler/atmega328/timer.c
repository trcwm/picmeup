#include <avr/io.h>
#include "timer.h"

#define TICKS_PER_MS (F_CPU/1024/1000)

void TIMER_setup()
{
    TCCR1A = 0;     // no IO pins used, wave generation: normal operation.
    TCCR1B = 0x05;  // prescaler set to DIV 1024
    TCCR1C = 0;
}

uint16_t TIMER_read()
{
    return TCNT1;
}

void TIMER_reset()
{
    TCNT1 = 0;
}

void wait_ms(uint16_t ms)
{
    const uint16_t ticks = ms * TICKS_PER_MS;
    TIMER_reset();
    while(TCNT1 < ticks) {};
}

volatile bool checkTimeout(uint16_t ms)
{
    const uint16_t ticks = ms * TICKS_PER_MS;
    if (TCNT1 >= ticks) return true;
    return false;
}
