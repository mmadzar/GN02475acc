#ifndef APPCONFIG_H_
#define APPCONFIG_H_

#define HOST_NAME "GN02475acc"

#define SPEED "speed"

// DCDC converter
#define DCVOLTAGE "dcVoltage"
#define DCCURRENT "dcCurrent"
#define DCTEMP1 "dcTemp1"
#define DCTEMP2 "dcTemp2"
#define DCTEMP3 "dcTemp3"
#define DCSTATUS "dcStatus"

// Charger
#define VOLTAGE "batteryVoltage"
#define SUPPLYVOLTAGE "supplyVoltage"
#define SUPPLYCURRENT "supplyCurrent"
#define TEMP1 "temp1"
#define TEMP2 "temp2"
#define TEMP3 "temp3"
#define TEMP4 "temp4"
#define DCBUSVOLTAGE "dcBusVoltage"
#define EVSECTC "evseCTC"

// mosfets
#include "../../secrets.h"
#include <stdint.h>
#include <Arduino.h>
#include <driver/gpio.h>
#include "shared/configtypes/configtypes.h"

struct Settings
{
#define ListenChannelsCount 1
  const char *listenChannels[ListenChannelsCount] = {"GN02475inv/out/collectors/rpm"};

  const gpio_num_t led = (gpio_num_t)2;      // status led
  const gpio_num_t can0_rx = (gpio_num_t)32; // can0 transciever rx line
  const gpio_num_t can0_tx = (gpio_num_t)22; // can0 transciever tx line

#define CollectorCount 16
  // all 0 delay, controlled by interval of requests to charger
  CollectorConfig collectors[CollectorCount] = {
      {DCVOLTAGE, 0},     // (h04DC=12,45V -> 0,01V/bit)
      {DCCURRENT, 0},     // (H53=8,3A -> 0,1A/bit)
      {DCTEMP1, 0},       // (starts at -40degC, +1degC/bit)
      {DCTEMP2, 0},       // (starts at -40degC, +1degC/bit)
      {DCTEMP3, 0},       // (starts at -40degC, +1degC/bit)
      {DCSTATUS, 0},      // (h20=standby, h21=error, h22=in operation)
      {VOLTAGE, 0},       // Battery Voltage (as seen by the charger), needs to be scaled x 2, so can represent up to 255*2V; used to monitor battery during charge
      {SUPPLYVOLTAGE, 0}, // Charger supply voltage, no scaling needed
      {SUPPLYCURRENT, 0}, // Charger Supply Current x 10
      {TEMP1, 0},         // temp x 2?
      {TEMP2, 0},         // temp x 2?
      {TEMP3, 0},         
      {TEMP4, 0},         
      {EVSECTC, 0},      // EVSE Control Duty Cycle (granny cable ~26 = 26%),
      {DCBUSVOLTAGE, 0},
      {SPEED, 0}};

#define SwitchCount 8
  SwitchConfig switches[SwitchCount] = {
      {devicet::msft_vacuum, "msft_vacuum", 21, switcht::on_off},                  // mosfet gate 1  - brake vacuum pump
      {devicet::msft_servo, "msft_servo", 17, switcht::on_off},                    // mosfet gate 2  - steering servo pump
      {devicet::msft_coolant_pwm, "msft_coolant_pwm", 33, 1, switcht::pwm_signal}, // mosfet gate 00 - coolant pump PWM
      {devicet::msft_heater_pwm, "msft_heater_pwm", 18, 2, switcht::pwm_signal},   // mosfet gate 01 - heater pump PWM
      {devicet::msft_01_pwm, "msft1", 19, switcht::on_off},                        // mosfet gate 02
      {devicet::msft_02_pwm, "msft2", 23, switcht::on_off},                        // mosfet gate 03
      {devicet::msft_03_pwm, "msft3", 5, 3, switcht::pwm_signal},                  // mosfet gate 04 - charger EVSE - TODO 1 kHz set in base class
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
  int statusPublish = 1000;     // interval at which status is published to MQTT
  int Can2Mqtt = 1000;          // send CAN messages to MQTT every n secconds. Accumulate messages until. Set this to 0 for forwarding all CAN messages to MQTT as they are received.
  int CANsend = 20000;          // interval at which to send CAN Messages to car bus network (microseconds)
  int outChannelsPublish = 500; // interval at which status is published to MQTT out channels
  int click_onceDelay = 1000;   // milliseconds
};

struct BrakesSettings
{
  uint8_t manual_vacuum = 0;
  uint16_t vacuum_min = 2319; // ADC value - turn pump on
  uint16_t vacuum_max = 1450; // ADC value - turn pump off cca 1.4V
};

extern Settings settings;
extern Intervals intervals;
extern BrakesSettings brakesSettings;

#endif /* APPCONFIG_H_ */
