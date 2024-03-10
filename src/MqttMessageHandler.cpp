#include "Arduino.h"
#include "appconfig.h"
#include "status.h"
#include "MqttMessageHandler.h"

MqttMessageHandler::MqttMessageHandler()
{
}

void MqttMessageHandler::HandleMessage(const char *command, const char *message, int length)
{
  if (strcmp(command, "dme_temperature") == 0)
    status.temperatureCAN = String(message).toInt();
  else if (strcmp(command, "dme_rpm") == 0)
    status.rpmCAN = String(message).toInt();
  else if (strcmp(command, "dme_consumption") == 0)
    status.consumptionCAN = String(message).toInt();
  else if (strcmp(command, "charger_voltage_request") == 0)
    status.chargerVoltageRequest = String(message).toInt();
  else if (strcmp(command, "charger_current_request") == 0)
    status.chargerCurrentRequest = String(message).toInt();
  else if (strcmp(command, "charger_started") == 0)
    status.chargerStarted = String(message).toInt();
  else if (strcmp(command, "charger_pull_evse") == 0)
    status.chargerPullEVSE = String(message).toInt();
  else if (strcmp(command, "tone") == 0)
  {
    String pload = String(message);
    // status.tonef = pload.substring(0, pload.indexOf(" ")).toInt();
    // status.toned = pload.substring(pload.indexOf(" ") + 1).toInt();
    // tone(settings.buzzer, status.tonef, status.toned);
    tone(settings.buzzer, pload.substring(0, pload.indexOf(" ")).toInt(), pload.substring(pload.indexOf(" ") + 1).toInt());
  }
  else
  {
    int s = settings.getSwitchIndex(command);
    if (s >= 0)
      status.switches[s] = String(message).toInt();
  }
}

void MqttMessageHandler::callback(char *topic, byte *message, unsigned int length)
{
}

void MqttMessageHandler::handle()
{
}