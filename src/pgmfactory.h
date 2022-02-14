// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include "devicepgminterface.h"

class ProgrammerFactory
{
public:
    static std::shared_ptr<IDeviceProgrammer> create(const std::string &deviceFamily, 
        std::shared_ptr<Serial> serial);
};
