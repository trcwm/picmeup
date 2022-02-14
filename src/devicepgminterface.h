// SPDX-License-Identifier: GPL-3.0-only
// Copyright N.A. Moseley 2022

#pragma once

#include <memory>
#include <vector>
#include <cstdint>
#include "serial.h"

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


class IDeviceProgrammer
{
public:
    IDeviceProgrammer(std::shared_ptr<Serial> serial) : m_serial(serial) {}

    virtual void massErase() = 0;

    /** Read the device ID. The ID also contains silicon revision bits that need to be masked out */
    virtual std::optional<uint16_t> readDeviceId() = 0;

    /** Upload to flash */
    virtual bool uploadFlash(const DeviceInfo &info, const std::vector<uint8_t> &memory) = 0;

    /** Download from flash */
    virtual std::vector<uint8_t> downloadFlash(const DeviceInfo &info) = 0;

    /** Upload configuration bits */
    virtual bool uploadConfig(const DeviceInfo &info, const std::vector<uint8_t> &config) = 0;

    /** Download configuration bits */
    virtual std::vector<uint8_t> downloadConfig(const DeviceInfo &info) = 0;

    /** check the device is blank. returns true if device is blank */
    virtual bool isDeviceBlank(const DeviceInfo &info) = 0;

    /** Enter LV programming mode */
    virtual void enterProgMode() = 0;

    /** Leave LV programming mode */
    virtual void exitProgMode() = 0;

protected:
    bool m_verbose = false;
    std::shared_ptr<Serial> m_serial;
};
