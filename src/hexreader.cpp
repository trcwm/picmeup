#include <iostream>
#include <fstream>
#include "hexreader.h"

#define IHEX_DATA   0
#define IHEX_EOL    1
#define IHEX_EXTSEG 2
#define IHEX_STARTSEGADDR 3
#define IHEX_EXTLIN 4
#define IHEX_STARTLINADD 5

void HexReader::read(const std::string &filename, 
    std::vector<uint8_t> &flash,
    std::vector<uint8_t> &config)
{
    const bool dump = true;
    std::ifstream hexfile(filename);
    if (!hexfile.is_open())
    {
        return;
    }

    uint32_t addressOffset = 0;
    while(!hexfile.eof())
    {
        std::string line;
        std::getline(hexfile, line);
        
        if (line.empty() || line.at(0) != ':')
        {
            continue;
        }

        auto lineLengthOpt  = Utils::hexStrToUint32(line.substr(1, 2));
        auto lineAddressOpt = Utils::hexStrToUint32(line.substr(3, 4));
        auto lineTypeOpt    = Utils::hexStrToUint32(line.substr(7, 2));

        if ((!lineLengthOpt) || (!lineAddressOpt) || (!lineTypeOpt))
        {
            std::cerr << "Error reading HEX file\n";
            return;
        }

        auto lineLength     = lineLengthOpt.value();
        auto lineAddress    = lineAddressOpt.value();
        auto lineType       = lineTypeOpt.value();

        uint32_t effectiveAddress = lineAddress + (addressOffset << 16);

        switch(lineType)
        {
        case IHEX_DATA:
            if constexpr (dump)
            {
                printf("\n%06X: ", effectiveAddress);
            }

            for(size_t i=0; i<lineLength; i++)
            {
                auto byteHexStr = line.substr(9 + i*2, 2);
                auto byteOpt = Utils::hexStrToUint32(byteHexStr);
                if (!byteOpt)
                {
                    std::cerr << "Error reading IHEX data record\n";
                    return;
                }
                
                uint32_t byte = byteOpt.value();

                if (effectiveAddress < flash.size())
                {
                    flash.at(effectiveAddress+i) = byte;
                }
                else if (addressOffset == 0x01)
                {
                    // Special code to get the configuration bits for PIC16:
                    config.at(lineAddress + i - 0xE) = byte;
                }

                if constexpr (dump)
                {
                    printf("%02X", byte);
                    fflush(stdout);
                }
            }
            break;
        case IHEX_EXTLIN:
            auto addrHexStr = line.substr(9, 4);
            auto addrOpt = Utils::hexStrToUint32(addrHexStr);
            if (!addrOpt)
            {
                std::cerr << "Error reading IHEX extended linear record\n";
                return;
            }   

            addressOffset = addrOpt.value();
            if constexpr (dump)
            {
                //printf("\noffset: %04X\n", addressOffset);
            }             
            break;
        }

        //std::cout << lineLength << " " << lineAddress << " " << lineType << "\n";
    }

    if constexpr (dump)
    {
        printf("\nok\n");
    }    
}
