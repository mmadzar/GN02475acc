#include <Arduino.h>
#include "shared/WiFiOTA.h"
#include "MqttMessageHandler.h"
#include "appconfig.h"
#include "status.h"
#include "shared/MqttPubSub.h"
#include <string.h>
#include "shared/Bytes2WiFi.h"
#include "Sensors.h"
#include "Switches.h"
#include "CanBus.h"

Status status;
Settings pins;
Intervals intervals;
WiFiSettings wifiSettings;
WiFiOTA wota;
MqttPubSub mqtt;
Switches relays;
Sensors buttons;
Bytes2WiFi bytesWiFi;
CanBus can;

long loops = 0;
long lastLoopReport = 0;

bool firstRun = true;

long lastVacuumReadTime = 0;
int lastVacuumRead = 0;

void setup()
{
  SETTINGS.loadSettings();
  relays.setup(mqtt);
  buttons.setup(mqtt);
  pinMode(pins.led, OUTPUT);
  wota.setupWiFi();
  wota.setupOTA();
  mqtt.setup();
  bytesWiFi.setup(23);
  can.setup(mqtt, bytesWiFi);
}

void loop()
{
  status.currentMillis = millis();
  if (status.currentMillis - lastLoopReport > 1000) // number of loops in 1 second - for performance measurement
  {
    lastLoopReport = status.currentMillis;
    // Serial.print("Loops in a second: ");
    // Serial.println(loops);
    status.loops = loops;
    loops = 0;
    if (status.timeinfo.tm_year == 70)
    {
      getLocalTime(&(status.timeinfo), 10);
      if (status.timeinfo.tm_year != 70)
      {
        strftime(status.upsince, sizeof(status.upsince), "%Y-%m-%d %H:%M:%S UTC", &(status.timeinfo));
        strftime(status.connectedsince, sizeof(status.connectedsince), "%Y-%m-%d %H:%M:%S UTC", &(status.timeinfo));
      }
    }
  }
  else
  {
    loops++;
  }

  can.sendMessageSet();
  can.handle();

  relays.handle();
  buttons.handle();

  wota.handleWiFi();
  wota.handleOTA();
  if (loops % 5 == 0) // check mqtt every 5th cycle
    mqtt.handle();

  bytesWiFi.handle();
  mqtt.publishStatus(!firstRun);
  firstRun = false;
}
