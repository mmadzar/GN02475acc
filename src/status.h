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

  int sensors[SensorCount];
  int switches[SwitchCount]{
      -1, -1, -1, -1, -1, -1, -1, -1};

  JsonObject GenerateJson()
  {

    JsonObject root = this->PrepareRoot();

    JsonObject jbrakes = root.createNestedObject("brakes");
    jbrakes["vacuum_min"] = brakesSettings.vacuum_min;
    jbrakes["vacuum_max"] = brakesSettings.vacuum_max;

    JsonObject jdisplay = root.createNestedObject("IKE");
    jdisplay["coolant_temp"] = coolant_temp;
    jdisplay["rpm"] = rpm;
    jdisplay["fuel_level"] = ikeFuelLevel;

    JsonObject jsensors = root.createNestedObject("sensors");
    for (size_t i = 0; i < SensorCount; i++)
      jsensors[pinsSettings.sensors[i].name] = sensors[i];

    JsonObject jswitches = root.createNestedObject("switches");
    for (size_t i = 0; i < SwitchCount; i++)
      jswitches[pinsSettings.switches[i].name] = switches[i];

    return root;
  }
};

extern Status status;

#endif /* STATUS_H_ */
