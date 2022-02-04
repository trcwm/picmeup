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
    while(len > 0)
    {
        if (mem.at(start++) != 0xFF)
        {
            return false;
        }
        len--;
    }
    return true;
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
        dev.flashMemSize = flashMemOpt.value();

        // read flash mem size (in words)
        auto flashPageOpt = Utils::intStrToint32(tokens.at(2));
        if (!flashPageOpt)
        {
            std::cerr << "Error parsing flash page size on line " << lineNum << "\n";
            return std::vector<DeviceInfo>();
        }
        dev.flashPageSize = flashPageOpt.value();

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

    // read the input hex file if there is one
    if (!uploadHexfileName.empty())
    {
        if (verbose)
        {
            std::cout << "Reading IHEX file " << uploadHexfileName << "\n";
        }

        HexReader::read(uploadHexfileName, flashMem, configMem);
    }

    if (cpuErase)
    {
        std::cout << "Erasing flash memory\n";
        pgm->resetPointer();
        pgm->massErase();
    }

    if (upload)
    {
        std::cout << "Programming flash..\n";
        pgm->resetPointer();

        for(size_t address=0; address < targetDeviceInfo.flashMemSize; address += targetDeviceInfo.flashPageSize)
        {
            pgm->writePage(&flashMem[address*2], targetDeviceInfo.flashPageSize*2);
            std::cout << "#" << std::flush;
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
                //printf("%02X", byte);
                //if ((offset % 16) == 15)
                //{
                //    printf("\n");
                //}
#if 1   
                if (byte != flashMem.at(address*2 + offset))
                {
                    std::cout << "Verification failure at address " << std::hex << std::uppercase;
                    std::cout << address + offset << std::nouppercase << std::dec << "\n";

                    // dump the page
                    size_t o = 0;
                    for(auto byte2 : page)
                    {
                        printf("have %02X - want %02X\n", byte2, flashMem[o++]);
                    }

                    pgm->exitProgMode();
                    return EXIT_FAILURE;
                }
#endif                
                offset++;
            }
        }
        std::cout << "Verify ok!\n";
    }    
    

#if 0
    std::cout << "Reading from address 0\n";
    pgm.resetPointer();
    auto page = pgm.readPage(64);

    if (page.empty())
    {
        std::cerr << "Could not read page!";
    }
    else
    {
        size_t count = 0;
        for(auto c : page)
        {            
            fprintf(stdout, "%02X ", c);
            count++;
            if (count == 16)
            {
                fprintf(stdout, "\n");
                count = 0;
            }
        }
    }

    std::cout << "Writing to address 0\n";
    pgm.resetPointer();    
    std::vector<uint8_t> data;
    data.resize(64,0);    
    pgm.writePage(&data[0], data.size());

    std::cout << "Reading from address 0\n";
    pgm.resetPointer();
    page = pgm.readPage(64);

    if (page.empty())
    {
        std::cerr << "Could not read page!";
    }
    else
    {
        size_t count = 0;
        for(auto c : page)
        {            
            fprintf(stdout, "%02X ", c);
            count++;
            if (count == 16)
            {
                fprintf(stdout, "\n");
                count = 0;
            }
        }
    }

    pgm.massErase();
#endif

    pgm->exitProgMode();

    std::cout << "Done.\n";

    return EXIT_SUCCESS;
}

