#include <iostream>
#include "serial.h"
#include "pic16a.h"

int main(int argc, char *argv[])
{
    auto serial = Serial::open("/dev/ttyUSB0", 57600);

    if (serial)
    {
        std::cout << "Serial port opened!\n";
    }
    else
    {
        std::cout << "Error opening serial port!\n";
        return EXIT_FAILURE;
    }

    sleep(2);

    PIC16A pgm(serial);

    pgm.enterProgMode();

    auto id = pgm.readDeviceId();
    if (id)
    {
        std::cout << "Device ID is: " << std::hex << id.value() << std::dec << "\n";
    }
    else
    {
        std::cerr << "Could not read the device ID!\n";
    }

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

    pgm.exitProgMode();

    return EXIT_SUCCESS;
}
