#ifndef APPCONFIG_H_
#define APPCONFIG_H_

#define HOST_NAME "GN02475rel"

#define RELAY1 "relay1"
#define RELAY2 "relay2"
#define RELAY3 "relay3"
#define RELAY4 "relay4"

#define BUTTON1 "button1"
#define BUTTON2 "button2"
#define BUTTON3 "button3"
#define BUTTON4 "button4"

// mosfets
#include "../../secrets.h"
#include <stdint.h>
#include <Arduino.h>
#include <driver/gpio.h>
#include "shared/configtypes/configtypes.h"

struct Settings
{
#define ListenChannelsCount 0
  const char *listenChannels[ListenChannelsCount] = {};

  const gpio_num_t led = (gpio_num_t)2; // status led
  const gpio_num_t buzzer = (gpio_num_t)4;

#define CollectorCount 0
  CollectorConfig collectors[CollectorCount] = {};

#define SwitchCount 4
  SwitchConfig switches[SwitchCount] = {
      {devicet::relay, RELAY1, 25, switcht::on_off},
      {devicet::relay, RELAY2, 26, switcht::on_off},
      {devicet::relay, RELAY3, 33, switcht::on_off},
      {devicet::relay, RELAY4, 32, switcht::on_off}};

#define SensorCount 4
  SensorConfig sensors[SensorCount] = {
    {devicet::input, BUTTON1, 34, sensort::digital},
    {devicet::input, BUTTON2, 35, sensort::digital},
    {devicet::input, BUTTON3, 36, sensort::digital},
    {devicet::input, BUTTON4, 39, sensort::digital}
  };

  int getSensorIndex(const char *name)
  {
    for (size_t i = 0; i < SensorCount; i++)
    {
      if (strcmp(sensors[i].name, name) == 0)
        return i;
    }
  }

  int getSensorIndex(devicet device)
  {
    for (size_t i = 0; i < SensorCount; i++)
    {
      if (sensors[i].device == device)
        return i;
    }
  }

  int getSwitchIndex(devicet device)
  {
    for (size_t i = 0; i < SwitchCount; i++)
    {
      if (switches[i].device == device)
        return i;
    }
  }

  int getSwitchIndex(const char *name)
  {
    for (size_t i = 0; i < SwitchCount; i++)
    {
      if (strcmp(switches[i].name, name) == 0)
        return i;
    }
    return -1;
  }

  int getCollectorIndex(const char *name)
  {
    for (size_t i = 0; i < CollectorCount; i++)
    {
      if (strcmp(collectors[i].name, name) == 0)
        return i;
    }
    return -1;
  }
};

struct Intervals
{
  int statusPublish = 1000;   // interval at which status is published to MQTT
  int Can2Mqtt = 50;          // send CAN messages to MQTT every n milliseconds. Accumulate messages until. Set this to 0 for forwarding all CAN messages to MQTT as they are received.
  int DmeCANsend = 20;        // interval at which to send CAN Messages to car bus network (milliseconds)
  int ChargerCANsendS = 25;   // short interval at which to send CAN Messages to charger bus network (milliseconds), charger evse pull cp
  int ChargerCANsendL = 470;  // long interval at which to send CAN Messages to car bus network (milliseconds), charger voltage request
  int click_onceDelay = 1000; // milliseconds
};

extern Settings settings;
extern Intervals intervals;

#endif /* APPCONFIG_H_ */
