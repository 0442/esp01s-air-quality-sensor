#include "pm1006.h"
#include <vector>
#include <string>
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <Arduino.h>

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

    return pm25_reading;
}

int send_payload(Stream& serial, std::vector<std::string> &payload)
{
    for (std::string hex_byte : payload)
    {
        unsigned char byte_chr = static_cast<char>(hex_to_dec(hex_byte));

        int n = serial.write(&byte_chr, 1);
        switch (n)
        {
        case -1:
            // serial.printf("Error %d from read: %s\n", errno, strerror(errno));
            return 1;
        case 0:
            // serial.println("zero bytes read");
            continue;
        }
    }

    return 0;
}

int get_pm1006_message(Stream& serial, std::vector<ByteStruct> &byte_buffer)
{
    /* Fills in the given byte_buffer with the next message from the pm1006. */

    /* the pm1006s send 20 byte long packets, which all start with "16 11 0b".
        wait for the starting bytes, i.e sync up with the pm1006, then start
        appending byte buffer. */

    ByteStruct byte_obj;

    while (true)
    {
        int n = serial.readBytes(&byte_obj.byte, 1);

        switch (n)
        {
        case -1:
            // serial.printf("Error %d from read: %s\n", errno, strerror(errno));
            return 1;

        case 0:
            // serial.println("Zero bytes read");
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
            /*for (byte byte : byte_buffer)
                serial.print(byte.hex);
            serial.println();*/

            byte_buffer.erase(byte_buffer.begin());
            continue;
        }

        // Fill the buffer with the rest of the message.
        if (byte_buffer.size() != PM1006_MESSAGE_LEN)
            continue;
    }

    return 0;
}
