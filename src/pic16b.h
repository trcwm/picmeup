// SPDX-License-Identifier: GPL-3.0-only
// Copyright N.A. Moseley 2022

#pragma once

#include "pic16a.h"

class PIC16B : public PIC16A
{
public:
    PIC16B(std::shared_ptr<Serial> serial) : PIC16A(serial) {}

    bool uploadConfig(const DeviceInfo &info, const std::vector<uint8_t> &config) override;
};
