#ifndef CONFIGURE_SERIAL_H
#define CONFIGURE_SERIAL_H

#include <termios.h>

int configure_serial(int *serial_port, struct termios *tty);

#endif // CONFIGURE_SERIAL_H