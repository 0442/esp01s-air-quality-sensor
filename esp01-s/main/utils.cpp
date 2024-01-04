#include "utils.h"
#include <cmath>
#include <string>
#include <vector>


std::string dec_to_hex(int number)
{
    const char hex_digits[16] = {
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

    std::string base_16_str;

    std::vector<int> remainders;

    int remainder;
    int quotient = number;

    while (true)
    {
        remainder = quotient % 16;
        quotient = (quotient - remainder) / 16;

        remainders.push_back(remainder);

        if (quotient == 0)
            break;
    }

    for (std::vector<int>::reverse_iterator i = remainders.rbegin(); i != remainders.rend(); i++)
    {
        std::string as_string(sizeof(char), hex_digits[*i]);
        base_16_str.append(as_string);
    }

    return base_16_str;
}

unsigned int hex_to_dec(std::string& hex_num)
{
    const char hex_digits[16] = {
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

    std::vector<int> hex_digits_as_decs = {};
    for (char num_digit : hex_num)
        for (int i = 0; i < sizeof(hex_digits); i++)
            if (num_digit == hex_digits[i])
                hex_digits_as_decs.push_back(i);

    if (hex_digits_as_decs.size() < 1)
        return 0;

    unsigned int dec_num = 0;

    const unsigned int n_digits = hex_num.length();
    int exponent;
    for (int i = 0; i < n_digits; i++)
    {
        exponent = n_digits - i;
        dec_num += pow(16, exponent - 1) * hex_digits_as_decs[i];
    }

    return dec_num;
}