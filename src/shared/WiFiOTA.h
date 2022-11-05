#ifndef WIFIOTA_H_
#define WIFIOTA_H_

#include <ArduinoOTA.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <ESPmDNS.h>
#include <esp_wpa2.h>
#include "appconfig.h"
#include "../../../secrets.h"
#include "status.h"

class WiFiOTA
{
private:
  int positionAP; // used switch to next AP on every subsequent connect
  long lastOTAmillis = 0;
  static void WiFiEvent(WiFiEvent_t event);

public:
  WiFiOTA();
  void setupWiFi();
  void ReconnectWiFi();
  void setupOTA();
  void handleWiFi();
  void handleOTA();
};

#endif /* WIFIOTA_H_ */
