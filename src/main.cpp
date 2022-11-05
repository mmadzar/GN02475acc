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

Status status;
PinsSettings pins;
Intervals intervals;
WiFiSettings wifiSettings;
BrakesSettings brakesSettings;
WiFiOTA wota;
MqttPubSub mqtt;
Switches pwmCtrl;
Sensors sensors;
Bytes2WiFi bytesWiFi;

long loops = 0;
long lastLoopReport = 0;

bool firstRun = true;

long lastVacuumReadTime = 0;
int lastVacuumRead = 0;

void setup()
{
  SETTINGS.loadSettings();
  pwmCtrl.setup(mqtt);
  sensors.setup(mqtt);
  pinMode(pinsSettings.led, OUTPUT);
  Serial.begin(115200);
  // delay(500);
  Serial.println("Serial started!");
  wota.setupWiFi();
  wota.setupOTA();
  mqtt.setup();
  bytesWiFi.setup();
}

void loop()
{
  status.currentMillis = millis();
  if (status.currentMillis - lastLoopReport > 1000) // number of loops in 1 second - for performance measurement
  {
    lastLoopReport = status.currentMillis;
    Serial.print("Loops in a second: ");
    Serial.println(loops);
    status.loops = loops;
    loops = 0;
    // refresh monitor values
    status.freeMem = esp_get_free_heap_size();
    status.minFreeMem = esp_get_minimum_free_heap_size();
  }
  else
  {
    loops++;
  }

  sensors.handle();
  pwmCtrl.handle();

  wota.handleWiFi();
  wota.handleOTA();
  if (loops % 10 == 0) // check mqtt every 10th cycle
    mqtt.handle();

  bytesWiFi.handle();
  mqtt.publishStatus(!firstRun);
  firstRun = false;
}
