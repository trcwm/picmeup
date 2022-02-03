#pragma once

#include <stdint.h>

class ISP
{
public:
    void init();

    void send(uint8_t data, uint8_t n);
    
    uint16_t read16(void);
    uint16_t read14s(void);
    uint8_t  read8(void);
    
    void readPgm(uint8_t* data, uint8_t n);
    void writePgm(uint8_t* data, uint8_t n);
    void massErase(void);
    void resetPointer(void);

    void send_8_msb(unsigned char data);
};


