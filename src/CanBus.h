#ifndef CANBUS_H_
#define CANBUS_H_

#include <Arduino.h>
#include <esp32_can.h>
#include <CAN_config.h>
#include "appconfig.h"
#include <ArduinoJson.h>
#include "status.h"
#include "shared/Bytes2WiFi.h"
#include "shared/MqttPubSub.h"

class CanBus
{
private:
  Bytes2WiFi *b2w;
  Settings settings;
  void init();
  CAN_device_t CAN_cfg;    // CAN Config
  long previousMillis = 0; // will store last time a CAN Message was send
  int handle613(CAN_FRAME frame);

public:
  CanBus();
  void handle();
  void setup(class MqttPubSub &mqtt_client, Bytes2WiFi &wifiport);
  void sendMessageSet();
};

#endif /* CANBUS_H_ */
