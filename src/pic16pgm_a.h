// SPDX-License-Identifier: GPL-3.0-only
// Copyright N.A. Moseley 2022

#pragma once
#include "serial.h"
#include "pic16a.h"

class PIC16PGM_A : public PIC16A
{
public:
    PIC16PGM_A(std::shared_ptr<Serial> serial) : PIC16A(serial) {}

    void massErase() override;
    
    void enterProgMode() override;
    void exitProgMode() override;
};
