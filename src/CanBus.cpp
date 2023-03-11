#include "CanBus.h"

StaticJsonDocument<512> docJ;
char tempBufferCan[512];
CAN_FRAME frames[5];
long displayValue = 0;
int consumptionCounter = 0; // 0 - 65535(0xFFFF)
MqttPubSub *mqttClientCan;
Settings settingsCollectors;

CanBus::CanBus()
{
  init();
}

void CanBus::init()
{
  CAN0.setCANPins(settings.can0_rx, settings.can0_tx);
}

void CanBus::setup(class MqttPubSub &mqtt_client, Bytes2WiFi &wifiport)
{
  mqttClientCan = &mqtt_client;
  b2w = &wifiport;

  CAN0.begin(500000);
  CAN0.watchFor(); // watch all - performance OK until connected clients!
  // // BMW IKE
  // CAN0.watchFor(0x613);
  // // BMW DSC
  // CAN0.watchFor(0x153); // speed  km/h
  // CAN0.watchFor(0x1F0); // wheel speed km/h

  // // CAN0.watchFor(0x545);
  // // CAN0.watchFor(0x316);
  // // CAN0.watchFor(0x329);

  // init frames for sending
  // lamps off
  CAN_FRAME txFrame1;
  txFrame1.rtr = 0;
  txFrame1.extended = false;
  txFrame1.length = 8;
  txFrame1.id = 0x545;
  txFrame1.data.uint8[0] = 0x00; // no engine, cruise or EML
  txFrame1.data.uint8[1] = 0x00; // hfc.slice(0, 2)
  txFrame1.data.uint8[2] = 0x00; // hfc.slice(2, 4)
  txFrame1.data.uint8[3] = 0x00; // no overheat light
  txFrame1.data.uint8[4] = 0x7E;
  txFrame1.data.uint8[5] = 0x10;
  txFrame1.data.uint8[6] = 0x00;
  txFrame1.data.uint8[7] = 0x18;
  frames[0] = txFrame1;

  // RPM - test 3k rpm * 6.4 = 4b00
  CAN_FRAME txFrame2;
  txFrame2.rtr = 0;
  txFrame2.extended = false;
  txFrame2.length = 8;
  txFrame2.id = 0x316;
  txFrame2.data.uint8[0] = 0x05;
  txFrame2.data.uint8[1] = 0x62;
  txFrame2.data.uint8[2] = 0x00; // rpm 0,2
  txFrame2.data.uint8[3] = 0x00; // rpm 2,4
  txFrame2.data.uint8[4] = 0x65;
  txFrame2.data.uint8[5] = 0x12;
  txFrame2.data.uint8[6] = 0x00;
  txFrame2.data.uint8[7] = 0x62;
  frames[1] = txFrame2;

  // Temp - temp needle to beginning of red section [0xE6] - celsius*2
  CAN_FRAME txFrame3;
  txFrame3.rtr = 0;
  txFrame3.extended = false;
  txFrame3.length = 8;
  txFrame3.id = 0x329;
  txFrame3.data.uint8[0] = 0xD9;
  txFrame3.data.uint8[1] = 0xE6;
  txFrame3.data.uint8[2] = 0xB2;
  txFrame3.data.uint8[3] = 0x19;
  txFrame3.data.uint8[4] = 0x00;
  txFrame3.data.uint8[5] = 0xEE;
  txFrame3.data.uint8[6] = 0x00;
  txFrame3.data.uint8[7] = 0x00;
  frames[2] = txFrame3;

  // connect EVSE - every 100ms - BMS heartbeat
  CAN_FRAME txFrame4;
  txFrame4.rtr = 0;
  txFrame4.extended = false;
  txFrame4.length = 8;
  txFrame4.id = 0x285;
  txFrame4.data.uint8[0] = 0x00;
  txFrame4.data.uint8[1] = 0x00;
  txFrame4.data.uint8[2] = 0x00; // b6 to activate EVSE
  txFrame4.data.uint8[3] = 0x00;
  txFrame4.data.uint8[4] = 0x00;
  txFrame4.data.uint8[5] = 0x00;
  txFrame4.data.uint8[6] = 0x00;
  txFrame4.data.uint8[7] = 0x00;
  frames[3] = txFrame4;

  // set charge - every 800ms
  CAN_FRAME txFrame5;
  txFrame5.rtr = 0;
  txFrame5.extended = false;
  txFrame5.length = 8;
  txFrame5.id = 0x286;
  txFrame5.data.uint8[0] = 0x28; // voltage
  txFrame5.data.uint8[1] = 0x0d; // voltage 0f 388, 0d 337
  txFrame5.data.uint8[2] = 0x3c; // charge current 78 12A, 3c 6A
  txFrame5.data.uint8[3] = 0x37;
  txFrame5.data.uint8[4] = 0x00;
  txFrame5.data.uint8[5] = 0x00;
  txFrame5.data.uint8[6] = 0x0A;
  txFrame5.data.uint8[7] = 0x00;
  frames[4] = txFrame5;

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
  CAN_FRAME frame;
  if (CAN0.read(frame))
  {
    switch (frame.id)
    {
    case 0x613:
      handle613(frame);
      break;
    default:
      handleOBCDCFrame(frame);
      break;
    }

    // store message to buffer
    b2w->addBuffer(0xf1);
    b2w->addBuffer(0x00); // 0 = canbus frame sending
    uint64_t now = status.getTimestampMicro();
    b2w->addBuffer(now & 0xFF);
    b2w->addBuffer(now >> 8);
    b2w->addBuffer(now >> 16);
    b2w->addBuffer(now >> 24);
    b2w->addBuffer(frame.id & 0xFF);
    b2w->addBuffer(frame.id >> 8);
    b2w->addBuffer(frame.id >> 16);
    b2w->addBuffer(frame.id >> 24);
    b2w->addBuffer(frame.length + (uint8_t)(((int)1) << 4)); // 2 ibus address
    for (int c = 0; c < frame.length; c++)
      b2w->addBuffer(frame.data.uint8[c]);
    b2w->addBuffer(0x0a); // new line in serial monitor

    if (status.currentMillis - lastSentCanLog >= 100)
    {
      lastSentCanLog = status.currentMillis;
      b2w->send();
    }
  }
}

