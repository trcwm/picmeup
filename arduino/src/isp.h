// SPDX-License-Identifier: GPL-3.0-only
// Copyright N.A. Moseley 2022

#pragma once

#include <stdint.h>

class ISP
{
public:
    void init();

    /** sends up to 16 bits, LSB first */
    void send(uint16_t data, const uint8_t n);
    
    uint16_t read16(void);
    uint16_t read14s(void);
    uint8_t  read8(void);
    
    void readPgm(uint16_t* data, uint8_t n);
    void writePgm(uint16_t* data, uint8_t n);

    void enterProgMode();
    void exitProgMode();

    void enterProgModeWithPGMPin();
    void exitProgModeWithPGMPin();

    void massErase(void);
    void resetPointer(void);
    void incrementPointer();

    void loadConfig(uint16_t data);
    void send_8_msb(unsigned char data);

    constexpr static uint16_t c_bufsize = 260;

    uint16_t m_flashBuffer[c_bufsize];
};
