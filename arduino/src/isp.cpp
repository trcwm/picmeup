#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include "isp.h"

#define ISP_PORT  PORTC
#define ISP_DDR   DDRC
#define ISP_PIN   PINC
#define ISP_MCLR  3
#define ISP_DAT   1
#define ISP_CLK   0

#define  ISP_MCLR_1 ISP_PORT |= (1<<ISP_MCLR);
#define  ISP_MCLR_0 ISP_PORT &= ~(1<<ISP_MCLR);
#define  ISP_MCLR_D_I ISP_DDR &= ~(1<<ISP_MCLR);
#define  ISP_MCLR_D_0 ISP_DDR |= (1<<ISP_MCLR);

#define  ISP_DAT_1 ISP_PORT |= (1<<ISP_DAT);
#define  ISP_DAT_0 ISP_PORT &= ~(1<<ISP_DAT);
#define  ISP_DAT_V (ISP_PIN&(1<<ISP_DAT))
#define  ISP_DAT_D_I ISP_DDR &= ~(1<<ISP_DAT);
#define  ISP_DAT_D_0 ISP_DDR |= (1<<ISP_DAT);

#define  ISP_CLK_1 ISP_PORT |= (1<<ISP_CLK);
#define  ISP_CLK_0 ISP_PORT &= ~(1<<ISP_CLK);
#define  ISP_CLK_D_I ISP_DDR &= ~(1<<ISP_CLK);
#define  ISP_CLK_D_0 ISP_DDR |= (1<<ISP_CLK);

#define  ISP_CLK_DELAY  1

void ISP::init()
{
    ISP_CLK_D_0
    ISP_DAT_D_0
    ISP_DAT_0
    ISP_CLK_0
    ISP_MCLR_D_0
    ISP_MCLR_1    
}

void ISP::send(uint8_t data, uint8_t n)
{
    uint8_t i;
    ISP_DAT_D_0

    for (i=0; i<n; i++)
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
        
        data = data >> 1;

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

void ISP::readPgm(uint8_t* data, uint8_t n)
{
    for (uint8_t i=0; i<n; i++)  
    {
        send(0x04, 6);
        data[i] = read14s();
        send(0x06, 6);
    }    
}

uint16_t ISP::read14s(void)
{
    return (read16() & 0x7FFE) >> 1;
}

void ISP::writePgm(uint8_t *data, uint8_t n)
{
    for (uint8_t i=0; i<n; i++)  
    {
        send(0x02,6);
        send(data[i]<<1,16);  
        if (i != (n-1))
        {
            send(0x06,6);
        }
    }
    
    send(0x08,6);
    
    const uint8_t slow = 1;

    if (slow==1)
        _delay_ms(5);
    else
        _delay_ms(3);

    send(0x06,6);
}

void ISP::isp_mass_erase()
{

}

void ISP::resetPointer()
{
    isp_send(0x16,6);
}

void isp_send_8_msb(unsigned char data)
{

}
