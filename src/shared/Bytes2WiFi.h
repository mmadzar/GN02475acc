#ifndef BYTES2WIFI_H_
#define BYTES2WIFI_H_

#define MAX_NMEA_CLIENTS 1 // keep this low for increased loop performance on read() and send()
#include <WiFi.h>
#include <WiFiClient.h>
#include "../src/status.h"

class Bytes2WiFi
{
public:
    Bytes2WiFi();
    void setup(uint16_t port);
    void send();
    void read();
    void handle();
    void addBuffer(byte b);
    void addBuffer(const char *buffer, size_t size);
    byte wifiCommand[128];
    int wifiCmdPos = 0;

private:
#define BUFFER_SIZE 2048
    long lastSend = 0;
    long lastReadCount = 0;
    WiFiServer(serverWiFi);
    WiFiClient TCPClient[MAX_NMEA_CLIENTS];

    byte content[BUFFER_SIZE];
    int position = 0;
    uint32_t lastMicros;
    WiFiUDP wifiUDPServer;
    uint32_t lastBroadcast;
};

#endif /* BYTES2WIFI_H_ */
