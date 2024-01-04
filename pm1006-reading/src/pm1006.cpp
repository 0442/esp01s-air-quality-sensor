#include "pm1006.h"
#include <vector>
#include <string>
#include <iostream>
#include <unistd.h>
#include <cstring>

#include "utils.h"


#define PM1006_MESSAGE_LEN 20

const std::vector<std::string> payload_prefix = {"16", "11", "0b"};

std::vector<std::string> mock_payload(int pm25_reading)
{
    // Creates a mock payload mimicing the pm1006,
    // which allows controlling the VINDRIKTNING's LED colors.

    std::string mock_reading = "ff"; // default to red

    if (pm25_reading <= 5)
        mock_reading = "00"; // sets LEDs to GREEN
    else if (5 < pm25_reading && pm25_reading <= 10)
        mock_reading = "50"; // sets LEDs to YELLOW
    else if (10 < pm25_reading)
        mock_reading = "ff"; // sets LEDs to RED

    std::cout << "mock_reading: " << mock_reading << std::endl;

    std::vector<std::string> payload(payload_prefix.begin(),
                                     payload_prefix.end());
    std::vector<std::string> data = {"00", "00", "00", mock_reading,
                                     "00", "00", "00", "00", "00", "00",
                                     "00", "00", "00", "00", "00", "00"};
    payload.insert(payload.end(), data.begin(), data.end());

    return payload;
};

int calculate_pm25(std::vector<ByteStruct> &byte_buffer)
{
    unsigned int df3 = byte_buffer[2 + 3].decimal;
    unsigned int df4 = byte_buffer[2 + 4].decimal;
    int pm25_reading = df3 * 255 + df4;
    std::cout << "pm: " << pm25_reading << std::endl;

    // Printing
    for (ByteStruct byte : byte_buffer)
        std::cout << byte.hex << " ";
    std::cout << std::endl;

    std::cout << "DF3 = " << df3 << "; DF4 = " << df4 << std::endl
              << "PM2.5 (μg/m³) = DF3 * 256 + DF4 = " << pm25_reading
              << "\n\n";

    return pm25_reading;
}

int send_payload(int serial_port, std::vector<std::string> &payload)
{
    std::cout << "Sending: ";
    for (std::string hex_byte : payload)
    {
        unsigned char byte = static_cast<char>(hex_to_dec(hex_byte));
        std::cout << dec_to_hex(static_cast<int>(byte)) << " ";

        int n = write(serial_port, &byte, 1);
        switch (n)
        {
        case -1:
            std::cout << "Error " << errno << " from read: " << strerror(errno) << "\n";
            return 1;
        case 0:
            std::cout << "zero bytes read";
            continue;
        }
    }
    std::cout << std::endl;

    return 0;
}

int get_pm1006_message(int serial_port, std::vector<ByteStruct> &byte_buffer)
{
    /* Fills in the given byte_buffer with the next message from the pm1006. */

    /* the pm1006s send 20 byte long packets, which all start with "16 11 0b".
        wait for the starting bytes, i.e sync up with the pm1006, then start
        appending byte buffer. */

    ByteStruct byte_obj;

    while (true)
    {
        int n = read(serial_port, &byte_obj.byte, 1);
        switch (n)
        {
        case -1:
            std::cout << "Error " << errno << " from read: " << strerror(errno) << "\n";
            return 1;

        case 0:
            std::cout << "Zero bytes read\n";
            continue;
        }

        byte_obj.decimal = static_cast<unsigned int>(byte_obj.byte);
        byte_obj.hex = dec_to_hex(byte_obj.decimal);
        if (byte_obj.hex.length() == 1)
            byte_obj.hex.insert(byte_obj.hex.begin(), '0');

        byte_buffer.push_back(byte_obj);

        // Keep filling buffer until it has 3 bytes
        if (byte_buffer.size() < 3)
            continue;

        // Keep shifting new bytes in until matching the payload_prefix
        bool equals = true;

        for (unsigned short int i = 0; i < 3; i++)
            byte_buffer.at(i).hex != payload_prefix[i] ? equals = false : false; // second false acts as "pass"

        if (!equals)
        {
            for (ByteStruct byte : byte_buffer)
                std::cout << byte.hex << " ";
            std::cout << std::endl;

            byte_buffer.erase(byte_buffer.begin());
            continue;
        }

        // Fill the buffer with the rest of the message.
        if (byte_buffer.size() != PM1006_MESSAGE_LEN)
            continue;
    }

    return 0;
}
