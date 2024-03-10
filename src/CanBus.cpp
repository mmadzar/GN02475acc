#include "CanBus.h"

StaticJsonDocument<512> docJ;
char tempBufferCan[512];

long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];
MCP_CAN CAN0(CAN0_CS);

long displayValue = 0;
int consumptionCounter = 0; // 0 - 65535(0xFFFF)
MqttPubSub *mqttClientCan;
Settings settingsCollectors;

long frameIds[5];
byte frames[5][8];

CanBus::CanBus()
{
  init();
}

void CanBus::init()
{
}

void CanBus::setup(class MqttPubSub &mqtt_client, Bytes2WiFi &wifiport)
{
  mqttClientCan = &mqtt_client;
  b2w = &wifiport;

  CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ);
  CAN0.setMode(MCP_NORMAL); // Set operation mode to normal so the MCP2515 sends acks to received data.

  // init frames for sending
  // lamps off
  frameIds[0] = 0x545;
  frames[0][0] = 0x00; // no engine, cruise or EML
  frames[0][1] = 0x00; // hfc.slice(0, 2)
  frames[0][2] = 0x00; // hfc.slice(2, 4)
  frames[0][3] = 0x00; // no overheat light
  frames[0][4] = 0x7E;
  frames[0][5] = 0x10;
  frames[0][6] = 0x00;
  frames[0][7] = 0x18;

  // RPM - test 3k rpm * 6.4 = 4b00
  frameIds[1] = 0x316;
  frames[1][0] = 0x05;
  frames[1][1] = 0x62;
  frames[1][2] = 0x00; // rpm 0,2
  frames[1][3] = 0x00; // rpm 2,4
  frames[1][4] = 0x65;
  frames[1][5] = 0x12;
  frames[1][6] = 0x00;
  frames[1][7] = 0x62;

  // Temp - temp needle to beginning of red section [0xE6] - celsius*2
  frameIds[2] = 0x329;
  frames[2][0] = 0xD9;
  frames[2][1] = 0xE6;
  frames[2][2] = 0xB2;
  frames[2][3] = 0x19;
  frames[2][4] = 0x00;
  frames[2][5] = 0xEE;
  frames[2][6] = 0x00;
  frames[2][7] = 0x00;

  // connect EVSE - every 100ms - BMS heartbeat
  frameIds[3] = 0x285;
  frames[3][0] = 0x00;
  frames[3][1] = 0x00;
  frames[3][2] = 0x00; // b6 to activate EVSE
  frames[3][3] = 0x00;
  frames[3][4] = 0x00;
  frames[3][5] = 0x00;
  frames[3][6] = 0x00;
  frames[3][7] = 0x00;

  // set charge - every 800ms
  frameIds[4] = 0x286;
  frames[4][0] = 0x28; // voltage
  frames[4][1] = 0x0d; // voltage 0f 388, 0d 337
  frames[4][2] = 0x3c; // charge current 78 12A, 3c 6A
  frames[4][3] = 0x37;
  frames[4][4] = 0x00;
  frames[4][5] = 0x00;
  frames[4][6] = 0x0A;
  frames[4][7] = 0x00;

  for (size_t i = 0; i < CollectorCount; i++)
  {
    CollectorConfig *sc = &settings.collectors[i];
    configs[i] = new CollectorConfig(sc->name, sc->sendRate);
    collectors[i] = new Collector(*configs[i]);
    collectors[i]->onChange([](const char *name, int value, int min, int max, int samplesCollected, uint64_t timestamp)
                            { 
                              status.collectors[settingsCollectors.getCollectorIndex(name)]=value;
                              JsonObject root = docJ.to<JsonObject>();
                              root["value"]=value;
                              root["min"]=min;
                              root["max"]=max;
                              root["timestamp"]=timestamp;
                              root["samples"]=samplesCollected;

                              serializeJson(docJ, tempBufferCan);
                              mqttClientCan->sendMessageToTopic(String(wifiSettings.hostname) + "/out/collectors/" + name, tempBufferCan); });
    collectors[i]->setup();
  }
}
void CanBus::handle()
{
}

void CanBus::sendMessageSet()
{
  status.currentMillis = millis();
  // create and send can message in intervals for IKE and charger
  if (status.currentMillis - previousDme >= intervals.DmeCANsend)
  {
    previousDme = status.currentMillis;

    consumptionCounter += status.consumptionCAN;
    if (consumptionCounter >= 65534)
      consumptionCounter = 0;

    // update rpm and coolant temp values
    displayValue = status.rpmCAN * 6.4;
    frames[1][2] = (int)((displayValue & 0X000000FF));      // rpm 0,2
    frames[1][3] = (int)((displayValue & 0x0000FF00) >> 8); // rpm 2,4
    displayValue = status.temperatureCAN * 2;
    frames[2][1] = (int)((displayValue & 0X000000FF));

    frames[0][1] = (int)((consumptionCounter & 0X000000FF));      // consumption counter 0,2
    frames[0][2] = (int)((consumptionCounter & 0x0000FF00) >> 8); // consumption counter 2,4

    CAN0.sendMsgBuf(frameIds[0], 0, 8, frames[0]);
    CAN0.sendMsgBuf(frameIds[1], 0, 8, frames[1]);
    CAN0.sendMsgBuf(frameIds[2], 0, 8, frames[2]);
  }

  // always send 0x285 as BMS hear beat
  if (status.currentMillis - previousChargerS >= intervals.ChargerCANsendS)
  {
    previousChargerS = status.currentMillis;
    if (status.chargerPullEVSE == 1)
      frames[3][2] = 0xB6;
    else
      frames[3][2] = 0x00;

    CAN0.sendMsgBuf(frameIds[3], 0, 8, frames[3]);
  }

  if (status.chargerStarted == 1 && status.currentMillis - previousChargerL >= intervals.ChargerCANsendL)
  {
    previousChargerL = status.currentMillis;
    frames[4][0] = (int)((status.chargerVoltageRequest & 0X000000FF));      // voltage*10 big endian
    frames[4][1] = (int)((status.chargerVoltageRequest & 0x0000FF00) >> 8); //
    frames[4][2] = status.chargerCurrentRequest;                            // current*10
    CAN0.sendMsgBuf(frameIds[4], 0, 8, frames[4]);
  }
}
