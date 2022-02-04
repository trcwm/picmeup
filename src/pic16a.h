#pragma once
#include "serial.h"
#include "devicepgminterface.h"
class PIC16A : public IDeviceProgrammer
{
public:
    PIC16A(std::shared_ptr<Serial> serial) : IDeviceProgrammer(serial) {}

    void massErase() override;

    void resetPointer() override;
    void incPointer(uint8_t number) override;
    
    void                    writePage(const uint8_t *data, uint8_t numBytes) override;
    std::vector<uint8_t>    readPage(uint8_t num) override;
    std::optional<uint16_t> readDeviceId() override;

    std::optional<uint16_t> getConfig(const uint8_t num, const uint16_t mask) override;
    
    /** sets the address pointer to 0x8000 and loads the first word */
    void loadConfig() override;
    
    void writeConfig(const std::vector<uint8_t> &config) override;

    void enterProgMode() override;
    void exitProgMode() override;
};
