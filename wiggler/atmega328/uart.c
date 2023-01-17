#include "uart.h"
#include "timer.h"

// https://wormfood.net/avrbaudcalc.php
#define CALC_UBBR(baud) (F_CPU/16/baud-1)

/** setup the io pins and baud rate etc. 
 *  115.2kbaud
 *  PD1 is USART TX
 *  PD0 is USART RX
*/
void UART_setup()
{
    // set baud rate
    const uint16_t br = CALC_UBBR(9600);
    UBRR0H = (br >> 8); 
    UBRR0L = br;
    UCSR0B = (1<<RXEN0) | (1<<TXEN0);   // Enable the receiver.
    UCSR0C = 0x06;                      // Set frame format: 8 data, 1 stop bit.    
}

/** read a character from the UART, time-out in 1 second */
bool UART_read(char *c)
{
    TIMER_reset();
    while(!(UCSR0A & (1<<RXC0))) 
    {
        if (checkTimeout(1000))
        {
            return false;
        }
    };
    
    *c = UDR0;
    return true;
}

bool UART_hasData()
{
    return !(UCSR0A & (1<<RXC0));
}

void UART_write(const char c)
{
    while (!( UCSR0A & (1<<UDRE0))) {}; // wait for TX register to be empty
    UDR0 = c;
}

void UART_writeString(const char *c)
{
    while(*c != 0)
    {
        UART_write(*c);
    }
}
