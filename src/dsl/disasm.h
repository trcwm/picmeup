#pragma once

#include "vm.h"

namespace VM
{
    struct DisasmResult
    {
        std::string m_txt;
        size_t      m_bytes = 0;
    };

    DisasmResult disasm(const uint8_t *code);
};

