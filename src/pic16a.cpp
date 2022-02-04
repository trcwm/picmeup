#include <iostream>
#include <algorithm>
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

void writeCommand(std::shared_ptr<Serial> &serial, PGMOperation op, bool verbose)
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
            if (verbose) std::cout << "CMD " << op << " ok\n";
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
    writeCommand(m_serial, PGMOperation::ResetPointer, m_verbose);
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
        if (m_verbose) std::cout << "CMD PointerIncrement ok!\n";
    }

}

void PIC16A::massErase()
{
    writeCommand(m_serial, PGMOperation::MassErasePIC16A, m_verbose);
}

void PIC16A::loadConfig()
{
    writeCommand(m_serial, PGMOperation::LoadConfig, m_verbose);
}

bool PIC16A::writePage(const std::vector<uint8_t> &data)
{
    if ((data.size() % 2) == 1)
    {
        // data must be an even number of bytes!
        return false;
    }

    auto payloadBytes = data.size() + 2;
    m_serial->write(PGMOperation::WritePage);
    m_serial->write(payloadBytes);
    m_serial->write(data.size()/2); // number of words, not bytes.
    m_serial->write(1);             // speed, 1 = slow, 0 = fast ?
    
    m_serial->write(data);

    auto resultOpt = m_serial->read();
    if ((!resultOpt) || (!(resultOpt.value() & 0x80)))
    {
        std::cout << "CMD WritePage failed\n";
        return false;
    }
    
    if (m_verbose) std::cout << "CMD WritePage ok\n";
    return true;
}

std::vector<uint8_t> PIC16A::readPage(uint8_t numberOfWords)
{
    const auto numberOfBytes = numberOfWords*2;

    m_serial->write(PGMOperation::ReadPage);
    m_serial->write(0x01);
    m_serial->write(numberOfWords);
    auto resultOpt = m_serial->read();
    if (!resultOpt)
    {
        return std::vector<uint8_t>();
    }
    if (resultOpt.value() != (static_cast<uint8_t>(PGMOperation::ReadPage) | 0x80))
    {
        return std::vector<uint8_t>();
    }

    std::vector<uint8_t> page(numberOfBytes, 0);
    for(uint8_t i=0; i<numberOfBytes; i++)
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
    return getConfigWord(6);
}


std::optional<uint16_t> PIC16A::getConfigWord(uint32_t wordOffset)
{
    resetPointer();
    loadConfig();
    incPointer(wordOffset);
    auto page = readPage(1);

    if (page.size() != 2)
    {
        return std::nullopt;
    }

    uint16_t byteHi = page.at(1);
    uint16_t byteLo = page.at(0);

    auto word = (byteLo + (byteHi << 8)); 
    return word;    
}

void PIC16A::writeConfig(const std::vector<uint8_t> &config)
{
    resetPointer();
    loadConfig();
    incPointer(7);

    std::vector<uint8_t> word1;
    std::vector<uint8_t> word2;

    word1.push_back(config.at(0));
    word1.push_back(config.at(1));
    word2.push_back(config.at(2));
    word2.push_back(config.at(3));

    writePage(word1); // slow write
    writePage(word2); // slow write

    // for B and D chips: writePage(/* word 0x8009 */, 2); // slow write
    // additionally for D chips: writePage(/* word 0x800A */, 2); // slow write
}

void PIC16A::enterProgMode() 
{
    writeCommand(m_serial, PGMOperation::EnterProgMode, m_verbose);
}

void PIC16A::exitProgMode()
{
    writeCommand(m_serial, PGMOperation::ExitProgMode, m_verbose);
}
