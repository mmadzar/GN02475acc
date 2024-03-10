#ifndef STATUS_H_
#define STATUS_H_

#include "appconfig.h"
#include "shared/status_base.h"

struct Status : public StatusBase
{
  // injected CAN values
  int consumptionCAN = 50;
  int temperatureCAN = 90;
  int rpmCAN = 3000;

  int chargerStarted = 0;
  int chargerPullEVSE = 0;
  int chargerVoltageRequest = 3350;
  int chargerCurrentRequest = 30;

  int collectors[CollectorCount];
  int sensors[SensorCount];
  int switches[SwitchCount]{0, 0, 0, 0};
  int tonef = 0;
  int toned = 0;

  JsonObject GenerateJson()
  {

    JsonObject root = this->PrepareRoot();
    root["tonef"] = tonef;
    root["toned"] = toned;
    JsonObject jcaninject = root.createNestedObject("dme");
    jcaninject["consumption"] = consumptionCAN;
    jcaninject["rpm"] = rpmCAN;
    jcaninject["temperature"] = temperatureCAN;

    JsonObject jcharger = root.createNestedObject("charger");
    jcharger["started"] = chargerStarted;
    jcharger["voltage_request"] = chargerVoltageRequest;
    jcharger["current_request"] = chargerCurrentRequest;
    jcharger["pull_evse"] = chargerPullEVSE;

    JsonObject jcollectors = root.createNestedObject("collectors");
    for (size_t i = 0; i < CollectorCount; i++)
      jcollectors[settings.collectors[i].name] = collectors[i];

    JsonObject jsensors = root.createNestedObject("sensors");
    for (size_t i = 0; i < SensorCount; i++)
      jsensors[settings.sensors[i].name] = sensors[i];

    JsonObject jswitches = root.createNestedObject("switches");
    for (size_t i = 0; i < SwitchCount; i++)
      jswitches[settings.switches[i].name] = switches[i];

    return root;
  }
};

extern Status status;

#endif /* STATUS_H_ */
