#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

#include "utils.h"
#include "serial.h"
#include "pgmfactory.h"

#include "contrib/cxxopts.hpp"
#include "hexreader.h"

/*
# name   flash page  ID  mask family_type
# flash size and page size in bytes 1 WORD = 2 BYTES
# 
# PIC16 family
*/

struct DeviceInfo
{
    std::string deviceName;
    uint32_t    flashMemSize;   ///< in words
    uint32_t    flashPageSize;  ///< in words
    uint32_t    configSize;     ///< in words
    uint32_t    deviceId;
    uint32_t    deviceIdMask;
    std::string deviceFamily;
};

bool isEmptyMem(const std::vector<uint8_t> &mem, size_t start, size_t len)
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

bool isEmptyMem(const std::vector<uint8_t> &mem)
{
    auto len = mem.size();
    return isEmptyMem(mem, 0, len);
}


void showTargetDeviceInfo(const DeviceInfo &info)
{
    std::cout << "Target            : " << info.deviceName << "\n";
    std::cout << "  Flash mem size  : " << info.flashMemSize << " words\n";
    std::cout << "  Flash page size : " << info.flashPageSize << " words\n";
    std::cout << "  Device ID       : " << std::hex << std::uppercase << info.deviceId;
    std::cout << std::dec << std::nouppercase << "\n";
    std::cout << "  Device Family   : " << info.deviceFamily << "\n";
}

std::vector<DeviceInfo> readDeviceInfo(const std::string &filename)
{
    std::vector<DeviceInfo> info;
    std::ifstream deviceFile(filename);

    struct FamilyInfo
    {
        std::string name;
        uint32_t    configSize; // in words
    };

    const std::array<FamilyInfo, 12> validFamilies = 
    {
        {{"CF_P16F_A", 2},
        {"CF_P16F_B", 2},
        {"CF_P16F_C", 2},  
        {"CF_P16F_D", 2},
        {"CF_P18F_A", 16},
        {"CF_P18F_B",  8},
        {"CF_P18F_C", 16 /* basically CF_P18F_A */},
        {"CF_P18F_D", 16},
        {"CF_P18F_E", 16},
        {"CF_P18F_F", 12},
        {"CF_P18F_G", 10 /* basically CF_P18F_F */},
        {"CF_P18F_Q", 12}}
    };

    if (!deviceFile.is_open())
    {
        std::cerr << "Cannot open device file: " << filename << "\n";
        return info;
    }

    size_t lineNum = 0;
    while(!deviceFile.eof())
    {
        std::string line;
        std::getline(deviceFile, line);

        lineNum++;

        // skip empty lines and comments
        if (line.empty() || (line.at(0) == '#'))
        {
            continue;
        }

        auto tokens = Utils::tokenize(line, ' ');
        
        auto &dev = info.emplace_back();

        if (tokens.size() < 6)
        {
            std::cerr << "Error parsing device file - not enough columns on line " << lineNum << "\n";
            return std::vector<DeviceInfo>();
        }

        // read device name
        dev.deviceName = tokens.at(0);

        // read flash mem size (in words)
        auto flashMemOpt = Utils::intStrToint32(tokens.at(1));
        if (!flashMemOpt)
        {
            std::cerr << "Error parsing flash mem size on line " << lineNum << "\n";
            return std::vector<DeviceInfo>();
        }
        dev.flashMemSize = flashMemOpt.value() / 2;     // convert bytes to words

        // read flash mem size (in words)
        auto flashPageOpt = Utils::intStrToint32(tokens.at(2));
        if (!flashPageOpt)
        {
            std::cerr << "Error parsing flash page size on line " << lineNum << "\n";
            return std::vector<DeviceInfo>();
        }
        dev.flashPageSize = flashPageOpt.value() / 2;   // convert bytes to words

        // read device ID
        auto deviceIdOpt = Utils::hexStrToUint32(tokens.at(3));
        if (!deviceIdOpt)
        {
            std::cerr << "Error parsing device ID on line " << lineNum << "\n";
            return std::vector<DeviceInfo>();            
        }
        dev.deviceId = deviceIdOpt.value();

        // read device ID bit mask
        auto deviceIdMaskOpt = Utils::hexStrToUint32(tokens.at(4));
        if (!deviceIdMaskOpt)
        {
            std::cerr << "Error parsing device ID mask on line " << lineNum << "\n";
            return std::vector<DeviceInfo>();            
        }        
        dev.deviceIdMask = deviceIdMaskOpt.value();

        // read device family
        const auto deviceFamily = tokens.at(5);
        auto iter = std::find_if(validFamilies.begin(), validFamilies.end(), 
            [&deviceFamily](const FamilyInfo &device)
            {
                return device.name == deviceFamily;
            }
        );

        if (iter == validFamilies.end())
        {
            std::cerr << "Error parsing device family on line " << lineNum << "\n";
            return std::vector<DeviceInfo>();            
        }        

        dev.deviceFamily = iter->name;
        dev.configSize   = iter->configSize;
    }

    return info;
};

