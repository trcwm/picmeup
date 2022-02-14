// SPDX-License-Identifier: GPL-3.0-only
// Copyright N.A. Moseley 2022

#pragma once
#include "serial.h"
#include "devicepgminterface.h"
class PIC16A : public IDeviceProgrammer
{
public:
    PIC16A(std::shared_ptr<Serial> serial) : IDeviceProgrammer(serial) {}

    void massErase() override;
    
    std::optional<uint16_t> readDeviceId() override;

    /** Upload configuration bits */
    bool uploadConfig(const DeviceInfo &info, const std::vector<uint8_t> &config) override;

    /** Download configuration bits */
    std::vector<uint8_t> downloadConfig(const DeviceInfo &info) override;

    /** Upload to flash */
    bool uploadFlash(const DeviceInfo &info, const std::vector<uint8_t> &memory) override;

    /** Download from flash */
    std::vector<uint8_t> downloadFlash(const DeviceInfo &info) override;

    /** check if the device is blank */
    bool isDeviceBlank(const DeviceInfo &info) override;

    void enterProgMode() override;
    void exitProgMode() override;

protected:
    void resetPointer();
    void incPointer(uint8_t number);

    bool                    writePage(const std::vector<uint8_t> &data);
    std::vector<uint8_t>    readPage(uint8_t num);
    
    void loadConfig();

    void writeCommand(PGMOperation op, bool verbose);
};
