#include "Sensors.h"
#include "status.h"

Settings settingsSensors;
MqttPubSub *mqttClientSensors;

Sensors::Sensors()
{
}

void Sensors::setup(class MqttPubSub &mqtt_client)
{
  Serial.println("setup sensors mqtt");
  mqttClientSensors = &mqtt_client;
  for (size_t i = 0; i < SensorCount; i++)
  {
    SensorConfig *sc = &settings.sensors[i];
    configs[i] = new SensorConfig(sc->device, sc->name, sc->pin, sc->R1, sc->R2, sc->sensortype);
    devices[i] = new Sensor(*configs[i]);
    devices[i]->onChange([](const char *name, devicet devicetype, int value)
                         { 
                          int si=settingsSensors.getSensorIndex(devicetype);
                           status.sensors[si]=value;

                           switch (devicetype)
                           {
                            case devicet::adc_vacuum:
                              status.sensors[si]=value;
                                if (brakesSettings.manual_vacuum == 0)
                                {
                                  if (value < brakesSettings.vacuum_max) // turn pump off
                                  {
                                    status.switches[settingsSensors.getSwitchIndex(devicet::msft_vacuum)]=LOW;
                                  }
                                  else if (value > brakesSettings.vacuum_min) // turn pump on
                                  {
                                    status.switches[settingsSensors.getSwitchIndex(devicet::msft_vacuum)]=HIGH;
                                  }
                                }
                              break;
                            default:
                              if(settingsSensors.sensors[si].sensortype==sensort::adc)
                                  status.sensors[si]=value;
                                else 
                                  status.sensors[si]=value/100.0;
                              break;
                           }
                          mqttClientSensors->sendMessageToTopic(String(value), String(wifiSettings.hostname) + "/out/sensors/" + name); });
    devices[i]->setup();
  }
}

void Sensors::handle()
{
  for (size_t i = 0; i < SensorCount; i++)
  {
    devices[i]->handle();
  }
}