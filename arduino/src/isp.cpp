// SPDX-License-Identifier: GPL-3.0-only
// Copyright N.A. Moseley 2022

#define F_CPU 16000000

#include <avr/io.h>
#include <util/delay.h>
#include "isp.h"

#include "../../src/pgmops.h"

#define ISP_PORT  PORTC
#define ISP_DDR   DDRC
#define ISP_PIN   PINC
#define ISP_MCLR  3
#define ISP_PGM   2
#define ISP_DAT   1
#define ISP_CLK   0

#define  ISP_MCLR_1   ISP_PORT |= (1<<ISP_MCLR);
#define  ISP_MCLR_0   ISP_PORT &= ~(1<<ISP_MCLR);
#define  ISP_MCLR_D_I ISP_DDR  &= ~(1<<ISP_MCLR);
#define  ISP_MCLR_D_0 ISP_DDR  |= (1<<ISP_MCLR);

#define  ISP_DAT_1    ISP_PORT |= (1<<ISP_DAT);
#define  ISP_DAT_0    ISP_PORT &= ~(1<<ISP_DAT);
#define  ISP_DAT_V    (ISP_PIN&(1<<ISP_DAT))
#define  ISP_DAT_D_I  ISP_DDR &= ~(1<<ISP_DAT);
#define  ISP_DAT_D_0  ISP_DDR |= (1<<ISP_DAT);

#define  ISP_CLK_1    ISP_PORT |= (1<<ISP_CLK);
#define  ISP_CLK_0    ISP_PORT &= ~(1<<ISP_CLK);
#define  ISP_CLK_D_I  ISP_DDR &= ~(1<<ISP_CLK);
#define  ISP_CLK_D_0  ISP_DDR |= (1<<ISP_CLK);

#define  ISP_PGM_1    ISP_PORT |= (1<<ISP_PGM);
#define  ISP_PGM_0    ISP_PORT &= ~(1<<ISP_PGM);
#define  ISP_PGM_D_I  ISP_DDR &= ~(1<<ISP_PGM);
#define  ISP_PGM_D_0  ISP_DDR |= (1<<ISP_PGM);


#define  ISP_CLK_DELAY  4

void ISP::init()
{
    ISP_CLK_D_0
    ISP_DAT_D_0
    ISP_DAT_0
    ISP_PGM_D_0
    ISP_PGM_0
    ISP_CLK_0
    ISP_MCLR_D_0
    ISP_MCLR_1    
}

void ISP::send(uint16_t data, const uint8_t n)
{
    ISP_DAT_D_0

    for(uint8_t i=0; i<n; i++)
    {
        if (data & 0x01)
        {
            ISP_DAT_1
        }
        else
        {
            ISP_DAT_0
        }
        
        _delay_us(ISP_CLK_DELAY);
        ISP_CLK_1
        
        data >>= 1;

        ISP_CLK_0
        ISP_DAT_0
    }
}

uint16_t ISP::read16(void)
{
    uint16_t out = 0;
    ISP_DAT_D_I
    for(uint8_t i=0; i<16; i++)
    {
        ISP_CLK_1
        _delay_us(ISP_CLK_DELAY);
        ISP_CLK_0
        _delay_us(ISP_CLK_DELAY);
        out = out >> 1;
        if (ISP_DAT_V)
            out = out | 0x8000;
    }
    return out;    
}

uint8_t ISP::read8(void)
{
    uint8_t out = 0;
    ISP_DAT_D_I
    for(uint8_t i=0; i<8; i++)
    {
        ISP_CLK_1
        _delay_us(ISP_CLK_DELAY);
        ISP_CLK_0
        _delay_us(ISP_CLK_DELAY);
        out = out >> 1;
        if (ISP_DAT_V)
            out = out | 0x80;
    }
    return out;    
}

void ISP::readPgm(uint16_t* data, uint8_t n)
{
    for (uint8_t i=0; i<n; i++)
    {
        send(0x04, 6);      // Read Data From Program Memory
        data[i] = read14s();
        incrementPointer();
    }    
}

uint16_t ISP::read14s(void)
{
    return (read16() & 0x7FFE) >> 1;
}

void ISP::writePgm(uint16_t *data, uint8_t n)
{
    for (uint8_t i=0; i<n; i++)  
    {
        send(0x02,6);   // load data for program memory
        send(data[i]<<1,16);  
        if (i != (n-1))
        {
            incrementPointer();
        }
    }
    
    send(0x08,6);       // Begin Internally Timed Programming
    
    const uint8_t slow = 1;

    if (slow==1)
        _delay_ms(5);
    else
        _delay_ms(3);

    incrementPointer();
}

void ISP::loadConfig(uint16_t data)
{
    send(0x00, 6);      // Load Configuration 
    send(data, 16);
}

void ISP::massErase()
{
    loadConfig(0);
    send(0x09, 6);      // internally timed bulk erase
    _delay_ms(10);
}

void ISP::resetPointer()
{
    send(0x16,6);
}

void ISP::incrementPointer()
{
    send(0x06,6);
}

void ISP::enterProgMode()
{
    // see: https://ww1.microchip.com/downloads/en/DeviceDoc/41573C.pdf
    ISP_MCLR_0
    _delay_us(300);
    send(0b01010000,8);
    send(0b01001000,8);
    send(0b01000011,8);
    send(0b01001101,8);
    send(0,1);
}

void ISP::exitProgMode()
{
    ISP_MCLR_1
    _delay_ms(30);
    ISP_MCLR_0
    _delay_ms(30);
    ISP_MCLR_1
}

void ISP::enterProgModeWithPGMPin()
{
    // for older devices such as PIC16F87X
    ISP_MCLR_0
    _delay_us(300);      // spec is min. 5us for PIC16F87X    
    ISP_PGM_1
    _delay_us(1);       // spec is min. 100ns for PIC16F87X
    ISP_MCLR_1
    _delay_us(300);      // spec is min. 5us for PIC16F87X
}

void ISP::exitProgModeWithPGMPin()
{
    // for older devices such as PIC16F87X
    ISP_PGM_0
    ISP_MCLR_1
    _delay_ms(30);
    ISP_MCLR_0
    _delay_ms(30);
    ISP_MCLR_1    
}
