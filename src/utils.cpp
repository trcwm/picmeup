// SPDX-License-Identifier: GPL-3.0-only
// Copyright N.A. Moseley 2022

#include <sstream>
#include <algorithm>
#include <iostream>
#include "utils.h"

bool Utils::isDigit(char c)
{
    return ((c>='0') && (c<='9'));
}


bool Utils::isHexDigit(char c)
{
    if (Utils::isDigit(c))
    {
         return true;
    }

    if ((c>='a') && (c<='f'))
    {
        return true;
    }
    
    if ((c>='A') && (c<='F'))
    {
        return true;
    }

    return false;
}

std::optional<int32_t> Utils::intStrToint32(const std::string &str)
{
    for(auto c : str)
    {        
        if (!isDigit(c))
        {
            return std::nullopt;
        }
    }    

    int32_t result;
    std::stringstream ss;
    ss << str;
    ss >> result;
    return result;    
}

std::optional<uint32_t> Utils::hexStrToUint32(const std::string &str)
{
    for(auto c : str)
    {
        if (!isHexDigit(c))
        {
            return std::nullopt;
        }
    }

    uint32_t result;
    std::stringstream ss;
    ss << std::hex << str;
    ss >> result;
    return result;
}

std::string Utils::toHex(uint32_t v, uint32_t digits)
{
    std::string result;
    while(digits > 0)
    {
        digits--;
        auto nibble = (v >> (digits*4)) & 0xF;
        char digit = ' ';
        if (nibble > 9)
        {
            digit = 'A' + nibble-10;
        }
        else
        {
            digit = '0' + nibble;
        }
        result += digit;
    }

    return result;
}

std::vector<std::string> Utils::tokenize(const std::string &str, const char delim)
{
    std::vector<std::string> out;
    size_t start;
    size_t end = 0;
 
    while ((start = str.find_first_not_of(delim, end)) != std::string::npos)
    {
        end = str.find(delim, start);
        out.push_back(str.substr(start, end - start));
    }

    return out;
}

std::string Utils::toLower(const std::string &str)
{
    auto result = str;

    std::transform(result.begin(), result.end(), result.begin(), 
        [](auto c)
        {
            return std::tolower(c);
        }
    );

    return result;
}

bool Utils::isEmptyMem(const std::vector<uint8_t> &mem, size_t start, size_t len)
{
    if ((len & 1) != 0)
    {
        std::cerr << "Error: isEmptyMem called with odd number of bytes\n";
        return false;
    }

    while(len > 0)
    {
        auto low = static_cast<uint16_t>(mem.at(start));
        auto hi  = static_cast<uint16_t>(mem.at(start+1));

        //FIXME: for PIC16, the program word is 14 bits
        //       so we need to check against 0x3FFF
        //       however, for other parts it might be wider..

        auto word = low | (hi<<8);
        if (word != 0x3FFF)
        {
            return false;
        }

        start += 2;
        len-=2;
    }
    return true;
}

bool Utils::isEmptyMem(const std::vector<uint8_t> &mem)
{
    auto len = mem.size();
    return isEmptyMem(mem, 0, len);
}
