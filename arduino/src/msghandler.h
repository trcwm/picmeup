#pragma once
#include "uart.h"

class MessageHandler
{
public:
    void init();

    bool tick();

protected:
    constexpr static uint8_t c_bufsize = 100;

    enum class RxState : uint8_t
    {
        START = 0,
        PAYLOADSIZE,
        PAYLOAD
    };

    UART m_uart;

    RxState m_rxState = RxState::START;

    uint8_t m_bytesToReceive = 0;
    uint8_t m_bufferIdx = 0;
    uint8_t m_buffer[c_bufsize];
};
