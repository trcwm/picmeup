// SPDX-License-Identifier: GPL-3.0-only
// Copyright N.A. Moseley 2022

#define F_CPU 16000000
#define BAUD 57600

#include <avr/io.h>
#include <util/setbaud.h>
#include "uart.h"

void UART::init()
{
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;    

#if USE_2X
    UCSR0A |= _BV(U2X0);
#else
    UCSR0A &= ~(_BV(U2X0));
#endif

    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);     /* 8-bit data */
    UCSR0B = _BV(RXEN0)  | _BV(TXEN0);      /* Enable RX and TX */    
}

void UART::write(uint8_t byte)
{
    loop_until_bit_is_set(UCSR0A, UDRE0);   /* Wait until data register empty */
    UDR0 = byte;
}

uint8_t UART::read()
{
    loop_until_bit_is_set(UCSR0A, RXC0);    /* Wait until data exists. */
    return UDR0;
}

bool UART::hasData() const
{
    return (UCSR0A & _BV(RXC0));
}
