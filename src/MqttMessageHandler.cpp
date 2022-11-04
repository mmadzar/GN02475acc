#include "Arduino.h"
#include "appconfig.h"
#include "status.h"
#include "MqttMessageHandler.h"

MqttMessageHandler::MqttMessageHandler()
{
}

void MqttMessageHandler::HandleMessage(const char *command, const char *message)
{
  if (command == "vacuum_min")
    brakesSettings.vacuum_min = String(message).toInt();
  else if (command == "vacuum_max")
    brakesSettings.vacuum_max = String(message).toInt();
  else
  {
    for (size_t i = 0; i < SwitchCount; i++)
    {
      SwitchConfig *sc = &pinsSettings.switches[i];
      // find switch in settings and set status value by index
      if (String(sc->name).equals(command))
      {
        status.switches[i] = String(message).toInt();
        break;
      }
    }
  }
}
