// SPDX-License-Identifier: GPL-3.0-only
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
    WritePage           = 0x08,

    EnterProgModeWithPGM= 0x10,     // classic devices such as PIC16F87X
    ExitProgModeWithPGM = 0x11,     // classic devices such as PIC16F87X
    LoadConfigWithArg   = 0x12,     // 1 word argument
    BulkEraseSetup1     = 0x13,
    BulkEraseSetup2     = 0x14,
    BeginEraseProgramming = 0x15
};
