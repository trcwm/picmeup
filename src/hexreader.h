// SPDX-License-Identifier: GPL-3.0-only
// Copyright N.A. Moseley 2022

#pragma
#include <cstdint>
#include <vector>
#include "utils.h"

namespace HexReader
{

    bool read(const std::string &filename,
            std::vector<uint8_t> &flash,
            std::vector<uint8_t> &config);
    
};

