#include "serial.h"
#include <sys/poll.h>

Serial::~Serial()
{
    if (m_serialPortHandle >= 0)
    {
        ::close(m_serialPortHandle);
    }
}

std::optional<uint8_t> Serial::read()
{
    uint8_t v;
    int bytes = ::read(m_serialPortHandle, &v, 1);
    if (bytes != 1)
    {
        return std::nullopt;
    }
    return v;
}

std::optional<std::vector<uint8_t> > Serial::read(size_t bytes)
{
    std::vector<uint8_t> buffer(bytes,0);
    int rdbytes = ::read(m_serialPortHandle, &buffer[0], buffer.size());
    if (rdbytes != buffer.size())
    {
        return std::nullopt;
    }

    return buffer;
}

bool Serial::waitForData(int timeOutMilliSeconds) const
{
    struct pollfd fds[1];
    fds[0].fd = m_serialPortHandle;
    fds[0].events = POLLIN ;
    
    const int numberOfDescriptors = 1;
    int result = poll(fds, numberOfDescriptors, timeOutMilliSeconds);
    if (result < 0)
    {
        // poll error
        return false;
    }

    if (fds[0].revents & POLLIN)
    {
        return true;
    }

    return false;
}

bool Serial::hasData() const
{
    struct pollfd fds[1];
    fds[0].fd = m_serialPortHandle;
    fds[0].events = POLLIN ;
    
    const int timeOutMilliSeconds = 1;
    const int numberOfDescriptors = 1;
    int result = poll(fds, numberOfDescriptors, timeOutMilliSeconds);
    if (result < 0)
    {
        // poll error
        return false;
    }

    if (fds[0].revents & POLLIN)
    {
        return true;
    }

    return false;
}

void Serial::write(PGMOperation op)
{
    uint8_t opcode = static_cast<uint8_t>(op);
    ::write(m_serialPortHandle, &opcode, 1);
}

void Serial::write(uint8_t c)
{
    ::write(m_serialPortHandle, &c, 1);
}

void Serial::write(const char *data, size_t len)
{
    ::write(m_serialPortHandle, data, len);
}

void Serial::write(const uint8_t *data, size_t len)
{
    ::write(m_serialPortHandle, data, len);
}

void Serial::write(const std::vector<uint8_t> &data)
{
    ::write(m_serialPortHandle, &data[0], data.size());
}