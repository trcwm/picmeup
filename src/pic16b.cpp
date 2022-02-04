#include <iostream>
#include "pic16b.h"

void PIC16B::writeConfig(const std::vector<uint8_t> &config)
{
    if (config.size() != 6)
    {
        std::cerr << "Error: writeConfig requires 4 bytes\n";
        return;
    }

    resetPointer();
    loadConfig();
    incPointer(7);

    std::vector<uint8_t> word1;
    std::vector<uint8_t> word2;
    std::vector<uint8_t> word3;

    word1.push_back(config.at(0));
    word1.push_back(config.at(1));
    word2.push_back(config.at(2));
    word2.push_back(config.at(3));
    word3.push_back(config.at(4));
    word3.push_back(config.at(5));

    writePage(word1); // slow write
    writePage(word2); // slow write
    writePage(word3); // slow write
}
