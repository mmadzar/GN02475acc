#ifndef STATUS_H_
#define STATUS_H_

#include "appconfig.h"
#include "shared/status_base.h"

class Status : public StatusBase
{
public:
  int rpm = 600; // minimum value to keep DSC working
  int coolant_temp = 90;
  int ikeFuelLevel = -1;
  int wifiCan = 0;
  int chargerStarted = 0;
  int chargerPullEVSE = 0;
  int chargerVoltageRequest = 3350;
  int chargerCurrentRequest = 30;

  int collectors[CollectorCount];
  int sensors[SensorCount];
  int switches[SwitchCount]{
      -1, -1, -1, -1, -1, -1, -1, -1};

  JsonObject GenerateJson()
  {

    JsonObject root = this->PrepareRoot();
    root["wifiCan"] = wifiCan;

    JsonObject jbrakes = root.createNestedObject("brakes");
    jbrakes["manual_vacuum"] = brakesSettings.manual_vacuum;
    jbrakes["vacuum_min"] = brakesSettings.vacuum_min;
    jbrakes["vacuum_max"] = brakesSettings.vacuum_max;

    JsonObject jdisplay = root.createNestedObject("IKE");
    jdisplay["coolant_temp"] = coolant_temp;
    jdisplay["rpm"] = rpm;
    jdisplay["fuel_level"] = ikeFuelLevel;

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
