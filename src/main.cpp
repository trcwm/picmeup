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

    return EXIT_SUCCESS;
}
