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
                          int si=settingsSensors.getSensorIndex(name);
                          bool changed=status.sensors[si]!=value;
                          status.sensors[si]=value;
                          if(changed)
                          {
                            mqttClientSensors->sendMessageToTopic(String(value), String(wifiSettings.hostname) + "/out/sensors/" + name); 
                            if(strcmp(name, BUTTON1)==0 && value ==0)
                              status.chargerStarted = 0;
                            if(strcmp(name, BUTTON2)==0 && value ==0)
                              status.chargerStarted = 1;
                          }
                        });
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