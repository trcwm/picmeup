// SPDX-License-Identifier: GPL-3.0-only
// Copyright N.A. Moseley 2022

#include "pic16pgm_a.h"

void PIC16PGM_A::massErase()
{
}

void PIC16PGM_A::enterProgMode() 
{
    writeCommand(PGMOperation::EnterProgModeWithPGM, m_verbose);
}

void PIC16PGM_A::exitProgMode()
{
    writeCommand(PGMOperation::ExitProgModeWithPGM, m_verbose);
}

