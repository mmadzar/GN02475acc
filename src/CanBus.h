#ifndef CANBUS_H_
#define CANBUS_H_

#include <Arduino.h>
#include "shared/base/Collector.h"
#include "shared/configtypes/configtypes.h"
#include <mcp_can.h>
#include <SPI.h>
#include "appconfig.h"
#include <ArduinoJson.h>
#include "status.h"
#include "shared/Bytes2WiFi.h"
#include "shared/MqttPubSub.h"
#define CAN0_INT 17 // Set INT pin
#define CAN0_CS 5   // Set CS pin

class CanBus
{
private:
  Collector *collectors[CollectorCount];
  CollectorConfig *configs[CollectorCount];
  Bytes2WiFi *b2w;
  Settings settings;
  void init();
  uint64_t previousDme = 0;      // will store last time a CAN Message was send
  uint64_t previousChargerS = 0; // charger evse pull cp
  uint64_t previousChargerL = 0; // charger voltage request
  long consumptionCounter = 0;
  long lastSentCanLog = 0; // last time when logged CAN messages are sent over WiFi

public:
  CanBus();
  void handle();
  void setup(class MqttPubSub &mqtt_client, Bytes2WiFi &wifiport);
  void sendMessageSet();
};

#endif /* CANBUS_H_ */
