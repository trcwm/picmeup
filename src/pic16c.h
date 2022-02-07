#pragma once

#include "pic16a.h"

class PIC16C : public PIC16A
{
public:
    PIC16C(std::shared_ptr<Serial> serial) : PIC16A(serial) {}

    //void writeConfig(const std::vector<uint8_t> &config) override;
};
