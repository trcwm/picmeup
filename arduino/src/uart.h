// SPDX-License-Identifier: GPL-3.0-only
// Copyright N.A. Moseley 2022

#pragma once

#include <stdint.h>

class UART
{
public:
    void    init();
    void    write(uint8_t byte);
    uint8_t read();
    bool    hasData() const;
};
