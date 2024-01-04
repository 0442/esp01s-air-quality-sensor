#ifndef UTILS_H
#define UTILS_H

#include <string>

std::string dec_to_hex(int number);
unsigned int hex_to_dec(std::string &hex_num);
struct ByteStruct
{
    unsigned char byte;   // memory for receiving a single byte (8 bits) from read
    unsigned int decimal; // decimal representation of byte
    std::string hex;      // hex representation of byte
};

#endif // UTILS_H