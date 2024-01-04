#include "configure_serial.h"
#include <iostream>
#include <cerrno>
#include <cstring>
#include <termio.h>
#include <fcntl.h>
#include <unistd.h>

#define DEVICE "/dev/ttyUSB0"

int configure_serial(int *serial_port, struct termios *tty)
{
    *serial_port = open(DEVICE, O_RDWR);

    if (*serial_port < 0)
    {
        std::cout << "Error " << errno << " from open: " << strerror(errno) << "\n";
        close(*serial_port);
        return 1;
    }

    // set defaults
    if (tcgetattr(*serial_port, tty) != 0)
    {
        std::cout << "Error " << errno << " from tcgetattr: " << strerror(errno) << "\n";
        close(*serial_port);
        return 1;
    }

    // configure custom settings to for overwriting those defaults

    // control modes c_cflags
    tty->c_cflag &= ~PARENB;         // disable parity bit
    tty->c_cflag &= ~CSTOPB;         // disable stop bits field -> 1 stop bits
    tty->c_cflag |= CS8;             // 8 data bits per byte
    tty->c_cflag &= ~CRTSCTS;        // no RTS/CTS hardware flow control, no signaling for when data ready to be sent/received.
    tty->c_cflag &= ~CREAD | CLOCAL; // no READ and ignore ctrl lines (CLOCAL = 1)

    // local modes c_lflag
    tty->c_lflag &= ~ICANON; // disable canonical mode. (dont process input line-by-line, when new line char received)

    tty->c_lflag &= ~ECHO;   // disable echo
    tty->c_lflag &= ~ECHOE;  // disable erasure
    tty->c_lflag &= ~ECHONL; // disable new-line echo

    tty->c_lflag &= ~ISIG; // disable signal chars; interpretation of INTR, QUIT and SUSP

    // input modes c_iflag
    tty->c_iflag &= ~(IXON | IXOFF | IXANY);                                      // disable s/w flow control
    tty->c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL); // disable any spceial handling of received bytes

    // output modes c_oflag
    tty->c_oflag &= ~OPOST; // prevent special interpretation of output bytes (e.g. newline chars)
    tty->c_oflag &= ~ONLCR; // prevent conversion of newline to carriage return/line feed

    // vtime and vmin (wait time, min bytes)?
    tty->c_cc[VTIME] = 0;
    tty->c_cc[VMIN] = 1;

    // set in/out baudrates
    cfsetispeed(tty, B9600);
    cfsetospeed(tty, B9600);

    // save custom tty settings, check errors
    if (tcsetattr(*serial_port, TCSANOW, tty) != 0)
    {
        std::cout << "Error " << errno << " from tcsetattr: " << strerror(errno) << "\n";
        close(*serial_port);
        return 1;
    }

    return 0;
}