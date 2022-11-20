#include "Arduino.h"
#include "appconfig.h"
#include "status.h"
#include "MqttMessageHandler.h"

MqttMessageHandler::MqttMessageHandler()
{
}

void MqttMessageHandler::HandleMessage(const char *command, const char *message, int length)
{
  if (strcmp(command, "coolant_temp") == 0)
    status.coolant_temp = String(message).toInt();
  else if (strcmp(command, "rpm") == 0)
    status.rpm = String(message).toInt();
  else if (strcmp(command, "vacuum_min") == 0)
    brakesSettings.vacuum_min = String(message).toInt();
  else if (strcmp(command, "vacuum_max") == 0)
    brakesSettings.vacuum_max = String(message).toInt();
  else
  {
    for (size_t i = 0; i < SwitchCount; i++)
    {
      SwitchConfig *sc = &pinsSettings.switches[i];
      // find switch in settings and set status value by index
      if (strcmp(sc->name, command) == 0)
      {
        status.switches[i] = String(message).toInt();
        break;
      }
    }
  }
}

void MqttMessageHandler::callback(char *topic, byte *message, unsigned int length)
{

}

void MqttMessageHandler::handle()
{
}