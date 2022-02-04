#pragma
#include <cstdint>
#include <vector>
#include "utils.h"

namespace HexReader
{

    void read(const std::string &filename,
            std::vector<uint8_t> &flash,
            std::vector<uint8_t> &config);
    
};

