#ifndef STATUS_H_
#define STATUS_H_

#include "appconfig.h"
#include "shared/status_base.h"

class Status : public StatusBase
{
public:
  int sensors[SensorCount];
  int switches[SwitchCount]{
      -1, -1, -1, -1, -1, -1, -1, -1};

  JsonObject GenerateJson()
  {

    JsonObject root = this->PrepareRoot();
    root["vacuum_min"] = brakesSettings.vacuum_min;
    root["vacuum_max"] = brakesSettings.vacuum_max;

    JsonObject jsensors = root.createNestedObject("sensors");
    for (size_t i = 0; i < SensorCount; i++)
    {
      jsensors[pinsSettings.sensors[i].name] = sensors[i];
    }

    JsonObject jswitches = root.createNestedObject("switches");
    for (size_t i = 0; i < SwitchCount; i++)
    {
      jswitches[pinsSettings.switches[i].name] = switches[i];
    }

    return root;
  }
};

extern Status status;

#endif /* STATUS_H_ */
