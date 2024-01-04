#include <cerrno>
#include <iostream>
#include <termio.h>
#include <vector>
#include <unistd.h>

#include "configure_serial.h"
#include "utils.h"
#include "pm1006.h"

int main()
{
    int serial_port;
    struct termios tty;

    int status = configure_serial(&serial_port, &tty);
    if (status == 1)
        return 1;

    std::vector<ByteStruct> byte_buffer;

    try
    {
        for (;;)
        {
            get_pm1006_message(serial_port, byte_buffer);
            int pm25_reading = calculate_pm25(byte_buffer);
            auto payload = mock_payload(pm25_reading);
            send_payload(serial_port, payload);
            byte_buffer.clear();
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception caught: " << e.what() << "\n";
    }
    catch (...)
    {
        std::cerr << "Unknown :exception caught"
                  << "\n";
    }

    close(serial_port);

    return 0;
}
