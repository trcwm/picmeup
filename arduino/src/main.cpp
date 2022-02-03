#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include "msghandler.h"

int main()
{
    MessageHandler handler;
    handler.init();

    DDRB = (1<<5); // setting DDR of PORT B
    while (1)
    {
        // LED on
        PORTB |= 1<<5;      // PC0 = High = LED attached on PC0 is ON
        //_delay_ms(500);     // wait 500 milliseconds

        //LED off
        PORTB &= ~(1<<5);      // PC0 = Low = LED attached on PC0 is OFF
        //_delay_ms(500);     // wait 500 milliseconds

        if (handler.tick())
        {
            // handler has a message ready
        }
    }
    return 0;
}
