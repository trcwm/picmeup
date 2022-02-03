#include "msghandler.h"

void MessageHandler::init()
{
    m_uart.init();
}

// FIXME: we really should change this to COBS encoding
bool MessageHandler::tick()
{
    while(m_uart.hasData())
    {
        switch(m_rxState)
        {
        case RxState::START:
            m_bufferIdx = 0;
            m_buffer[m_bufferIdx++] = m_uart.read();
            m_rxState = RxState::PAYLOADSIZE;
            break;
        case RxState::PAYLOADSIZE:
            m_bytesToReceive = m_uart.read();
            if (m_bytesToReceive >= c_bufsize)
            {
                m_bytesToReceive = c_bufsize-1;
            }

            m_buffer[m_bufferIdx++] = m_bytesToReceive;
            if (m_bytesToReceive == 0)
            {
                m_rxState = RxState::START;
                return true;
            }
            m_rxState = RxState::PAYLOAD;
            break;
        case RxState::PAYLOAD:
            m_buffer[m_bufferIdx++] = m_uart.read();
            m_bytesToReceive--;
            if (m_bytesToReceive==0)
            {
                m_rxState = RxState::START;
                return true;
            }
            break;
        }
    }
    return false;
}
