#pragma once
#include "serial.h"

class IDeviceProgrammer
{
public:
    IDeviceProgrammer(std::shared_ptr<Serial> serial) : m_serial(serial) {}

    virtual void massErase() = 0;
    
    virtual void resetPointer() = 0;
    virtual void incPointer(uint8_t number) = 0;
    
    virtual void                    writePage(const uint8_t *data, uint8_t numBytes) = 0;
    virtual std::vector<uint8_t>    readPage(uint8_t words) = 0;
    virtual std::optional<uint16_t> readDeviceId() = 0;

    virtual std::optional<uint16_t> getConfig(const uint8_t num, const uint16_t mask)  = 0;
    virtual void loadConfig() = 0;
    virtual void writeConfig(const std::vector<uint8_t> &config) = 0;

    virtual void enterProgMode() = 0;
    virtual void exitProgMode() = 0;

protected:
    bool m_verbose = false;
    std::shared_ptr<Serial> m_serial;
};


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
