#include <iostream>
#include "pic16a.h"
#include "pgmops.h"

std::ostream& operator<<(std::ostream &os, const PGMOperation &op)
{
    switch(op)
    {
    case PGMOperation::EnterProgMode:
        os << "EnterProgMode";
        break;
    case PGMOperation::ExitProgMode:
        os << "ExitProgMode";
        break;
    case PGMOperation::LoadConfig:
        os << "LoadConfig";
        break;
    case PGMOperation::MassErasePIC16A:
        os << "MassErasePIC16A";
        break;
    case PGMOperation::PointerIncrement:
        os << "PointerIncrement";
        break;
    case PGMOperation::ReadPage:
        os << "ReadPage";
        break;
    case PGMOperation::ResetPointer:
        os << "ResetPointer";
        break;                                        
    case PGMOperation::WritePage:
        os << "WritePage";
        break;                     
    }
    return os;
}

void writeCommand(std::shared_ptr<Serial> &serial, PGMOperation op)
{
    serial->write(op);
    serial->write(0x00);  // length
    auto resultOpt = serial->read();
    if (!resultOpt)
    {
        std::cerr << "No response to cmd " << op << "\n";
        return;
    }
    else
    {
        if (resultOpt.value() & 0x80)
        {
            std::cout << "CMD " << op << " ok\n";
        }
        else
        {
            std::cerr << "CMD " << op << " failed! reply=";
            std::cerr << std::hex << static_cast<uint16_t>(resultOpt.value()) << std::dec << "\n";
        }
    }
}

void PIC16A::resetPointer()
{
    writeCommand(m_serial, PGMOperation::ResetPointer);
}

void PIC16A::incPointer(uint8_t number)
{
    m_serial->write(PGMOperation::PointerIncrement);
    m_serial->write(0x01);  // length
    m_serial->write(number);
    
    auto resultOpt = m_serial->read();
    if ((!resultOpt) || (!(resultOpt.value() & 0x80)))
    {
        std::cerr << "CMD PointerIncrement failed - reply=";
        std::cerr << std::hex << static_cast<uint16_t>(resultOpt.value()) << std::dec << "\n";
    }
    else
    {
        std::cout << "CMD PointerIncrement ok!\n";
    }

}

void PIC16A::massErase()
{
    writeCommand(m_serial, PGMOperation::MassErasePIC16A);
}

void PIC16A::loadConfig()
{
    writeCommand(m_serial, PGMOperation::LoadConfig);
}

void PIC16A::writePage(const uint8_t *data, uint8_t numBytes)
{
    m_serial->write(PGMOperation::WritePage);  // write page?
    m_serial->write(numBytes+2);
    m_serial->write(numBytes);
    m_serial->write(1);     // speed, 1 = slow, 0 = fast ?
    
    m_serial->write(data, numBytes);

    auto resultOpt = m_serial->read();
    if ((!resultOpt) || (!(resultOpt.value() & 0x80)))
    {
        std::cout << "CMD WritePage failed\n";
    }
    else
    {
        std::cout << "CMD WritePage ok\n";
    }
}

std::vector<uint8_t> PIC16A::readPage(uint8_t num)
{
    m_serial->write(PGMOperation::ReadPage);
    m_serial->write(0x01);
    m_serial->write(num / 2);
    m_serial->read();

    std::vector<uint8_t> page(num, 0);
    for(uint8_t i=0; i<num; i++)
    {
        auto optByte = m_serial->read();
        if (optByte.has_value())
        {
            page.at(i) = optByte.value();
        }
        else
        {
            return std::vector<uint8_t>();
        }
    }
    return page;
}

std::optional<uint16_t> PIC16A::readDeviceId()
{
    return getConfig(6, 0xffff);
}

std::optional<uint16_t> PIC16A::getConfig(const uint8_t num, const uint16_t mask)
{
    resetPointer();
    loadConfig();
    incPointer(num);
    auto page = readPage(4);

    if (page.size() == 0)
    {
        return std::nullopt;
    }

    uint16_t deviceHi = page.at(3);
    uint16_t deviceLo = page.at(2);

    auto id = (deviceLo + (deviceHi << 8)) & mask; 
    return id;    
}

void PIC16A::writeConfig(const std::vector<uint8_t> &config)
{
    resetPointer();
    loadConfig();
    incPointer(7);

    writePage(&config[0], 2); // slow write
    writePage(&config[2], 2); // slow write

    // for B and D chips: writePage(/* word 0x8009 */, 2); // slow write
    // additionally for D chips: writePage(/* word 0x800A */, 2); // slow write
}

void PIC16A::enterProgMode() 
{
    writeCommand(m_serial, PGMOperation::EnterProgMode);
}

void PIC16A::exitProgMode()
{
    writeCommand(m_serial, PGMOperation::ExitProgMode);
}
