#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include <iomanip>

#include <string>
#include <list>
#include <vector>
#include <cmath>

#include <stdio.h>
#include <stdlib.h>
#include <strstream>
#include <iostream>
#include <cstddef>

#include "pm1006.h"
#include "utils.h"

#define UDP_PORT 5505
#define WIFI_NAME ""
#define WIFI_PASSWORD ""

IPAddress espIp;
IPAddress remoteIp = IPAddress(192, 168, 1, 10);

WiFiUDP Udp;

void setup()
{
    Serial.begin(9600, SERIAL_8N1); // baud 9600, 8data0parity1stop

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);

    WiFi.begin(WIFI_NAME, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED)
        delay(500);

    Udp.begin(UDP_PORT);

    espIp = WiFi.localIP();

    digitalWrite(LED_BUILTIN, HIGH);
    Serial.setTimeout(10);
}

void respond_to_request(int pm25)
{
    int packetSize = Udp.parsePacket();
    if (packetSize == 0)
        return;

    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write(pm25);
    Udp.endPacket();

    digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(LED_BUILTIN, HIGH);
}

int latest_reading = -1;

void loop()
{
    std::vector<ByteStruct> byte_buffer;
    int pm25_reading = -1;

    for (;;){
        respond_to_request(pm25_reading);

        get_pm1006_message(Serial, byte_buffer);
        pm25_reading = calculate_pm25(byte_buffer);
        auto payload = mock_payload(pm25_reading);
        send_payload(Serial, payload);
        byte_buffer.clear();
    }
}
