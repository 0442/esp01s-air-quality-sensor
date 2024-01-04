#ifndef PM1006_H
#define PM1006_H

#include <vector>
#include <string>
#include "utils.h"
#include <Arduino.h>

std::vector<std::string> mock_payload(int pm25_reading);
int calculate_pm25(std::vector<ByteStruct> &byte_buffer);
int send_payload(Stream& serial, std::vector<std::string> &payload);
int get_pm1006_message(Stream& serial, std::vector<ByteStruct> &byte_buffer);

#endif // PM1006_CPP