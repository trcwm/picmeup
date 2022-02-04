#pragma once

enum class PGMOperation : uint8_t
{
    EnterProgMode       = 0x01,
    ExitProgMode        = 0x02,
    ResetPointer        = 0x03,
    LoadConfig          = 0x04,
    PointerIncrement    = 0x05,    
    ReadPage            = 0x06,
    MassErasePIC16A     = 0x07,
    WritePage           = 0x08  ///< 
};