int CanBus::handle613(CAN_FRAME frame)
{
  const int v = (int)(frame.data.bytes[2]);
  if (status.ikeFuelLevel != v)
  {
    mqttClientCan->sendMessageToTopic(String(v), (String(wifiSettings.hostname) + "/out/IKE/fuel_level"));
    status.ikeFuelLevel = v;
  }
  return status.ikeFuelLevel;
}

long CanBus::handleOBCDCFrame(CAN_FRAME frame)
{
  if (frame.id == 0x377 || frame.id == 0x389 || frame.id == 0x38a)
  {
    uint64_t ts = status.getTimestampMicro();
    switch (frame.id)
    {
    case 0x377:
      collectors[settingsCollectors.getCollectorIndex(DCVOLTAGE)]->handle(frame.data.bytes[0] << 8 | frame.data.bytes[1], ts); // centivolts
      collectors[settingsCollectors.getCollectorIndex(DCCURRENT)]->handle(frame.data.bytes[2] << 8 | frame.data.bytes[3], ts); // deciamps
      collectors[settingsCollectors.getCollectorIndex(DCTEMP1)]->handle(frame.data.bytes[4] - 40, ts);                         // celsius
      collectors[settingsCollectors.getCollectorIndex(DCTEMP2)]->handle(frame.data.bytes[5] - 40, ts);                         // celsius
      collectors[settingsCollectors.getCollectorIndex(DCTEMP3)]->handle(frame.data.bytes[6] - 40, ts);                         // celsius
      collectors[settingsCollectors.getCollectorIndex(DCSTATUS)]->handle(frame.data.bytes[7], ts);                              // 0x20 standby, 0x21 error, 0x22 working
      break;
    case 0x389:
      collectors[settingsCollectors.getCollectorIndex(VOLTAGE)]->handle(frame.data.bytes[0] * 2, ts);   // volts
      collectors[settingsCollectors.getCollectorIndex(SUPPLYVOLTAGE)]->handle(frame.data.bytes[1], ts); // volts
      collectors[settingsCollectors.getCollectorIndex(SUPPLYCURRENT)]->handle(frame.data.bytes[2], ts); // deciamps
      collectors[settingsCollectors.getCollectorIndex(TEMP1)]->handle(frame.data.bytes[3] - 40, ts);    // celsius
      collectors[settingsCollectors.getCollectorIndex(TEMP2)]->handle(frame.data.bytes[4] - 40, ts);    // celsius
      break;
    case 0x38A:
      collectors[settingsCollectors.getCollectorIndex(EVSECTC)]->handle(frame.data.bytes[3], ts); // pwm %
    default:
      break;
    }
    return 0;
  }
  return 0;
}

void CanBus::sendMessageSet()
{
  // IKE - create and send can message in intervals
  if (status.currentMillis - previousMillis >= intervals.CANsend)
  {
    previousMillis = status.currentMillis;

    // update rpm and coolant temp values
    displayValue = status.rpm * 6.4;
    frames[1].data.uint8[2] = (int)((displayValue & 0X000000FF));      // rpm 0,2
    frames[1].data.uint8[3] = (int)((displayValue & 0x0000FF00) >> 8); // rpm 2,4
    displayValue = status.coolant_temp * 2;
    frames[2].data.uint8[1] = (int)((displayValue & 0X000000FF));

    CAN0.sendFrame(frames[0]);
    CAN0.sendFrame(frames[1]);
    CAN0.sendFrame(frames[2]);
  }

  // always send 0x285 as BMS hear beat
  if (status.currentMillis - previousMillis100 >= 30)
  {
    previousMillis100 = status.currentMillis;
    if (status.chargerPullEVSE == 1)
      frames[3].data.uint8[2] = 0xB6;
    else
      frames[3].data.uint8[2] = 0x00;

    CAN0.sendFrame(frames[3]);
  }

  if (status.chargerStarted == 1 && status.currentMillis - previousMillis800 >= 500)
  {
    previousMillis800 = status.currentMillis;
    frames[4].data.uint8[0] = (int)((status.chargerVoltageRequest & 0X000000FF));      // voltage*10 big endian
    frames[4].data.uint8[1] = (int)((status.chargerVoltageRequest & 0x0000FF00) >> 8); //
    frames[4].data.uint8[2] = status.chargerCurrentRequest;                            // current*10
    CAN0.sendFrame(frames[4]);
  }
}
