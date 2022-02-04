#pragma once

#include <memory>
#include <vector>
#include <cstdint>
#include "serial.h"

class IDeviceProgrammer
{
public:
    IDeviceProgrammer(std::shared_ptr<Serial> serial) : m_serial(serial) {}

    virtual void massErase() = 0;
    
    virtual void resetPointer() = 0;
    virtual void incPointer(uint8_t number) = 0;
    
    /** Write/flash a page of data. The data vector must have an even number of bytes.
     *  Ordered as they appear in the IHEX file
    */
    virtual bool                    writePage(const std::vector<uint8_t> &data) = 0;

    /** Read a page of data. The number of output bytes is twice the number of words */
    virtual std::vector<uint8_t>    readPage(uint8_t words) = 0;

    /** Read the device ID. The ID also contains silicon revision bits that need to be masked out */
    virtual std::optional<uint16_t> readDeviceId() = 0;

    /** Read the a configuration word at the specified offset (offset = 0 is first word) */
    virtual std::optional<uint16_t> getConfigWord(uint32_t wordOffset) = 0;
    
    /** Sets the read address pointer to the config space and prepares the uC for reading */
    virtual void loadConfig() = 0;

    /** Write the configuration bits. The number of bytes required depends on the uC type */
    virtual void writeConfig(const std::vector<uint8_t> &config) = 0;

    /** Enter LV programming mode */
    virtual void enterProgMode() = 0;

    /** Leave LV programming mode */
    virtual void exitProgMode() = 0;

protected:
    bool m_verbose = false;
    std::shared_ptr<Serial> m_serial;
};

