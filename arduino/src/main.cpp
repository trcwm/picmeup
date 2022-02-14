// SPDX-License-Identifier: GPL-3.0-only
// Copyright N.A. Moseley 2022

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include "msghandler.h"

int main()
{
    MessageHandler handler;
    handler.init();

    while (1)
    {
        handler.tick();
    }
    return 0;
}
