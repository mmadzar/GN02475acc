#ifndef APPCONFIG_H_
#define APPCONFIG_H_

#define HOST_NAME "GN02475acc222"

#include "../../secrets.h"
#include <stdint.h>
#include <Arduino.h>
#include <driver/gpio.h>
#include "shared/configtypes/configtypes.h"

struct PinsSettings
{
  const gpio_num_t led = (gpio_num_t)2;      // status led
  const gpio_num_t can0_rx = (gpio_num_t)32; // can0 transciever rx line
  const gpio_num_t can0_tx = (gpio_num_t)22; // can0 transciever tx line

#define SwitchCount 8
  SwitchConfig switches[SwitchCount] = {
      {devicet::msft_vacuum, "msft_vacuum", 21, switcht::on_off},                  // mosfet gate 1  - brake vacuum pump
      {devicet::msft_servo, "msft_servo", 17, switcht::on_off},                    // mosfet gate 2  - steering servo pump
      {devicet::msft_coolant_pwm, "msft_coolant_pwm", 33, 1, switcht::pwm_signal}, // mosfet gate 00 - coolant pump PWM
      {devicet::msft_heater_pwm, "msft_heater_pwm", 18, 2, switcht::pwm_signal},   // mosfet gate 01 - heater pump PWM
      {devicet::msft_01_pwm, "msft1", 19, switcht::on_off},                        // mosfet gate 02
      {devicet::msft_02_pwm, "msft2", 23, switcht::on_off},                        // mosfet gate 03
      {devicet::msft_03_pwm, "msft3", 5, switcht::on_off},                         // mosfet gate 04
      {devicet::msft_04_pwm, "msft4", 16, switcht::on_off}                         // mosfet gate 05
  };

#define SensorCount 1
  SensorConfig sensors[SensorCount] = {
      {devicet::adc_vacuum, "adc_vacuum", 34, sensort::adc} // brake vacuum sensor
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
};

struct Intervals
{
  int statusPublish = 1000;     // interval at which status is published to MQTT
  int Can2Mqtt = 1000;          // send CAN messages to MQTT every n secconds. Accumulate messages until. Set this to 0 for forwarding all CAN messages to MQTT as they are received.
  int CANsend = 10;             // interval at which to send CAN Messages to car bus network (milliseconds)
  int outChannelsPublish = 500; // interval at which status is published to MQTT out channels
  int click_onceDelay = 1000;   // milliseconds
};

struct BrakesSettings
{
  uint16_t vacuum_min = 2519; // ADC value - turn pump on cca 1.7V
  uint16_t vacuum_max = 1350; // ADC value - turn pump off cca 1.4V
};

extern PinsSettings pinsSettings;
extern Intervals intervals;
extern BrakesSettings brakesSettings;

#endif /* APPCONFIG_H_ */