bool checkDevice(std::shared_ptr<IDeviceProgrammer> iface, const DeviceInfo &target)
{
    // read the device ID from the interface.
    // note: the programmer must be in programming mode to make this work

    auto idOpt = iface->readDeviceId();
    if (!idOpt)
    {
        std::cerr << "Could not read device ID!\n";
        return false;
    }

    const uint32_t IDcheck = idOpt.value() & target.deviceIdMask;

    bool IDok = (IDcheck == target.deviceId);
    if (!IDok)
    {
        std::cerr << "Device ID mismatch! Wanted " << Utils::toHex(target.deviceId) << " but got " << Utils::toHex(IDcheck) << "\n";
        return false;
    }

    return true;    
}

int main(int argc, char *argv[])
{
    std::string comName;
    std::string targetName;
    std::string uploadHexfileName;
    std::string downloadHexfileName;
    bool verify;
    bool upload;
    bool download;
    bool verbose;
    bool cpuErase;
    bool blankCheck = true;

    std::cout << "--== PICMEUP version 0.1a ==--\n\n";
    try
    {
        cxxopts::Options options(argv[0], "command line options");
        options
            .positional_help("[optional args]")
            .show_positional_help();

        options
            .set_width(70)
            .add_options()
            ("t,target","target cpu name", cxxopts::value<std::string>(targetName))
            ("p,port",  "serial port device name", cxxopts::value<std::string>(comName)->default_value("/dev/ttyUSB0"))
            ("i,input", "upload Intel HEX file", cxxopts::value<std::string>(uploadHexfileName))
            ("o,output","download Intel HEX file", cxxopts::value<std::string>(downloadHexfileName)->default_value("download.hex"))
            ("v,verify","Verify program", cxxopts::value<bool>(verify)->default_value("false"))
            ("verbose","Verbose output", cxxopts::value<bool>(verbose)->default_value("false"))
            ("d,download","Download program", cxxopts::value<bool>(download)->default_value("false"))
            ("e,erase","Erase before programming", cxxopts::value<bool>(cpuErase)->default_value("true"))
            ("u,upload","Upload program", cxxopts::value<bool>(upload)->default_value("false"))
            ("h, help", "Print help");

        auto result = options.parse(argc, argv);

        if (result.count("help"))
        {
            std::cout << options.help({"", "Group"}) << std::endl;
            return EXIT_FAILURE;
        }
    }
    catch(const cxxopts::OptionException& e)
    {
        std::cerr << "Error parsing options: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    if (targetName.empty())
    {
        std::cerr << "Please specify a target name\n";
        return EXIT_FAILURE;
    }

    // read the device file
    auto deviceInfo = readDeviceInfo("devices.dat");
    if (deviceInfo.empty())
    {
        return EXIT_FAILURE;
    }

#if 0
    if (verbose)
    {
        std::cout << "Read " << deviceInfo.size() << " devices from info file\n";
    }
#endif

    // find the target device in the device list
    targetName = Utils::toLower(targetName);
    auto iter = std::find_if(deviceInfo.begin(), deviceInfo.end(),
        [&targetName](const DeviceInfo &device)
        {
            return device.deviceName == targetName;
        }
    );

    if (iter == deviceInfo.end())
    {
        std::cerr << "Cannot find target device " << targetName << " in device list\n";
        return EXIT_FAILURE;
    }

    auto targetDeviceInfo = *iter;

    showTargetDeviceInfo(targetDeviceInfo);

    std::cout << "\n";

    auto serial = Serial::open(comName.c_str(), 57600);
    if (serial)
    {
        std::cout << "Serial port opened!\n";
    }
    else
    {
        std::cout << "Error opening serial port " << comName << " !\n";
        return EXIT_FAILURE;
    }

    // Arduino resets when the UART connects.
    // and we have to wait a bit before the uC comes online.
    // This is why we can't have nice things.
    if (verbose)
    {
        std::cout << "Waiting for the programmer to come online..\n";
    }

    sleep(2);

    // FIXME: use factory to create the correct programmer
    // for the device family
    auto pgm = ProgrammerFactory::create(targetDeviceInfo.deviceFamily, serial);

    if (!pgm)
    {
        std::cerr << "Device family " << targetDeviceInfo.deviceFamily << " is not supported\n";
        return EXIT_FAILURE;
    }

    pgm->enterProgMode();

    if (!checkDevice(pgm, targetDeviceInfo))
    {
        pgm->exitProgMode();
        return EXIT_FAILURE;
    }
    else
    {
        std::cout << "Device ID ok!\n";
    }

    std::vector<uint8_t> flashMem(targetDeviceInfo.flashMemSize*2, 0xFF);
    std::vector<uint8_t> configMem(targetDeviceInfo.configSize*2,  0xFF);

    // FIXME: PIC16 has 14-bit word, so we need
    //        to make sure the top 2 bits are 0
    //        however, other PICs might have
    //        a wide pgm word..
    for(size_t idx=1; idx<flashMem.size(); idx+=2)
    {
        flashMem.at(idx) &= 0x3F;
    }

    // read the input hex file if there is one
    if (!uploadHexfileName.empty())
    {
        if (verbose)
        {
            std::cout << "Reading IHEX file " << uploadHexfileName << "\n";
        }

        HexReader::read(uploadHexfileName, flashMem, configMem);
    }

    bool isBlank = true;
    if (blankCheck)
    {
        std::cout << "Blank check\n";
        pgm->resetPointer();        
        for(size_t address=0; address < targetDeviceInfo.flashMemSize; address += targetDeviceInfo.flashPageSize)
        {
            auto page = pgm->readPage(targetDeviceInfo.flashPageSize);
            if (page.size() == 0)
            {
                std::cout << "Could not read page\n";
                pgm->exitProgMode();
                return EXIT_FAILURE;
            }
            if (!isEmptyMem(page))
            {
                std::cout << "### Warning: uC is not blank! ###\n";
                isBlank = false;
                break;
            }
        }
    }

    if (cpuErase & !isBlank)
    {
        std::cout << "Erasing flash memory\n";
        pgm->resetPointer();
        pgm->massErase();
        sleep(1);
    }

    if (upload)
    {
        std::cout << "Programming flash..\n";
        pgm->resetPointer();

        for(size_t address=0; address < targetDeviceInfo.flashMemSize; address += targetDeviceInfo.flashPageSize)
        {   
            std::vector<uint8_t> memChunk(flashMem.begin() + address*2, flashMem.begin() + (address+targetDeviceInfo.flashPageSize)*2);
            
            if (isEmptyMem(memChunk))
            {
                pgm->incPointer(targetDeviceInfo.flashPageSize);
                std::cout << "." << std::flush;
            }
            else
            {
                pgm->writePage(memChunk);
                std::cout << "#" << std::flush;
            }            
        }

        std::cout << "\nProgramming config..\n";
        pgm->writeConfig(configMem);
    }

    if (verify)
    {
        std::cout << "Verifying..\n";
        pgm->resetPointer();
        for(size_t address=0; address < targetDeviceInfo.flashMemSize; address += targetDeviceInfo.flashPageSize)
        {
            auto page = pgm->readPage(targetDeviceInfo.flashPageSize);
            size_t offset = 0;
            for(auto byte : page)
            {
                if (byte != flashMem.at(address*2 + offset))
                {
                    std::cout << "Verification failure at byte address 0x" << Utils::toHex(address*2 + offset) << "\n";

                    // dump the page
                    size_t o = 0;
                    for(auto byte2 : page)
                    {
                        if (byte2 != flashMem.at(address*2 + o))
                        {
                            std::cout << "  " << Utils::toHex(address*2 + o) << "  have " << Utils::toHex(byte2,2);
                            std::cout << " - want " << Utils::toHex(address*2+o,2) << "\n";
                        }                        
                        o++;
                    }

                    pgm->exitProgMode();
                    return EXIT_FAILURE;
                }
                offset++;
            }
        }
        std::cout << "Verify ok!\n";
    }    
    
    pgm->exitProgMode();

    std::cout << "Done.\n";

    return EXIT_SUCCESS;
}

