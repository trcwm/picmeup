#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <streambuf>

#include "utils.h"
#include "serial.h"
#include "pgmfactory.h"

#include "contrib/cxxopts.hpp"
#include "hexreader.h"

extern "C" char g_devices[];
extern "C" size_t g_devices_size;


void showTargetDeviceInfo(const DeviceInfo &info)
{
    std::cout << "Target            : " << info.deviceName << "\n";
    std::cout << "  Flash mem size  : " << info.flashMemSize << " words\n";
    std::cout << "  Flash page size : " << info.flashPageSize << " words\n";
    std::cout << "  Device ID       : " << std::hex << std::uppercase << info.deviceId;
    std::cout << std::dec << std::nouppercase << "\n";
    std::cout << "  Device Family   : " << info.deviceFamily << "\n";
}

struct MemoryStream : std::streambuf
{
    MemoryStream(char *begin, char *end)
    {
        this->setg(begin, begin, end);
    }
};

std::vector<DeviceInfo> readDeviceInfo()
{
    std::vector<DeviceInfo> info;
    //std::ifstream deviceFile(filename);

    MemoryStream memstream(g_devices, g_devices + g_devices_size);
    std::istream deviceFile(&memstream);

    struct FamilyInfo
    {
        std::string name;
        uint32_t    configSize; // in words
    };

    const std::array<FamilyInfo, 12> validFamilies = 
    {
        {{"CF_P16F_A", 2},
        {"CF_P16F_B",  3},
        {"CF_P16F_C",  2},  
        {"CF_P16F_D",  2},
        {"CF_P18F_A", 16},
        {"CF_P18F_B",  8},
        {"CF_P18F_C", 16 /* basically CF_P18F_A */},
        {"CF_P18F_D", 16},
        {"CF_P18F_E", 16},
        {"CF_P18F_F", 12},
        {"CF_P18F_G", 10 /* basically CF_P18F_F */},
        {"CF_P18F_Q", 12}}
    };

    //if (!deviceFile.is_good())
    //{
    //    std::cerr << "Cannot open device file: " << filename << "\n";
    //    return info;
    //}

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
    bool showConfig;
    bool showDevices;
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
            ("showconfig","Print the configuration bits", cxxopts::value<bool>(showConfig)->default_value("false"))
            ("showdevices","Print supported devices", cxxopts::value<bool>(showDevices)->default_value("false"))
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
    //auto deviceInfo = readDeviceInfo("devices.dat");
    auto deviceInfo = readDeviceInfo();
    if (deviceInfo.empty())
    {
        return EXIT_FAILURE;
    }

    if (showDevices)
    {
        std::cout << "Supported devices:\n";
        for(auto const device : deviceInfo)
        {
            if (device.deviceFamily == "CF_P16F_A")
            {
                std::cout << "  " << device.deviceName << "\n";
            }
        }
    }

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
        isBlank = pgm->isDeviceBlank(targetDeviceInfo);
        if (isBlank)
        {
            std::cout << "Device is blank\n";
        }
    }

    if (cpuErase && !isBlank)
    {
        std::cout << "Erasing flash memory\n";
        pgm->massErase();
        sleep(1);
    }

    if (upload)
    {
        std::cout << "Programming flash..\n";
        pgm->uploadFlash(targetDeviceInfo, flashMem);
        pgm->uploadConfig(targetDeviceInfo, configMem);
        std::cout << "\n";
    }

    if (verify)
    {        
        std::cout << "Verifying.. ";
        auto flashContents = pgm->downloadFlash(targetDeviceInfo);

        for(size_t address = 0; address < targetDeviceInfo.flashMemSize*2; address+=2)
        {
            bool check1 = flashContents.at(address) == flashMem.at(address);
            bool check2 = flashContents.at(address+1) == flashMem.at(address+1);
            if ((check1 && check2) == false)
            {
                std::cerr << "Flash memory mismatch at address " << (address/2) << "\n";
                std::cerr << "  wanted: " << Utils::toHex(flashMem.at(address+1),2);
                std::cerr << Utils::toHex(flashMem.at(address),2) << "  but got: ";
                std::cerr << Utils::toHex(flashContents.at(address+1),2);
                std::cerr << Utils::toHex(flashContents.at(address),2) << "\n";
                pgm->exitProgMode();
                return EXIT_FAILURE;
            }
        }
        std::cout << "Ok!\n";
    }    

    if (showConfig)
    {
        std::cout << "Configuration words:\n";
        std::cout << "  ";

        auto configBytes = pgm->downloadConfig(targetDeviceInfo);

        if (configBytes.empty())
        {
            std::cerr << "Could not read configuration bytes!\n";
            pgm->exitProgMode();
            return EXIT_FAILURE;
        }

        for(size_t address=0; address < configBytes.size(); address += 2)
        {
            const auto byte1 = configBytes.at(address+1);
            const auto byte2 = configBytes.at(address);
            std::cout << " 0x" << Utils::toHex(byte1,2) << Utils::toHex(byte2,2) << " ";
        }
        std::cout << "\n";
    }

    pgm->exitProgMode();

    std::cout << "Done.\n";

    return EXIT_SUCCESS;
}

