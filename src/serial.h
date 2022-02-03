#pragma once


#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()

#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>
#include <memory>
#include <utility>
#include <optional>

#include "pgmops.h"

class Serial
{
public:
    Serial() = delete;

    virtual ~Serial();

    static std::shared_ptr<Serial> open(const std::string &portname, uint32_t baudrate)
    {
        int serialPortHandle = ::open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY);
        if (serialPortHandle < 0)
        {
            return nullptr;
        }

        fcntl(serialPortHandle, F_SETFL, 0);

        struct termios tty;
        memset (&tty, 0, sizeof(tty));

        if (tcgetattr(serialPortHandle, &tty) != 0) 
        {
            return nullptr;
        }
#if 0 
        tty.c_cflag &= ~PARENB;     // clear parity bit
        tty.c_cflag &= ~CSTOPB;     // one stop bit
        tty.c_cflag &= ~CSIZE;      // clear size bits
        tty.c_cflag |= CS8;         // 8 bits per byte 
        tty.c_cflag &= ~CRTSCTS;    // no flow control .. 
        tty.c_lflag &= ~ICANON;     // disable canonical mode
        tty.c_lflag &= ~ECHO;       // disable echo
        tty.c_lflag &= ~ECHOE;      // disable erasure
        tty.c_lflag &= ~ECHONL;     // disable new-line echo
        tty.c_lflag &= ~ISIG;       // disable interpretation of INTR, QUIT and SUSP
        tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
        tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes

        tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
        tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
        
        // tty.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT ON LINUX)
        // tty.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT ON LINUX)

        tty.c_cc[VTIME] = 10;    // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
        tty.c_cc[VMIN] = 0;        
#else
        tty.c_lflag  &=  ~(ICANON | ECHO | ECHOE | ISIG);
        tty.c_cflag |=  (CLOCAL | CREAD);
        tty.c_cflag &=  ~PARENB;
        tty.c_cflag &= ~CSTOPB;
        tty.c_cflag &=  ~CSIZE;
        tty.c_cflag |=  CS8;
        tty.c_oflag &=  ~OPOST;
        tty.c_iflag &=  ~INPCK;
        tty.c_iflag &=  ~ICRNL;		//do NOT translate CR to NL
        tty.c_iflag &=  ~(IXON | IXOFF | IXANY);
#endif
        int rate = B9600;
        if (baudrate == 57600)
        {
            rate = B57600;
        }

        cfsetispeed(&tty, rate);
        cfsetospeed(&tty, rate);

        if (tcsetattr(serialPortHandle, TCSANOW, &tty) != 0) 
        {
            //printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
            return nullptr;
        }

        tcflush(serialPortHandle, TCIOFLUSH);

        auto s = new Serial(serialPortHandle);

        return std::shared_ptr<Serial>(s);
    };

    bool waitForData(int timeOutMilliSeconds = 1000) const;
    bool hasData() const;
    std::optional<uint8_t> read();
    std::optional<std::vector<uint8_t> > read(size_t bytes);

    void write(PGMOperation op);
    void write(uint8_t c);
    void write(const char *data, size_t len);
    void write(const uint8_t *data, size_t len);
    void write(const std::vector<uint8_t> &data);

protected:
    Serial(int serialPortHandle) : m_serialPortHandle(serialPortHandle) {}

    int m_serialPortHandle = -1;
};
