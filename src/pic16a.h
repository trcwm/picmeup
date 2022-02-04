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
    
    bool                    writePage(const std::vector<uint8_t> &data) override;
    std::vector<uint8_t>    readPage(uint8_t num) override;
    std::optional<uint16_t> readDeviceId() override;

    std::optional<uint16_t> getConfigWord(uint32_t wordOffset) override;
    
    void loadConfig() override;
    void writeConfig(const std::vector<uint8_t> &config) override;

    void enterProgMode() override;
    void exitProgMode() override;
};
