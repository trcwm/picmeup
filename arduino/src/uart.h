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
