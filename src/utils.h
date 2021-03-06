// SPDX-License-Identifier: GPL-3.0-only
// Copyright N.A. Moseley 2022

#pragma once
#include <optional>
#include <string>
#include <vector>

namespace Utils
{
    bool isDigit(char c);
    bool isHexDigit(char c);

    std::string toLower(const std::string &str);
    std::string toHex(uint32_t v, uint32_t digits = 4);

    std::optional<int32_t> intStrToint32(const std::string &str);
    std::optional<uint32_t> hexStrToUint32(const std::string &str);
    std::vector<std::string> tokenize(const std::string &str, const char delim);

    bool isEmptyMem(const std::vector<uint8_t> &mem, size_t start, size_t len);
    bool isEmptyMem(const std::vector<uint8_t> &mem);
};
