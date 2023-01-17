#ifndef atmega328_uart_h
#define atmega328_uart_h

#include <avr/io.h>
#include <stdbool.h>

/** setup the io pins and baud rate etc. 
 *  115.2kbaud
*/
void UART_setup();

/** read a character from the UART, time-out in 1 second 
 *  uses timer 1 for time-outs.
*/
bool UART_read(char *c);

/** returns true if the UART has data to read*/
bool UART_hasData();

/** write a single character. blocking. */
void UART_write(const char c);

/** write a null-terminated string. blocking. */
void UART_writeString(const char *c);

#endif