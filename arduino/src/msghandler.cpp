// 
#include <avr/io.h>
#include "msghandler.h"
#include "../../src/pgmops.h"

void MessageHandler::init()
{
    DDRB |= (1<<5); // enable LED output pin
    m_uart.init();
    m_isp.init();
    ledOff();
}

void MessageHandler::ledOn()
{
    PORTB |= (1<<5);
}

void MessageHandler::ledOff()
{
    PORTB &= ~(1<<5);
}

bool MessageHandler::loop()
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

// FIXME: we really should change this to COBS encoding
void MessageHandler::tick()
{
    while(!loop()) {};

    if (m_bufferIdx <= 0)
    {
        // some kind of error!
        return;
    }

    ledOn();

    auto cmdId = m_buffer[0];
    switch(static_cast<PGMOperation>(cmdId))
    {
    case PGMOperation::EnterProgMode:
        m_isp.enterProgMode();
        m_uart.write(0x81);
        break;
    case PGMOperation::ExitProgMode:
        m_isp.exitProgMode();
        m_uart.write(0x82);
        break;        
    case PGMOperation::ResetPointer:
        m_isp.resetPointer();
        m_uart.write(0x83);
        break;
    case PGMOperation::LoadConfig:
        m_isp.loadConfig(0);
        m_uart.write(0x84);
        break;
    case PGMOperation::PointerIncrement:
        {
            if (m_bufferIdx != 3)
            {
                m_uart.write(0x05);
                // error!
                return;
            }
            for(uint8_t i=0; i<m_buffer[2]; i++)
            {
                m_isp.incrementPointer();
            }
            m_uart.write(0x85);
        }
        break;
    case PGMOperation::ReadPage:
        /*
            Buffer layout:
            0x00: operation ID
            0x01: total bytes of payload = 1
            0x02: number of words to read
        */    
        if (m_bufferIdx != 3)
        {
            m_uart.write(0x06);
            // error!
            return;
        }    
        
        m_uart.write(0x86);
        {
            const auto words = m_buffer[2];
            m_isp.readPgm(m_isp.m_flashBuffer, words);

            for(uint8_t i=0; i<words; i++)
            {
                m_uart.write(m_isp.m_flashBuffer[i] & 0xFF);
                m_uart.write(m_isp.m_flashBuffer[i] >> 8);
            }
        }
        break;
    case PGMOperation::MassErasePIC16A:
        m_isp.massErase();
        m_uart.write(0x87);
        break;
    case PGMOperation::WritePage:
        {
            /*
                Buffer layout:
                0x00: operation ID
                0x01: total bytes of payload
                0x02: number of words to program
                0x03: speed 1 = slow, 0 = fast
                0x04: first byte (MSB) of first word
                0x05: second byte of first word etc..
            */
            const uint8_t words = m_buffer[2];
            uint8_t *ptr = m_buffer+4;
            for (uint16_t i=0; i<words; i++)
            {
                m_isp.m_flashBuffer[i] = static_cast<uint16_t>(ptr[(2*i)+1]<<8) + static_cast<uint16_t>(ptr[(2*i)]);
            }

            m_isp.writePgm(m_isp.m_flashBuffer, words);
        }
        m_uart.write(0x88);
        break;
    case PGMOperation::EnterProgModeWithPGM:
        m_isp.enterProgModeWithPGMPin();
        m_uart.write(0x90);
        break;
    case PGMOperation::ExitProgModeWithPGM:
        m_isp.exitProgModeWithPGMPin();
        m_uart.write(0x91);
        break;
    default:
        m_uart.write(0x00);
        break;
    }

    ledOff();
}
