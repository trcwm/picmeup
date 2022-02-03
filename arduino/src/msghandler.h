#pragma once
#include "uart.h"
#include "isp.h"

class MessageHandler
{
public:
    void init();

    void tick();

    uint8_t getMessageLen() const
    {
        return m_bufferIdx;
    }

protected:
    bool loop();

    void ledOn();
    void ledOff();

    constexpr static uint16_t c_bufsize = 280;

    enum class RxState : uint8_t
    {
        START = 0,
        PAYLOADSIZE,
        PAYLOAD
    };

    ISP  m_isp;
    UART m_uart;

    RxState m_rxState = RxState::START;

    uint16_t m_bytesToReceive = 0;
    uint16_t m_bufferIdx = 0;
    uint8_t  m_buffer[c_bufsize];
};
