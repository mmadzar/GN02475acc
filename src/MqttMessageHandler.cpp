#include "Arduino.h"
#include "appconfig.h"
#include "status.h"
#include "MqttMessageHandler.h"

static StaticJsonDocument<1024> mhdoc;

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
  else if (strcmp(command, "manual_vacuum") == 0)
    brakesSettings.manual_vacuum = String(message).toInt();
  else if (strcmp(command, "charger_voltage_request") == 0)
    status.chargerVoltageRequest = String(message).toInt();
  else if (strcmp(command, "charger_current_request") == 0)
    status.chargerCurrentRequest = String(message).toInt();
  else if (strcmp(command, "charger_started") == 0)
    status.chargerStarted = String(message).toInt();
  else if (strcmp(command, "charger_pull_evse") == 0)
    status.chargerPullEVSE = String(message).toInt();
  else if (strcmp(command, "wifiCan") == 1)
    status.wifiCan = String(message).toInt();
  else
  {
    for (size_t i = 0; i < SwitchCount; i++)
    {
      SwitchConfig *sc = &settings.switches[i];
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
  // only subscribed to GN02475inv/out/collectors/rpm
  if (length > 0)
  {
    String t = String(topic);
    if (t.equals("GN02475inv/out/collectors/rpm"))
    {
      char msg[length + 1];
      for (size_t i = 0; i < length; i++)
        msg[i] = (char)message[i];
      msg[length] = 0x0a;
      deserializeJson(mhdoc, message);
      status.rpm = mhdoc["max"];
      if (status.rpm < 600)
        status.rpm = 600;
    }
  }
}

void MqttMessageHandler::handle()
{
}