

#include "Bytes2WiFi.h"

static IPAddress broadcastAddr(255, 255, 255, 255);

Bytes2WiFi::Bytes2WiFi()
{
}

void Bytes2WiFi::setup(uint16_t port)
{
    serverWiFi.begin(port); // start TCP server
    serverWiFi.setNoDelay(true);
}

void Bytes2WiFi::addBuffer(byte b)
{
    if (position > BUFFER_SIZE - 1)
    {
        send();
        position = 0;
    }
    content[position++] = b;
}

void Bytes2WiFi::addBuffer(const char *buffer, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        addBuffer(buffer[i]);
    }
}

void Bytes2WiFi::send()
{
    if (serverWiFi.hasClient())
    {
        for (byte i = 0; i < MAX_NMEA_CLIENTS; i++)
        {
            // find free/disconnected spot
            if (!TCPClient[i] || !TCPClient[i].connected())
            {
                if (TCPClient[i])
                    TCPClient[i].stop();
                TCPClient[i] = serverWiFi.available();
                continue;
            }
        }
        // no free/disconnected spot so reject
        WiFiClient TmpserverClient = serverWiFi.available();
        TmpserverClient.stop();
    }

    // send bytes to WiFi
    if (position > 0)
    {
        bool sent = false;
        for (byte cln = 0; cln < MAX_NMEA_CLIENTS; cln++)
        {
            if (TCPClient[cln])
            {
                // send plain bytes
                uint8_t *buff = content;
                TCPClient[cln].write(buff, position);
                sent = true; // preserve position for next send cycle if there are clients
            }
        }
        if (sent || position > BUFFER_SIZE - 16) // reserve some space in buffer
            position = 0;
    }
}

void Bytes2WiFi::read()
{
    if (serverWiFi.hasClient())
    {
        for (byte i = 0; i < MAX_NMEA_CLIENTS; i++)
        {
            // find free/disconnected spot
            if (!TCPClient[i] || !TCPClient[i].connected())
            {
                if (TCPClient[i])
                    TCPClient[i].stop();
                TCPClient[i] = serverWiFi.available();
                continue;
            }
        }
        // no free/disconnected spot so reject
        WiFiClient TmpserverClient = serverWiFi.available();
        TmpserverClient.stop();
    }

    for (byte cln = 0; cln < MAX_NMEA_CLIENTS; cln++)
    {
        if (TCPClient[cln])
        {
            // collect command
            while (TCPClient[cln].available())
            {
                wifiCommand[wifiCmdPos] = TCPClient[cln].read(); // read char from client
                if (wifiCmdPos < 128 - 1)
                    wifiCmdPos++;
            }
            char cmd[wifiCmdPos + 1];
            String((char *)wifiCommand).toCharArray(cmd, wifiCmdPos + 1);
            if (wifiCmdPos > 1)
            {
                // Serial.println(cmd);
                //  execute command
                if (strcmp(cmd, "ping") == 0)
                {
                    addBuffer("pong", 4);
                    wifiCmdPos = 0;
                }
                else if (strcmp(cmd, "status") == 0)
                {
                    char wbuffer[1024];
                    serializeJson(status.GenerateJson(), wbuffer);
                    String st(wbuffer);
                    addBuffer(st.c_str(), st.length());
                    wifiCmdPos = 0;
                }
                else if (strcmp(cmd, "restart") == 0)
                {
                    ESP.restart();
                    wifiCmdPos = 0;
                }
                else if (strcmp(cmd, "reconnect") == 0)
                {
                    WiFi.disconnect(false, false);
                    wifiCmdPos = 0;
                }
            }
            // else
            // reset command if not known
            wifiCmdPos = 0;
        }
    }
}

void Bytes2WiFi::handle()
{
    if (strcmp(status.SSID, "") != 0)
    {
        lastReadCount++;
        if (lastReadCount > 10) // read command every 10th cycle to improve loop performance alot
        {
            lastReadCount = 0;
            read();
        }
        if (status.currentMillis - lastSend > 1000) // empty buffer every second
            if (position > 0)
            {
                lastSend = status.currentMillis;
                send();
            }

        if ((status.currentMillis - lastBroadcast) > 1000) // every second send out a broadcast ping
        {
            uint8_t buff[4] = {0x1C, 0xEF, 0xAC, 0xED};
            lastBroadcast = status.currentMillis;
            wifiUDPServer.beginPacket(broadcastAddr, 17222);
            wifiUDPServer.write(buff, 4);
            wifiUDPServer.endPacket();
        }
    }
}
