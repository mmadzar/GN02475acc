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
  char msg[length + 1];
  for (size_t i = 0; i < length; i++)
    msg[i] = (char)message[i];
  msg[length] = 0x0a; // important to add termination to string! messes string value if ommited

  String t = String(topic);
  String cmd = t.substring(String("GN02475inv/out/").length(), t.length());
  if (length > 0)
  {
    if (cmd.equals("inverter/rpm"))
    {
      //status.rpm = String((const char *)message).toInt();
    }
    else if (cmd.equals("inverter/pot")) // for testing responsivnes
      status.rpm = String((const char *)message).toInt();
    //Serial.println(cmd);
  }
}

void MqttMessageHandler::handle()
{
}